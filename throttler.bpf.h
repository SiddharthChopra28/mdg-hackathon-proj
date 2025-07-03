#include"vmlinux.h"
#include <bpf/bpf_helpers.h> 
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct rate_limit{
    u64 rate; // bytes per sec
    u64 max_tokens; 
    u64 tokens;
    u64 last_time;
};

