#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#define MAP_PATH "/sys/fs/bpf/cpu_times"

#define MAX_PROCESSES 4096      
#define MAX_NAME_LEN  256
#define IGNORE_CPU_THRESHOLD_NS 2000000000ULL  // 2 seconds

struct ProcessInfo {
    char name[MAX_NAME_LEN];
    uint64_t total_cpu_time;
    int pid_count;
};

const char *protected_names[] = {
    "init", "systemd", "kworker", "migration", "rcu_sched", "watchdog",
    "Thread<", "ThreadPool", "QDBusConnection", "QSGRenderThread",
    "ksgrd_", "org_kde_", "krunner_", "libinput-", "plasma-",
    "kded6", "xdg-", "gmenudbusmenupr", "fcitx", "rs:", "in:imuxsock",
    "GUsbEventThread", "MemoryInfra", NULL
};

int is_protected_process(const char *name) {
    for (int i = 0; protected_names[i]; i++) {
        if (strncmp(name, protected_names[i], strlen(protected_names[i])) == 0)
            return 1;
    }
    return 0;
}

int is_kernel_thread(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    return access(path, F_OK) != 0;
}

int find_or_create(struct ProcessInfo *list, int *size, const char *name) {
    for (int i = 0; i < *size; i++) {
        if (strcmp(list[i].name, name) == 0)
            return i;
    }
    if (*size >= MAX_PROCESSES) return -1;
    strncpy(list[*size].name, name, MAX_NAME_LEN - 1);
    list[*size].name[MAX_NAME_LEN - 1] = '\0';
    list[*size].total_cpu_time = 0;
    list[*size].pid_count = 0;
    return (*size)++;
}

int compare_cpu(const void *a, const void *b) {
    uint64_t ta = ((struct ProcessInfo *)a)->total_cpu_time;
    uint64_t tb = ((struct ProcessInfo *)b)->total_cpu_time;
    return (tb > ta) - (tb < ta);
}

int main() {
    int map_fd = bpf_obj_get(MAP_PATH);
    if (map_fd < 0) {
        perror("Failed to open BPF map");
        return 1;
    }

    struct ProcessInfo aggregated[MAX_PROCESSES];
    int agg_size = 0;

    struct ProcessInfo other = { .name = "Other", .total_cpu_time = 0, .pid_count = 0 };

    __u32 pid = -1, next_pid;
    __u64 cpu_time;

    while (bpf_map_get_next_key(map_fd, &pid, &next_pid) == 0) {
        if (bpf_map_lookup_elem(map_fd, &next_pid, &cpu_time) != 0) {
            pid = next_pid;
            continue;
        }

        char path[64], name[MAX_NAME_LEN];
        snprintf(path, sizeof(path), "/proc/%u/comm", next_pid);
        FILE *f = fopen(path, "r");
        if (!f || !fgets(name, sizeof(name), f)) {
            pid = next_pid;
            if (f) fclose(f);
            continue;
        }
        fclose(f);
        name[strcspn(name, "\n")] = '\0';

        if (is_kernel_thread(next_pid) || is_protected_process(name)) {
            pid = next_pid;
            continue;
        }

        if (cpu_time < IGNORE_CPU_THRESHOLD_NS) {
            other.total_cpu_time += cpu_time;
            other.pid_count += 1;
            pid = next_pid;
            continue;
        }

        int idx = find_or_create(aggregated, &agg_size, name);
        if (idx >= 0) {
            aggregated[idx].total_cpu_time += cpu_time;
            aggregated[idx].pid_count += 1;
        }

        pid = next_pid;
    }

    qsort(aggregated, agg_size, sizeof(struct ProcessInfo), compare_cpu);

    printf("%-25s %-12s %-6s\n", "Process Name", "CPU Time(s)", "PIDs");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < agg_size; i++) {
        printf("%-25s %-12.2f %-6d\n",
               aggregated[i].name,
               aggregated[i].total_cpu_time / 1e9,
               aggregated[i].pid_count);
    }

    if (other.pid_count > 0) {
        printf("%-25s %-12.2f %-6d\n",
               other.name,
               other.total_cpu_time / 1e9,
               other.pid_count);
    }

    printf("\nTotal Unique Apps: %d\n", agg_size + (other.pid_count > 0 ? 1 : 0));
    return 0;
}
