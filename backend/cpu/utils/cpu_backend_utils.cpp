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

bool is_kernel_thread(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);      // we are writing the path to actual executable file that launched the process
                                                            // Proc is a virtual filesystem used by Linux, providing an interface to kernel level data structures
                                                            // at proc/pid/ we get information of a specific pid
                                                            // exe is a symbolic link that points to said executable file.
    return access(path, F_OK) != 0;         // Checking if file exists and can be accessed. Returns 0 if true or returns -1 if false.
                                            // Kernel level threads are not launched from binaries on the system. So if a pid represents a kernel level thread,
                                            // it returns -1. So our function returns 1.
}

bool is_root_process(pid_t pid) {
    string path = "/proc/" + to_string(pid);
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return st.st_uid == 0;      // uid of a root process is 0
    }
    return false;
}

void ensure_whitelist_dir() {
    const char* home = getenv("HOME");
    if (!home) return;

    string path = string(home) + "/.config/cpu_optimizer";

    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        mkdir((string(home) + "/.config").c_str(), 0755);  // Ensure ~/.config exists
        mkdir(path.c_str(), 0755);                         // Then create ~/.config/cpu_optimizer
    }
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


const string WHITELIST_PATH = string(getenv("HOME")) + "/.config/cpu_optimizer/whitelist.txt";


set<string> load_whitelist() {
    ensure_whitelist_dir();

    set<string> whitelist;
    ifstream file(WHITELIST_PATH);
    string line;
    while (getline(file, line)) {
        if (!line.empty()) whitelist.insert(line);
    }
    return whitelist;
}

bool add_to_whitelist(const string& name) {
    ensure_whitelist_dir();

    set<string> whitelist = load_whitelist();
    if (whitelist.count(name)) return false;

    ofstream file(WHITELIST_PATH, ios::app);
    if (!file) return false;

    file << name << '\n';
    return true;
}

bool remove_from_whitelist(const string& name) {
    ensure_whitelist_dir();

    set<string> whitelist = load_whitelist();
    if (!whitelist.erase(name)) return false;

    ofstream file(WHITELIST_PATH, ios::trunc);
    for (const auto& p : whitelist) {
        file << p << '\n';
    }
    return true;
}


