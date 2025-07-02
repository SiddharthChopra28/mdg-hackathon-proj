// cpu_optimizer_backend.cpp
// This is the high-level backend interface for applying actions on process groups

#include "./utils/cpu_backend_utils.h"
#include <sys/resource.h>
#include <signal.h>
#include <iostream>
#include <map>

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
    // Example policy: If more than 3 PIDs and CPU time > 5 seconds, lower priority
    if (group.pids.size() > 3 && group.total_cpu_time > 5e9) {
        std::cout << "Optimizing group: " << group.name << "\n";
        return set_nice_for_group(group, 10);
    }
    return false;
}

void optimize_all_heavy_processes() {
    auto groups = read_process_groups_from_map();
    for (const auto& group : groups) {
        optimize_group(group.second);
    }
}


int main()
{
    cout << "Optimizing all heavy processes if Y is entered" << endl;
    char s;
    cin >> s;
    if(s== 'Y')
    {
        optimize_all_heavy_processes();
    }

    auto groups = read_process_groups_from_map();
    for (const auto& pair : groups) {
        const ProcessGroup& group = pair.second;
        if (group.name == "code" || group.name == "brave") {
            pause_group(group);
            sleep(5);
            resume_group(group);
        }
    }
    return 0;
}