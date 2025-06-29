#define BPF_NO_GLOBAL_DATA

// #include <linux/bpf.h>
// #include <linux/types.h>
#include<vmlinux.h>
#include <bpf/bpf_helpers.h> 
#include <bpf/bpf_tracing.h>

// typedef unsigned int u32;
// typedef unsigned long u64;
typedef int pid_t;
const pid_t pid_filter = 0;

typedef struct net_stats_t{
    u64 bytes_sent;
    u64 bytes_recvd;
    u64 total_bytes;
} net_stats_t;

struct{
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10240);
    __type(key, u32); // PID
    __type(value, net_stats_t); // bytes transferred
} pid_bytes SEC(".maps");

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("kprobe/tcp_sendmsg")
int BPF_KPROBE(tcp_sendmsg, struct sock* sk, struct msghdr *msg, size_t size){
    u32 pid = bpf_get_current_pid_tgid() >> 32;
    net_stats_t* stats_struct = bpf_map_lookup_elem(&pid_bytes, &pid);
    u64 new_bytes_sent = size;
    
    if (stats_struct){
        stats_struct->bytes_sent += new_bytes_sent;
        stats_struct-> total_bytes += new_bytes_sent;
    }
    else{
        net_stats_t new_struct = {
            .bytes_sent = new_bytes_sent,
            .bytes_recvd = 0,
            .total_bytes = new_bytes_sent
        };
        
        bpf_map_update_elem(&pid_bytes, &pid, &new_struct, BPF_ANY);
    }
    

    // make a queue of the pid wise network activity. need to add time checks in the user space as well tho.

    
    return 0;
    

}