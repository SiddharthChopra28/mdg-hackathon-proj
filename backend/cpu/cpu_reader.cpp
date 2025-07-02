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

using namespace std;

#define MAP_PATH "/sys/fs/bpf/cpu_times"
#define MAX_NAME_LEN 256
#define IGNORE_CPU_THRESHOLD_NS 2000000000ULL  // 2 seconds

struct ProcessInfo {
    string name;
    uint64_t total_cpu_time;
    int pid_count;
};

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

int main() 
{
    int map_fd = bpf_obj_get(MAP_PATH);
    if (map_fd < 0) 
    {
        perror("Failed to open BPF map");
        return 1;
    }
    // Checked if map was existing or not.

    vector<ProcessInfo> aggregated;         //Initializing vector of process infor
    ProcessInfo other = {"Other", 0, 0};    // catch for all processes that do not meet display criteria

    __u32 pid = -1, next_pid;               // init 2 vars, current pid and next pid. current is at -1 which is used to get the reference to the first pid
    __u64 cpu_time;                         // store the looked up value for each pid

    while (bpf_map_get_next_key(map_fd, &pid, &next_pid) == 0)      // syscall func returns 0 as long as next key exists for current key. 
    {
        if (bpf_map_lookup_elem(map_fd, &next_pid, &cpu_time) != 0) // syscall func to read corresponding value of given key. 3rd param consits
                                                                    // of the looked up val, and func returns 0 as long as val is found for the key
        {
            pid = next_pid;                                         // So if for current pid lookup val failed, skip it and move to next pid.
            continue;
        }

        char path[64], name[MAX_NAME_LEN];
        snprintf(path, sizeof(path), "/proc/%u/exe", next_pid);    // use /proc/[pid]/exe to follow symlink to full binary path
        char resolved_path[PATH_MAX];
        ssize_t len = readlink(path, resolved_path, sizeof(resolved_path) - 1);
        if (len == -1) {
            pid = next_pid;
            continue;
        }
        resolved_path[len] = '\0';
        
        // Extract just the executable name from the full path
        const char* base = strrchr(resolved_path, '/');
        if (!base || strlen(base + 1) >= MAX_NAME_LEN) {
            pid = next_pid;
            continue;
        }
        strncpy(name, base + 1, MAX_NAME_LEN);

        name[strcspn(name, "\n")] = '\0';               //remove newline char aT end of process name

        if (is_kernel_thread(next_pid) || is_root_process(next_pid))    // Skip kernel level threads or root processes
        {
            pid = next_pid;
            continue;
        }

        if (cpu_time < IGNORE_CPU_THRESHOLD_NS)           // If cpu time is lesser than some minimum threshold, ignore that pid
        {
            other.total_cpu_time += cpu_time;
            other.pid_count += 1;
            pid = next_pid;
            continue;
        }
                                                                //Lambda function go brr
        auto it =  find_if(aggregated.begin(), aggregated.end(), [&](const ProcessInfo& p)      
        {
            return p.name == name;
        });

        if (it != aggregated.end())
        {
            it->total_cpu_time += cpu_time;
            it->pid_count += 1;
        } else 
        {
            aggregated.push_back({name, cpu_time, 1});
        }

        pid = next_pid;
    }

     sort(aggregated.begin(), aggregated.end(), [](const ProcessInfo& a, const ProcessInfo& b) 
    {
        return a.total_cpu_time > b.total_cpu_time;
    });

    printf("%-25s %-12s %-6s\n", "Process Name", "CPU Time(s)", "PIDs");
    printf("----------------------------------------------------------\n");

    for (const auto& proc : aggregated) 
    {
        printf("%-25s %-12.2f %-6d\n",
               proc.name.c_str(),
               proc.total_cpu_time / 1e9,
               proc.pid_count);
    }
            
    if (other.pid_count > 0) 
    {
        printf("%-25s %-12.2f %-6d\n",
               other.name.c_str(),
               other.total_cpu_time / 1e9,
               other.pid_count);
    }

    printf("\nTotal Unique Apps: %zu\n", aggregated.size() + (other.pid_count > 0 ? 1 : 0));
    return 0;
}
