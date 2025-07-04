#include "ipc_server.hpp"

const char *SOCKET_PATH = "/tmp/ram_optimizer.sock";
int server_fd = -1;
static volatile bool running = true;

void signal_handler(int signum)
{
    std::cout << "\nCaught signal, shutting down server..." << std::endl;
    running = false;
}

void cleanup()
{
    if (server_fd != -1)    
        close(server_fd);
    unlink(SOCKET_PATH);
}

bool is_numeric(const char *s)
{
    if (!s || *s == '\0')
        return false;
    while (*s)
    {
        if (!isdigit(*s))
            return false;
        s++;
    }
    return true;
}

json handle_ram_usage()
{
    FILE *fp_mem = fopen("/proc/meminfo", "r");
    char line[256];
    long total_kb = 0, available_kb = 0;

    if (fp_mem)
    {
        while (fgets(line, sizeof(line), fp_mem))
        {
            sscanf(line, "MemTotal: %ld kB", &total_kb);
            sscanf(line, "MemAvailable: %ld kB", &available_kb);
        }
        fclose(fp_mem);
    }

    double total_gb = total_kb / 1024.0 / 1024.0;
    double used_gb = (total_kb - available_kb) / 1024.0 / 1024.0;
    double percent = used_gb / total_gb * 100.0;

    return {
        {"name", "system_ram"},
        {"total_gb", round(total_gb * 100) / 100},
        {"used_gb", round(used_gb * 100) / 100},
        {"percent_used", round(percent * 100) / 100}};
}

json handle_top_processes()
{
    struct ProcessInfo
    {
        int pid;
        char name[256];
        long ram_kb;
    };

    auto compareByRam = [](const ProcessInfo &a, const ProcessInfo &b)
    {
        return b.ram_kb < a.ram_kb;
    };

    std::vector<ProcessInfo> processes;

    DIR *proc_dir = opendir("/proc");
    struct dirent *entry;

    while ((entry = readdir(proc_dir)) != nullptr)
    {
        if (entry->d_type == DT_DIR && is_numeric(entry->d_name))
        {
            ProcessInfo proc = {};
            proc.pid = atoi(entry->d_name);

            char path[512];
            FILE *fp;

            // 1. Get RAM usage
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            fp = fopen(path, "r");
            if (fp)
            {
                char line[256];
                while (fgets(line, sizeof(line), fp))
                {
                    if (strncmp(line, "VmRSS:", 6) == 0)
                    {
                        sscanf(line, "VmRSS:\t%ld kB", &proc.ram_kb);
                        break;
                    }
                }
                fclose(fp);
            }
            else
            {
                continue;
            }

            // 2. Get Process name
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
            fp = fopen(path, "r");
            if (fp)
            {
                fgets(proc.name, sizeof(proc.name), fp);
                strtok(proc.name, "\n");
                fclose(fp);
            }
            else
            {
                strcpy(proc.name, "N/A");
            }

            processes.push_back(proc);
        }
    }

    closedir(proc_dir);

    // Sort descending by ram_kb
    std::sort(processes.begin(), processes.end(), compareByRam);

    int display_count = std::min((int)processes.size(), 20);
    json j;
    j["name"] = "top 20 processes";

    for (int i = 0; i < display_count; ++i)
    {
        j[std::to_string(i + 1)] = {
            {"pid", processes[i].pid},
            {"ram_kb", processes[i].ram_kb},
            {"name", processes[i].name}};
    }

    return j;
}