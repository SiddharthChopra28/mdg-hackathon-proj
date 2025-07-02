// cpu_reader.cpp (modernized with root process protection and cleanup)

#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <bits/stdc++.h>
#include "Icpu_optimizer.h"
#include "./utils/cpu_backend_utils.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

#define MAP_PATH "/sys/fs/bpf/cpu_times"
#define MAX_NAME_LEN 256
#define IGNORE_CPU_THRESHOLD_NS 2000000000ULL  // 2 seconds

struct ProcessInfo {
    string name;
    uint64_t total_cpu_time;
    int pid_count;
};





// int main() 
// {
//     // print_processes();
//     return 0;
// }

json get_process_data() {
    json result = json::array();

    int map_fd = bpf_obj_get(MAP_PATH);
    if (map_fd < 0) {
        return "Failed to open BPF map\n";
    }

    vector<ProcessInfo> aggregated;
    ProcessInfo other = {"Other", 0, 0};

    __u32 pid = -1, next_pid;
    __u64 cpu_time;

    while (bpf_map_get_next_key(map_fd, &pid, &next_pid) == 0) {
        if (bpf_map_lookup_elem(map_fd, &next_pid, &cpu_time) != 0) {
            pid = next_pid;
            continue;
        }

        char path[64], name[MAX_NAME_LEN];
        snprintf(path, sizeof(path), "/proc/%u/exe", next_pid);
        char resolved_path[PATH_MAX];
        ssize_t len = readlink(path, resolved_path, sizeof(resolved_path) - 1);
        if (len == -1) {
            pid = next_pid;
            continue;
        }
        resolved_path[len] = '\0';

        const char* base = strrchr(resolved_path, '/');
        if (!base || strlen(base + 1) >= MAX_NAME_LEN) {
            pid = next_pid;
            continue;
        }
        strncpy(name, base + 1, MAX_NAME_LEN);
        name[strcspn(name, "\n")] = '\0';

        if (is_kernel_thread(next_pid) || is_root_process(next_pid)) {
            pid = next_pid;
            continue;
        }

        if (cpu_time < IGNORE_CPU_THRESHOLD_NS) {
            other.total_cpu_time += cpu_time;
            other.pid_count += 1;
            pid = next_pid;
            continue;
        }

        auto it = find_if(aggregated.begin(), aggregated.end(), [&](const ProcessInfo& p) {
            return p.name == name;
        });

        if (it != aggregated.end()) {
            it->total_cpu_time += cpu_time;
            it->pid_count += 1;
        } else {
            aggregated.push_back({name, cpu_time, 1});
        }

        pid = next_pid;
    }

    for (const auto& proc : aggregated) {
        result.push_back({
            {"name", proc.name},
            {"cpu_time", proc.total_cpu_time / 1e9},
            {"pid_count", proc.pid_count}
        });
    }

    if (other.pid_count > 0) {
        result.push_back({
            {"name", "Other"},
            {"cpu_time", other.total_cpu_time / 1e9},
            {"pid_count", other.pid_count}
        });
    }

    return result;
}
