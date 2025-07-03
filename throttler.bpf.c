#include "throttler.bpf.h"

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u64); //cid
    __type(value, struct rate_limit);
    __uint(max_entries, 1024);
} token_buckets SEC(".maps");

static __always_inline int throttle(__u32 len, __u64 cid){
    bpf_printk("hello");
    struct rate_limit *bucket = bpf_map_lookup_elem(&token_buckets, &cid);
    if (!bucket) return 1;

    if (bucket->rate == 0) return 1; // if the rate is 0- the filter is off

    u64 now = bpf_ktime_get_ns();
    u64 delta_ns = now - bucket->last_time;

    u64 hyp_tokens = bucket->tokens + (delta_ns * bucket->rate)/100000000ULL;
    bucket->tokens = hyp_tokens < bucket->max_tokens ? hyp_tokens : bucket->max_tokens;

    if (bucket->tokens < len){
        return 0; // drop
    }

    bucket->last_time = now;
    bucket->tokens -= len;
    return 1;
}


SEC("cgroup_skb/ingress")
int throttler_ingress(struct __sk_buff* skb){
    __u64 cid = bpf_get_current_cgroup_id();
    return throttle(skb->len, cid);
}



SEC("cgroup_skb/egress")
int throttler_egress(struct __sk_buff *skb) {
    __u64 cid = bpf_get_current_cgroup_id();
    return throttle(skb->len, cid);
}
