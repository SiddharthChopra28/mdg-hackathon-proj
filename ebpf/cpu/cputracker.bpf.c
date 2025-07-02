/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual MIT/GPL";

// Store CPU time per PID
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);          // PID
    __type(value, u64);        // Time in ns
    __uint(max_entries, 10240);
    __uint(pinning, LIBBPF_PIN_BY_NAME);  // Add this line for pinning
} cpu_times SEC(".maps");

SEC("tracepoint/sched/sched_switch")
int handle_sched_switch(struct trace_event_raw_sched_switch *ctx) {
    u64 now = bpf_ktime_get_ns();

    u32 prev_pid = ctx->prev_pid;
    u64 *start_time = bpf_map_lookup_elem(&cpu_times, &prev_pid);
    if (start_time) {
        u64 delta = now - *start_time;
        *start_time += delta;
    }

    u32 next_pid = ctx->next_pid;
    bpf_map_update_elem(&cpu_times, &next_pid, &now, BPF_ANY);

    bpf_printk("Switched from %d to %d\n", prev_pid, next_pid);

    return 0;
}