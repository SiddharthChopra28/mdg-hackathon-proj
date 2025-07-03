#include <sys/socket.h>
#include <sys/un.h>
#include "nlohmann_json.hpp"
#include <thread>
#include <future>
#include <fstream>
#include <csignal>
#include "network.hpp"

void handle_network_socket()
{
    const char *SOCKET_PATH = "/tmp/network_optimizer.sock";

    // Remove old socket if exists
    unlink(SOCKET_PATH);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket creation failed");
        return;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        return;
    }

    // Allow non-root processes to connect
    chmod(SOCKET_PATH, 0666);

    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        close(server_fd);
        return;
    }

    std::cout << "[+] Network socket server started on " << SOCKET_PATH << std::endl;

    while (true)
    {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            perror("accept failed");
            continue;
        }

        char buffer[4096] = {0};
        ssize_t len = read(client_fd, buffer, sizeof(buffer));
        if (len <= 0)
        {
            close(client_fd);
            continue;
        }

        try
        {
            auto j = nlohmann::json::parse(std::string(buffer, len));
            std::string action = j["action"];
            nlohmann::json response;

            if (action == "network_get_usage")
            {
                auto stat_map = sendNetworkStatsPerPID();
                for (const auto &[app, usage] : stat_map)
                {
                    response.push_back({{"app_name", app},
                                        {"upload_bytes", usage.first},
                                        {"download_bytes", usage.second}});
                }
            }
            else if (action == "network_set_speed_cap")
            {
                // std::string app = "firefox";
                // int mbps = 10;
                std::string app = j["app_name"];
                int mbps = j["speed_mbps"];
                int rate_bps = mbps * 1024 * 1024 / 8;
                setAppRateLimit(app, rate_bps);
                response = "Speed cap set";

            } else if (action == "network_reset_cap") {
                std::string app = j["app_name"];
                resetAppRateLimit(app);
                response = "Speed cap reset";
            }
            else if (action == "network_get_overall")
            {
                auto overall = sendNetworkStatsOverall();
                response["outgoing"] = overall.first;
                response["incoming"] = overall.second;
                response["total"] = overall.first + overall.second;
            }else
            {
                response = "Unknown action";
            }

            std::string resp_str = response.dump();
            ssize_t bytes_written = write(client_fd, resp_str.c_str(), resp_str.size());
            if (bytes_written < 0)
            {
                perror("write failed (response)");
            }
        }
        catch (std::exception &e)
        {
            std::string err = std::string("Error: ") + e.what();
            ssize_t err_written = write(client_fd, err.c_str(), err.size());
            if (err_written < 0)
            {
                perror("write failed (error response)");
            }
        }

        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
}


void maintainer(){

    for (auto it = overall_vector.begin(); it != overall_vector.end();){

        __u64 curr_time = get_kernel_time_ns();

        if (curr_time - it->timestamp_ns > 1000000000ULL){
            it = overall_vector.erase(it);
        }
        else
        {
            break;
        }
    }

    for (auto& pair : pid_wise_map){

        if (pair.second.size() != 0){
            for (auto it = pair.second.begin(); it!=pair.second.end();){
                __u64 curr_time = get_kernel_time_ns();
                if (curr_time - it->timestamp_ns > 1000000000ULL){
                    it = pair.second.erase(it);
                }
                else
                {
                    break;
                }
            }
        }
    }
}

int ringbuf_datahandler(void *ctx, void *data, size_t data_sz)
{
    net_data nd = *(net_data *)data;

    data_t d = {};
    d.bytes = nd.bytes;
    d.timestamp_ns = nd.timestamp_ns;

    std::string path = "/proc/" + std::to_string(nd.pid) + "/exe";
    char resolved[PATH_MAX];
    ssize_t len = readlink(path.c_str(), resolved, sizeof(resolved) - 1);

    std::string parentApp;

    if (len != -1)
    {
        resolved[len] = '\0';
        const char *last_slash = strrchr(resolved, '/');
        parentApp = last_slash ? std::string(last_slash + 1) : std::string(resolved);
    }
    else
    {
        std::cout << "error in reading /proc/pid/exe";
        return -1;
    }

    std::pair<int, std::string> np;
    np.first = nd.pid;
    np.second = parentApp;

    pid_wise_map[np].push_back(d);
    overall_vector.push_back(d);

    return 0;
}

