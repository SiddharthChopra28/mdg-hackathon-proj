#define BPF_NO_GLOBAL_DATA
#define __TARGET_ARCH_x86



#include"vmlinux.h"
#include <bpf/bpf_helpers.h> 
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

uint8_t zero = 0;

typedef struct net_data{
    int pid;
    unsigned long long timestamp_ns;
    int bytes; // + for sent, - for recvd
} net_data;