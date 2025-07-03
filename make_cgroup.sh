#!/bin/bash

sudo mount -t cgroup2 none /sys/fs/cgroup
sudo mkdir /sys/fs/cgroup/$1