void manageData(bool &stopPolling, struct network_bpf *skel)
{
    struct ring_buffer *rb = NULL;
    rb = ring_buffer__new(bpf_map__fd(skel->maps.netdata_ringbuf), ringbuf_datahandler, NULL, NULL);
    
    while (!stopPolling){
        int err = ring_buffer__poll(rb, 100);
        if (err == -EINTR)
        {
            break;
        }
        else if (err < 0)
        {
            std::cout << "Failed to poll ringbuff";
            break;
        }
        maintainer();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ring_buffer__free(rb);
}


void publisher(){
    while (true){
        //overall network usage
        int bytes_sent_per_sec = 0;
        int bytes_recd_per_sec = 0;

        for (auto entry : overall_vector)
        {
            if (entry.bytes < 0)
            {
                bytes_recd_per_sec -= entry.bytes;
            }
            else
            {
                bytes_sent_per_sec += entry.bytes;
            }
        }

        // std::cout<<overall_vector.size()<<std::endl;


        std::cout<<bytes_recd_per_sec<<std::endl;
        std::cout<<bytes_sent_per_sec<<std::endl;

        // std::cout<<"Upload speed (bytes/sec): "<<bytes_sent_per_sec<<std::endl;
        // std::cout<<"Download speed (bytes/sec): "<<bytes_recd_per_sec<<std::endl;
        // std::cout<<"-------"<<std::endl;

        // for (auto pair: pid_wise_map){
        //     std::cout<<pair.first.first<<" - "<<pair.first.second<<" - "<<std::endl;
        // }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

uint64_t get_cgroup_id(const char *cgroup_path)
{
    struct stat st;
    if (stat(cgroup_path, &st) == 0)
    {
        return st.st_ino;
    }
    else
    {
        perror("stat");
        return 0;
    }
}

void bucket_refiller()
{
    int map_fd = bpf_obj_get("/sys/fs/bpf/token_buckets");
    if (map_fd < 0)
    {
        std::cerr << "Failed to open mapfd";
    }

    while (1)
    {
        std::map<std::string, __u64> local_limits;
        {
            std::lock_guard<std::mutex> lock(mtx);
            local_limits = app_limits; // Copy under lock
        }

        for (auto pair : local_limits)
        {

            std::string appname = pair.first;
            __u64 rate = pair.second;

            std::string cgrouppath = "/sys/fs/cgroup/" + appname;
            uint64_t cid = get_cgroup_id(cgrouppath.c_str());
            if (cid == 0)
            {
                std::cout << "Invalid cgroup ID for " << appname << std::endl;
                continue;
            }

            rate_limit_t r = {};

            if (bpf_map_lookup_elem(map_fd, &cid, &r) != 0)
            {
                std::cout << "Error in reading the map";
                continue;
            }

            long double last_time = ((long double)r.last_time) / 1000000000;

            r.rate = pair.second;

            auto now = std::chrono::steady_clock::now();
            long double now_sec = std::chrono::duration_cast<std::chrono::duration<long double>>(now.time_since_epoch()).count();

            long double delta_sec = now_sec - last_time;

            __u64 hyp_tokens = r.tokens + (delta_sec * r.rate);

            r.tokens = hyp_tokens < r.max_tokens ? hyp_tokens : r.max_tokens;

            // std::cout<<"Total tokens"<<r.tokens<<std::endl;
            __u64 time2 = __u64(now_sec * 1000000000);
            r.last_time = time2;

            if (bpf_map_update_elem(map_fd, &cid, &r, BPF_ANY) < 0)
            {
                std::cout << "Error in writing the map";
                continue;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void throttle_app(std::string &appname, int ratelimit)
{
    std::lock_guard<std::mutex> lock(mtx);

    int map_fd = bpf_obj_get("/sys/fs/bpf/token_buckets");

    std::string cgrouppath = "/sys/fs/cgroup/" + appname;
    uint64_t cid = get_cgroup_id(cgrouppath.c_str());

    rate_limit_t r = {};
    r.rate = ratelimit;                                           // in bytes /s
    r.max_tokens = ratelimit / 100 < 100 ? 100 : ratelimit / 100; // burst bytes- keeping same as rate ie burst window = 1s
    r.tokens = r.max_tokens;                                      // initially bucket full hai

    auto now = std::chrono::system_clock::now();
    long double time_sec = std::chrono::duration_cast<std::chrono::duration<long double>>(now.time_since_epoch()).count();
    std::cout << time_sec << std::endl;
    __u64 time = (__u64)(time_sec * 1000000000);
    r.last_time = time; // in nanoseconds

    app_limits.insert({appname, r.rate});

    bpf_map_update_elem(map_fd, &cid, &r, BPF_ANY);
    rate_limit s = {};
    bpf_map_lookup_elem(map_fd, &cid, &s);
}

bool is_pid_in_cgroup(int pid, const std::string &cgroup_name)
{
    std::ifstream cgroup_file("/proc/" + std::to_string(pid) + "/cgroup");
    std::string line;
    while (std::getline(cgroup_file, line))
    {
        auto pos = line.find("::");
        if (pos != std::string::npos)
        {
            std::string path = line.substr(pos + 2);
            return path == cgroup_name || path == "/" + cgroup_name;
        }
    }
    return false;
}

int create_add_cgroup(int &pid, std::string &appname)
{

    struct stat info;
    std::string path = "/sys/fs/cgroup/" + appname;

    if (!(stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode)))
    {
        // make the cgroup
        int mkc = system("sudo mount -t cgroup2 none /sys/fs/cgroup");
        std::string cmd = "sudo mkdir /sys/fs/cgroup/" + appname;
        mkc = system(cmd.c_str());
    }

    if (!is_pid_in_cgroup(pid, appname))
    {
        std::string cmd2 = "echo " + std::to_string(pid) + " | sudo tee /sys/fs/cgroup/" + appname + "/cgroup.procs";
        int mkc2 = system(cmd2.c_str());
    }

    std::string cd = "sudo bpftool cgroup attach /sys/fs/cgroup/ ingress pinned /sys/fs/bpf/throttle_prog/throttler_ingress";
    int bc = system(cd.c_str());
    cd = "sudo bpftool cgroup attach /sys/fs/cgroup/ egress pinned /sys/fs/bpf/throttle_prog/throttler_egress";
    bc = system(cd.c_str());

    return 0;
}

std::map<std::string, std::pair<int, int>> sendNetworkStatsPerPID()
{
    std::map<std::string, std::pair<int, int>> retmap;
    // pair has upload, download
    for (auto pair : pid_wise_map)
    {
        std::pair<int, int> tempair;
        int uploads = 0;
        int downloads = 0;
        for (auto entry : pair.second)
        {
            if (entry.bytes > 0)
            {
                uploads += entry.bytes;
            }
            else
            {
                downloads -= entry.bytes;
            }
        }
        tempair.first = uploads;
        tempair.second = downloads;
        retmap.insert({pair.first.second, tempair});
    }
    return retmap;
}

std::pair<int, int> sendNetworkStatsOverall()
{
    std::pair<int, int> tempair;
    int uploads = 0;
    int downloads = 0;
    for (auto entry : overall_vector)
    {
        if (entry.bytes > 0)
        {
            uploads += entry.bytes;
        }
        else
        {
            downloads -= entry.bytes;
        }
    }
    tempair.first = uploads;
    tempair.second = downloads;

    return tempair;
}

std::vector<int> get_pids_of_app(std::string appname)
{
    std::vector<int> retvec;
    for (auto entry : pid_wise_map)
    {
        if (entry.first.second == appname)
        {
            retvec.push_back(entry.first.first);
        }
    }
    return retvec;
}

void setAppRateLimit(std::string appname, int rate)
{
    // rate is in bytes/sec
    auto pids = get_pids_of_app(appname);
    for (auto pid : pids)
    {
        create_add_cgroup(pid, appname);
    }
    throttle_app(appname, rate);
}

void cleanup()
{
    int bc = system("sudo ./unload_throttler.sh");
}

void resetAppRateLimit(std::string appname)
{
    throttle_app(appname, 0);
}

int main()
{

    struct network_bpf *network_skel = network_bpf__open_and_load();
    if (!network_skel)
    {
        std::cerr << "Error in opening network bpf object";
        return 1;
    }

    int err = network_bpf__attach(network_skel);
    if (err)
    {
        std::cerr << "Failed to attach BPF program";
        return 1;
    }

    std::cout << "Program loaded and attached" << std::endl;

    int bc = system("sudo ./load_throttler.sh");
    std::cout << "Success" << std::endl;

    bool stopPolling = false;

    std::thread t_manage(manageData, std::ref(stopPolling), network_skel);

    // std::thread t_publish(publisher);

    std::thread t_refiller(bucket_refiller);

    std::thread t_socket(handle_network_socket);

    t_socket.join();
    t_manage.join();
    // t_publish.join();
    t_refiller.join();

    return 0;
}