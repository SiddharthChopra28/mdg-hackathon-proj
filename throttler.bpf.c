#include "throttler.bpf.h"

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32); //pid
    __type(value, rate_limit_t);
    __uint(max_entries, 1024);
} token_buckets SEC(".maps");

SEC("classifier")
int throttle(struct __sk_buff *skb){
    u32 pid = bpf_get_current_pid_tgid() >> 32;

    rate_limit_t *bucket = bpf_map_lookup_elem(&token_buckets, &pid);
    if (!bucket){
        return BPF_OK;
    }

    u64 now = bpf_ktime_get_ns();
    u64 delta_ns = now - bucket->last_time;

    u64 hyp_tokens = bucket->tokens + (delta_ns * bucket->rate)/100000000ULL;
    bucket->tokens = hyp_tokens < bucket->max_tokens ? hyp_tokens : bucket->max_tokens;

    bucket->last_time = now;

    if (bucket->tokens > skb->len){
        bucket->tokens -= skb->len;
        return BPF_OK;
    }
    return BPF_DROP;

}