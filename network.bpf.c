#include "network.bpf.h"

struct{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 10240);
} netdata_ringbuf SEC(".maps");


SEC("kprobe/tcp_sendmsg")
int BPF_KPROBE(tcp_sendmsg, struct sock* sk, struct msghdr *msg, size_t size){
    u32 pid = bpf_get_current_pid_tgid() >> 32;

    Elf64_Addr trigger_time = bpf_ktime_get_ns();
    
    struct net_data* new = bpf_ringbuf_reserve(&netdata_ringbuf, sizeof(net_data), 0);
    if (!new){
        return -1;
    }
    new->pid = pid;
    new->timestamp_ns = trigger_time;
    new->bytes = size;

    bpf_ringbuf_submit(new, 0);

    return 0;
}

SEC("kprobe/tcp_recvmsg")
int BPF_KPROBE(tcp_recvmsg, struct sock* sk, struct msghdr *msg, size_t size){
    u32 pid = bpf_get_current_pid_tgid() >> 32;

    Elf64_Addr trigger_time = bpf_ktime_get_ns();
    
    struct net_data* new = bpf_ringbuf_reserve(&netdata_ringbuf, sizeof(net_data), 0);
    if (!new){
        return -1;
    }
    new->pid = pid;
    new->timestamp_ns = trigger_time;
    new->bytes = -size;

    bpf_ringbuf_submit(new, 0);

    return 0;

}