sudo bpftool cgroup detach /sys/fs/cgroup/ ingress pinned /sys/fs/bpf/throttle_prog/throttler_ingress
sudo bpftool cgroup detach /sys/fs/cgroup/ egress pinned /sys/fs/bpf/throttle_prog/throttler_egress

sudo rm -rf /sys/fs/bpf/throttle_prog /sys/fs/bpf/token_buckets
