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
#include<mutex>
#include<chrono>

extern "C"{
    #include "network.skel.h"
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

typedef struct rate_limit{
    __u64 rate; // bytes per sec
    __u64 max_tokens; 
    __u64 tokens;
    __u64 last_time;

} rate_limit_t;



void maintainer();
int ringbuf_datahandler(void* ctx, void* data, size_t data_sz);
void manageData(bool& stopPolling, struct network_bpf* skel);
void publisher();
uint64_t get_cgroup_id(const char* cgroup_path);
void bucket_refiller();
void throttle_app(std::string& appname, int ratelimit);
bool is_pid_in_cgroup(int pid, const std::string& cgroup_name);
int create_add_cgroup(int& pid, std::string& appname);
std::map<std::string, std::pair<int, int>> sendNetworkStatsPerPID();
std::pair<int, int> sendNetworkStatsOverall();
std::vector<int> get_pids_of_app(std::string appname);
void setAppRateLimit(std::string appname, int rate);
void cleanup();
void resetAppRateLimit(std::string);
int setup_network_prog();