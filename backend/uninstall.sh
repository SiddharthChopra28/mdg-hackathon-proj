#!/bin/bash

echo "Uninstalling CPU Optimizer..."

sudo systemctl stop cpu_optimizer.service
sudo systemctl disable cpu_optimizer.service
sudo rm -f /etc/systemd/system/cpu_optimizer.service
sudo rm -f /usr/local/bin/cpu_optimizer_daemon
sudo systemctl daemon-reload

echo "✅ Uninstalled successfully."
