// cpu_backend_utils.cpp
// Base utilities for working with eBPF map for throttling

#include "./cpu_backend_utils.h"

#include <bpf/bpf.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <iostream>

#define MAP_PATH "/sys/fs/bpf/cpu_times"
#define MAX_NAME_LEN 256

using namespace std;

bool is_kernel_thread(pid_t pid) 
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    return access(path, F_OK) != 0;
}

bool is_root_process(pid_t pid) 
{
    string path = "/proc/" + to_string(pid);
    struct stat st;
    if (stat(path.c_str(), &st) == 0) 
    {
        return st.st_uid == 0;
    }
    return false;
}

map<string, ProcessGroup> read_process_groups_from_map() 
{
    map<string, ProcessGroup> grouped;

    int map_fd = bpf_obj_get(MAP_PATH);     // Again open map
    if (map_fd < 0) 
    {
        perror("Failed to open BPF map");
        return grouped;
    }

    __u32 pid = -1, next_pid;
    __u64 cpu_time;

    while (bpf_map_get_next_key(map_fd, &pid, &next_pid) == 0) 
    {
        if (bpf_map_lookup_elem(map_fd, &next_pid, &cpu_time) != 0) 
        {
            pid = next_pid;
            continue;
        }

        // Same kind of traversal as described in cpu_reader.cpp

        char path[64], name[MAX_NAME_LEN];
        snprintf(path, sizeof(path), "/proc/%u/comm", next_pid);
        FILE* f = fopen(path, "r");
        if (!f || !fgets(name, sizeof(name), f)) 
        {
            pid = next_pid;
            if (f) fclose(f);
            continue;
        }
        fclose(f);
        name[strcspn(name, "\n")] = '\0';

        if (is_kernel_thread(next_pid) || is_root_process(next_pid)) 
        {
            pid = next_pid;
            continue;
        }

        string proc_name(name);
        if (grouped.count(proc_name) == 0) 
        {
            grouped[proc_name] = {proc_name, {}, 0};
        }

        grouped[proc_name].pids.push_back(next_pid);
        grouped[proc_name].total_cpu_time += cpu_time;

        pid = next_pid;
    }

    return grouped;
}
