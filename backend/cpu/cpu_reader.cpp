// cpu_reader.cpp

#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <limits.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <nlohmann/json.hpp>
#include "Icpu_optimizer.h"
#include "./utils/cpu_backend_utils.h"

using json = nlohmann::json;
using namespace std;

#define MAP_PATH "/sys/fs/bpf/cpu_times"
#define MAX_NAME_LEN 256
#define IGNORE_CPU_THRESHOLD_NS 2000000000ULL // 2 seconds

struct ProcessInfo
{
    string name;
    uint64_t total_cpu_time;
    int pid_count;
};

struct CpuInfo
{
    string model_name;
    double base_speed_ghz = 0.0;
    int core_count = 0;
};

CpuInfo get_cpu_info()
{
    ifstream file("/proc/cpuinfo");
    string line;
    CpuInfo info;
    int cores = 0;

    while (getline(file, line))
    {
        if (line.find("model name") != string::npos && info.model_name.empty())
        {
            auto pos = line.find(":");
            if (pos != string::npos)
                info.model_name = line.substr(pos + 2);
        }

        if (line.find("cpu MHz") != string::npos && info.base_speed_ghz == 0.0)
        {
            auto pos = line.find(":");
            if (pos != string::npos)
            {
                try
                {
                    info.base_speed_ghz = stod(line.substr(pos + 2)) / 1000.0;
                }
                catch (...)
                {
                    info.base_speed_ghz = -1.0;
                }
            }
        }

        if (line.find("processor") != string::npos)
            cores++;
    }

    info.core_count = cores;
    return info;
}

uint64_t get_total_cpu_time()
{
    ifstream file("/proc/stat");
    string line;
    getline(file, line); // "cpu  ..."

    istringstream iss(line);
    string cpu;
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    return user + nice + system + idle + iowait + irq + softirq + steal;
}

json get_processor_data()
{
    json result;

    CpuInfo info = get_cpu_info();
    result["model_name"] = info.model_name;
    result["base_speed_ghz"] = info.base_speed_ghz;
    result["core_count"] = info.core_count;

    return result;
}

json get_process_data()
{
    static map<string, uint64_t> last_proc_times;
    static uint64_t last_total_cpu_time = get_total_cpu_time();

    map<string, uint64_t> current_proc_times;
    uint64_t current_total_cpu_time = get_total_cpu_time();
    uint64_t delta_total_ticks = current_total_cpu_time - last_total_cpu_time;

    long ticks_per_sec = sysconf(_SC_CLK_TCK);
    double delta_total_ns = (double)delta_total_ticks * (1e9 / ticks_per_sec);

    json result = json::array();
    int map_fd = bpf_obj_get(MAP_PATH);
    if (map_fd < 0)
    {
        return "Failed to open BPF map\n";
    }

    vector<ProcessInfo> aggregated;
    ProcessInfo other = {"Other", 0, 0};

    __u32 pid = -1, next_pid;
    __u64 cpu_time;

    while (bpf_map_get_next_key(map_fd, &pid, &next_pid) == 0)
    {
        if (bpf_map_lookup_elem(map_fd, &next_pid, &cpu_time) != 0)
        {
            pid = next_pid;
            continue;
        }

        char path[64], name[MAX_NAME_LEN];
        snprintf(path, sizeof(path), "/proc/%u/exe", next_pid);
        char resolved_path[PATH_MAX];
        ssize_t len = readlink(path, resolved_path, sizeof(resolved_path) - 1);
        if (len == -1)
        {
            pid = next_pid;
            continue;
        }
        resolved_path[len] = '\0';

        const char *base = strrchr(resolved_path, '/');
        if (!base || strlen(base + 1) >= MAX_NAME_LEN)
        {
            pid = next_pid;
            continue;
        }
        strncpy(name, base + 1, MAX_NAME_LEN);
        name[strcspn(name, "\n")] = '\0';

        if (is_kernel_thread(next_pid) || is_root_process(next_pid))
        {
            pid = next_pid;
            continue;
        }

        if (cpu_time < IGNORE_CPU_THRESHOLD_NS)
        {
            other.total_cpu_time += cpu_time;
            other.pid_count += 1;
            pid = next_pid;
            continue;
        }

        auto it = find_if(aggregated.begin(), aggregated.end(), [&](const ProcessInfo &p)
                          { return p.name == name; });

        if (it != aggregated.end())
        {
            it->total_cpu_time += cpu_time;
            it->pid_count += 1;
        }
        else
        {
            aggregated.push_back({name, cpu_time, 1});
        }

        pid = next_pid;
    }
    static auto last_ts = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double poll_interval_sec = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_ts).count();
    last_ts = now;

    int core_count = get_cpu_info().core_count;

    for (const auto &proc : aggregated)
    {
        current_proc_times[proc.name] = proc.total_cpu_time;

        double percent = 0.0;
        auto it = last_proc_times.find(proc.name);
        if (it != last_proc_times.end() && delta_total_ns > 0.0)
        {
            uint64_t delta_proc_time = proc.total_cpu_time - it->second;
            percent = (double)delta_proc_time / delta_total_ns * 100.0;
            percent /= (double)core_count;
        }

        result.push_back({{"name", proc.name},
                          {"cpu_time", proc.total_cpu_time / 1e9},
                          {"pid_count", proc.pid_count},
                          {"cpu_percent", percent}});
    }

    last_proc_times = current_proc_times;
    last_total_cpu_time = current_total_cpu_time;

    return result;
}
