// cpu_optimizer_backend.cpp
#include "./utils/cpu_backend_utils.h"
#include <sys/resource.h>
#include <signal.h>
#include <iostream>
#include <map>
#include <algorithm>

#include "Icpu_optimizer.h"

using namespace std;

// Feature 1: Adjust nice value (priority)
bool set_nice_for_pid(pid_t pid, int nice_value) {
    return setpriority(PRIO_PROCESS, pid, nice_value) == 0;
}

bool set_nice_for_group(const ProcessGroup& group, int nice_value) {
    bool success = true;
    for (pid_t pid : group.pids) {
        if (!set_nice_for_pid(pid, nice_value)) {
            perror(("Failed to renice PID " + std::to_string(pid)).c_str());
            success = false;
        }
    }
    return success;
}

// Feature 2: Pause/resume processes
bool pause_group(const ProcessGroup& group) {
    bool success = true;
    for (pid_t pid : group.pids) {
        if (kill(pid, SIGSTOP) != 0) {
            perror(("Failed to pause PID " + std::to_string(pid)).c_str());
            success = false;
        }
    }
    return success;
}

bool resume_group(const ProcessGroup& group) {
    bool success = true;
    for (pid_t pid : group.pids) {
        if (kill(pid, SIGCONT) != 0) {
            perror(("Failed to resume PID " + std::to_string(pid)).c_str());
            success = false;
        }
    }
    return success;
}

// Feature 3: Optimize (composite action)
bool optimize_group(const ProcessGroup& group) {
    if (group.pids.size() > 3 && group.total_cpu_time > 5e9) {
        cout << "Optimizing group: " << group.name << "\n";
        return set_nice_for_group(group, 10);
    }
    return false;
}

void optimize_all_heavy_processes() {
    auto whitelist = load_whitelist();
    auto groups = read_process_groups_from_map();
    for (const auto& group : groups) {
        if (whitelist.count(group.first)) 
        {
            std::cout << "Skipping whitelisted: " << group.first << "\n";
            continue;
        }
        optimize_group(group.second);
    }
}

// Feature 4: Optimize specific process by name and level
bool optimize_process(const string& name, const string& level) {
    int nice_val = 0;
    if (level == "high") nice_val = 0;
    else if (level == "med") nice_val = 5;
    else if (level == "low") nice_val = 10;
    else {
        cerr << "Invalid priority level: " << level << "\n";
        return false;
    }

    auto groups = read_process_groups_from_map();
    auto it = groups.find(name);
    if (it != groups.end()) {
        return set_nice_for_group(it->second, nice_val);
    } else {
        cerr << "Process " << name << " not found.\n";
        return false;
    }
}

// Feature 5: Restore a specific process to normal priority
bool restore_process(const string& name) {
    auto groups = read_process_groups_from_map();
    auto it = groups.find(name);
    if (it != groups.end()) {
        return set_nice_for_group(it->second, 0);
    } else {
        cerr << "Process " << name << " not found.\n";
        return false;
    }
}

// Feature 6: Restore all to normal
void restore_all() {
    auto groups = read_process_groups_from_map();
    for (const auto& group : groups) {
        set_nice_for_group(group.second, 0);
    }
}

void add_to_whitelist_interface(const string& name)    
{
    add_to_whitelist(name);
}

void remove_from_whitelist_interface(const string& name)
{
    remove_from_whitelist(name);
}

set<string> show_all_whitelist()
{
    set<string> whitelist = load_whitelist();
    return whitelist;
}


// int main() {
//     cout << "Optimizing all heavy processes if Y is entered: ";
//     char s;
//     cin >> s;
//     if (s == 'Y') {
//         optimize_all_heavy_processes();
//     }

//     auto groups = read_process_groups_from_map();
//     for (const auto& pair : groups) {
//         const ProcessGroup& group = pair.second;
//         if (group.name == "code" || group.name == "brave") {
//             pause_group(group);
//             sleep(5);
//             resume_group(group);
//         }
//     }

    // Example usage:
    // optimize_process("brave", "low");
    // restore_process("brave");
    // restore_all();

//     return 0;
// }
