#!/bin/bash

echo "Welcome to the CPU Optimizer installer!"
echo
echo "This tool tracks CPU usage via eBPF and lets you throttle heavy apps."
echo "To do this, it must:"
echo "  - Load a pinned eBPF program on boot"
echo "  - Start a privileged socket server for safe frontend access"
echo
read -p "Do you want to proceed and install as a systemd service? (y/n) " choice

if [[ "$choice" != "y" ]]; then
  echo "Installation aborted."
  exit 1
fi

# Ask for sudo permission only here
sudo -v || { echo "Sudo required. Exiting."; exit 1; }

# Copy binary to /usr/local/bin
sudo cp backend/ipc_server /usr/local/bin/cpu_optimizer_daemon
sudo chmod +x /usr/local/bin/cpu_optimizer_daemon

# Copy service file
sudo cp system/cpu_optimizer.service /etc/systemd/system/cpu_optimizer.service

# Enable and start the service
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable cpu_optimizer.service
sudo systemctl start cpu_optimizer.service

echo "✅ Installed and started successfully!"
echo "You can check logs using: sudo journalctl -u cpu_optimizer -f"
