#!/bin/bash

IFACE=$(ip route get 8.8.8.8 | awk '{for(i=1;i<=NF;i++) if ($i=="dev") print $(i+1); exit}')

if [ -z "$IFACE" ]; then
  echo "Could not detect network interface."
  exit 1
fi

tc qdisc add dev "$IFACE" clsact 2>/dev/null

tc filter add dev "$IFACE" egress bpf da obj throttler.bpf.o sec classifier

exit 0