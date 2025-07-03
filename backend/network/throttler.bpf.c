#include "throttler.bpf.h"

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u64); //cid
    __type(value, struct rate_limit);
    __uint(max_entries, 1024);
} token_buckets SEC(".maps");

static __always_inline int throttle(__u32 len, __u64 cid){
    // bpf_printk("hi");
    struct rate_limit *bucket = bpf_map_lookup_elem(&token_buckets, &cid);
    if (!bucket){
        // bpf_printk("the bucket doesnt exist");
        return 1;
    } 

    if (bucket->rate == 0) return 1; // if the rate is 0- the filter is off

    bpf_printk("tokens in bucket: %u", bucket->tokens);   

    bpf_printk("size of packet: %u", len);


    if (bucket->tokens < len){
        bpf_printk("i dropped a packet");   
        return 0; // drop
    }

    bucket->tokens -= len;
    bpf_map_update_elem(&token_buckets, &cid, bucket, BPF_ANY);

    // bpf_printk("didnt drop the packet");
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
