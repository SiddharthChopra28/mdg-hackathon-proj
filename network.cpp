#include<iostream>
#include<sys/syscall.h>
#include<unistd.h>
#include<thread>
#include<vector>
#include<map>
#include<limits.h>
#include<cstdlib>
#include <net/if.h>
#include<array>
#include<sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<fstream>

extern "C"{
    #include "network.skel.h"
    // #include "throttler.skel.h"
    #include <bpf/bpf.h>
}

typedef struct net_data{
    int pid;
    unsigned long long timestamp_ns;
    int bytes; // + for sent, - for recvd
} net_data;

typedef struct data_t{
    int bytes;
    unsigned long long timestamp_ns;
}data_t;

uint64_t get_kernel_time_ns(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec*1000000000ULL + ts.tv_nsec;
}

typedef struct rate_limit{
    __u64 rate; // bytes per sec
    __u64 max_tokens; 
    __u64 tokens;
    __u64 last_time;

} rate_limit_t;




std::map<std::pair<int, std::string>, std::vector<data_t>> pid_wise_map;

std::vector<data_t> overall_vector;

std::map<std::string, long int> app_limits; 


void maintainer(){
    
    uint64_t curr_time = get_kernel_time_ns();

    for (auto it = overall_vector.begin(); it != overall_vector.end();){
        if (curr_time - it->timestamp_ns > 1000000000ULL){
            it = overall_vector.erase(it);
        }
        else{
            break;
        }
    }

    for (auto& pair : pid_wise_map){
        if (pair.second.size() != 0){
            for (auto it = pair.second.begin(); it!=pair.second.end();){
                if (curr_time - it->timestamp_ns > 1000000000ULL){
                    it = pair.second.erase(it);
                }
                else{
                    break;
                }

            }
        }
    }


    
}

int ringbuf_datahandler(void* ctx, void* data, size_t data_sz){
    net_data nd = *(net_data*)data;

    data_t d = {};
    d.bytes = nd.bytes;
    d.timestamp_ns = nd.timestamp_ns;

    std::string path = "/proc/"+ std::to_string(nd.pid)+"/exe";
    char resolved [PATH_MAX];
    ssize_t len = readlink(path.c_str(), resolved, sizeof(resolved)-1);

    std::string parentApp;

    if (len != -1){
        resolved[len] = '\0';
        const char* last_slash = strrchr(resolved, '/');
        parentApp  =  last_slash ? std::string(last_slash + 1) : std::string(resolved);
    }
    else{
        std::cout<<"error in reading /proc/pid/exe";
        return -1;
    }

    std::pair<int, std::string> np;
    np.first=nd.pid;
    np.second = parentApp;

    pid_wise_map[np].push_back(d);
    overall_vector.push_back(d);

    return 0;
}

void manageData(bool& stopPolling, struct network_bpf* skel){
    struct ring_buffer* rb = NULL;
    rb = ring_buffer__new(bpf_map__fd(skel->maps.netdata_ringbuf), ringbuf_datahandler, NULL, NULL);

    while (!stopPolling){
        int err = ring_buffer__poll(rb, 100);
        if (err == -EINTR){
            break;
        }
        else if (err<0){
            std::cout<<"Failed to poll ringbuff";
            break;
        }
        maintainer();
    }
    ring_buffer__free(rb);
}

void publisher(){
    while (true){
        //overall network usage
        int bytes_sent_per_sec = 0;
        int bytes_recd_per_sec = 0;
    
        for (auto entry: overall_vector){
            if (entry.bytes<0){
                bytes_recd_per_sec -= entry.bytes;

            }
            else{
                bytes_sent_per_sec += entry.bytes;
            }
        }

        // std::cout<<"Upload speed (bytes/sec): "<<bytes_sent_per_sec<<std::endl;
        // std::cout<<"Download speed (bytes/sec): "<<bytes_recd_per_sec<<std::endl;
        // std::cout<<"-------"<<std::endl;

        for (auto pair: pid_wise_map){
            std::cout<<pair.first.first<<" - "<<pair.first.second<<" - "<<std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
}


uint64_t get_cgroup_id(const char* cgroup_path) {
    struct stat st;
    if (stat(cgroup_path, &st) == 0) {
        return st.st_ino;
    } else {
        perror("stat");
        return 0;
    }
}

void bucket_refiller(){
    int map_fd = bpf_obj_get("/sys/fs/bpf/token_buckets");
    
    for (auto pair : )

}

void throttle_app(std::string& appname){

    int map_fd = bpf_obj_get("/sys/fs/bpf/token_buckets");

    std::string cgrouppath = "/sys/fs/cgroup/"+appname;
    uint64_t cid = get_cgroup_id(cgrouppath.c_str());

    rate_limit_t r = {};
    r.rate = 1000000; //1mbps
    r.max_tokens = 1000000; // burst bytes- keeping same as rate ie burst window = 1s
    r.tokens = 1000000; // initially bucket full hai
    r.last_time = get_kernel_time_ns();

    bpf_map_update_elem(map_fd, &cid, &r, BPF_ANY);
    
}


bool is_pid_in_cgroup(int pid, const std::string& cgroup_name) {
    std::ifstream cgroup_file("/proc/" + std::to_string(pid) + "/cgroup");
    std::string line;
    while (std::getline(cgroup_file, line)) {
        auto pos = line.find("::");
        if (pos != std::string::npos) {
            std::string path = line.substr(pos + 2);
            return path == cgroup_name || path == "/" + cgroup_name;
        }
    }
    return false;
}

int create_add_cgroup(int& pid, std::string& appname){
    
    struct stat info;
    std::string path = "/sys/fs/cgroup/"+appname;


    if (!(stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode))){
        // make the cgroup
        int mkc = system("sudo mount -t cgroup2 none /sys/fs/cgroup");
        std::string cmd = "sudo mkdir /sys/fs/cgroup/" + appname;
        mkc = system(cmd.c_str());
    }

    if (!is_pid_in_cgroup(pid, appname)){
        std::string cmd2 = "echo " + std::to_string(pid) + " | sudo tee /sys/fs/cgroup/"+appname+"/cgroup.procs";
        int mkc2 = system(cmd2.c_str());
    }

    std::string cd = "sudo bpftool cgroup attach /sys/fs/cgroup/ ingress pinned /sys/fs/bpf/throttle_prog/throttler_ingress";
    int bc = system(cd.c_str());
    cd = "sudo bpftool cgroup attach /sys/fs/cgroup/ egress pinned /sys/fs/bpf/throttle_prog/throttler_egress";
    bc = system(cd.c_str());

    return 0;

}

int delete_cgroup(int& pid){
    return 0;
}


int main(){

    struct network_bpf *network_skel = network_bpf__open_and_load();
    if (!network_skel){
        std::cerr<<"Error in opening network bpf object";
        return 1;
    }

    int err = network_bpf__attach(network_skel);
    if (err){
        std::cerr<<"Failed to attach BPF program";
        return 1;
    }

    std::cout<<"Program loaded and attached"<<std::endl;

    
    int bc = system("sudo ./load_throttler.sh");
    std::cout<<"Success"<<std::endl;

    int pid = 4299;
    std::string appname = "firefox";

    create_add_cgroup(pid, appname);
    throttle_app(appname);


    bool stopPolling = false;

    std::thread t_manage (manageData, std::ref(stopPolling), network_skel);

    std::thread t_publish(publisher);

    t_manage.join();
    t_publish.join();


    return 0;
}