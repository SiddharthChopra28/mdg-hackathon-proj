// #include <linux/bpf.h>
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

typedef int pid_t;
const volatile pid_t pid_filter = 0;

char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct ram_event_t {
    pid_t pid;
    char comm[16];
    int event_type; // 1 for page fault, 2 for OOM kill
};


struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(int));
    __uint(value_size, sizeof(int));
} ram_events SEC(".maps");

// === Page Fault Monitoring ===
SEC("kprobe/handle_mm_fault")
int BPF_KPROBE(handle_page_fault)
{
    pid_t pid = bpf_get_current_pid_tgid() >> 32;
    if (pid_filter && pid != pid_filter) {
        return 0;
    }

    struct ram_event_t event = {};

    event.pid = pid;
    event.event_type = 1; // 1 = Page Fault
    bpf_get_current_comm(&event.comm, sizeof(event.comm));

    // 3. Submit the event to user-space
    bpf_perf_event_output(ctx, &ram_events, BPF_F_CURRENT_CPU, &event, sizeof(event));

    return 0;
}

// === OOM Killer Monitoring ===
SEC("kprobe/oom_kill_process")
int BPF_KPROBE(handle_oom_kill)
{
    // 1. Create our event data structure
    struct ram_event_t event = {};

    // 2. Populate the event data
    event.pid = bpf_get_current_pid_tgid() >> 32;
    event.event_type = 2; // 2 = OOM Kill
    bpf_get_current_comm(&event.comm, sizeof(event.comm));

    // 3. Submit the event to user-space
    bpf_perf_event_output(ctx, &ram_events, BPF_F_CURRENT_CPU, &event, sizeof(event));

    return 0;
}