#!/bin/bash

sudo bpftool prog loadall throttler.bpf.o /sys/fs/bpf/throttle_prog

map_id=$(sudo bpftool map list | grep -A1 'name token_buckets' | head -n1 | cut -d: -f1)

sudo bpftool map pin id ${map_id} /sys/fs/bpf/token_buckets
