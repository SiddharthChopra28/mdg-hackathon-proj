#!/bin/bash

# =============================================================================
#           orion Application 
# This script uses CMake to build and install the backend services.
#-Check Status: systemctl status orion.service
#-Stop the Service: sudo systemctl stop orion.service
#-Start the Service: sudo systemctl start orion.service
#-Disable Auto-Start: sudo systemctl disable orion.service
# =============================================================================

# --- Step 1: Check for Root Permissions ---
if [ "$EUID" -ne 0 ]; then
  echo "This script must be run with sudo or as root."
  exit 1
fi

echo "--- Starting Ryuga backend installation ---"

# --- Step 2: Install Dependencies ---
echo "[1/5] Installing dependencies (cmake, g++, clang, libbpf)..."
apt-get update
# Add cmake to the list of dependencies
apt-get install -y cmake build-essential make g++ clang libbpf-dev libelf-dev nlohmann-json3-dev wget || { echo "ERROR: Failed to install dependencies."; exit 1; }

# --- Step 3: Build the Project using CMake ---
echo "[2/5] Configuring project with CMake and compiling..."

if [ ! -x ./ecc ]; then
    wget https://github.com/eunomia-bpf/eunomia-bpf/releases/latest/download/ecc || {
        echo "ERROR: Failed to download ecc."; exit 1; }
    chmod +x ./ecc
else
    echo "ecc already exists and is executable, skipping download."
fi
rm -rf build
mkdir build
cd build

cmake .. || { echo "ERROR: CMake configuration failed."; exit 1; }
make || { echo "ERROR: Compilation with make failed."; exit 1; }

echo "Compilation successful."

# --- Step 4: Install the Compiled Files using 'make install' ---

echo "[3/5] Installing application files to system directory..."
make install || { echo "ERROR: Failed to install files."; exit 1; }

cd ..
echo "Files installed successfully."


# --- Step 5: Create and Enable the systemd Service ---
echo "[4/5] Creating and configuring systemd service..."
INSTALL_DIR="/usr/local/bin/orion" 

cat > /etc/systemd/system/orion.service <<EOF
[Unit]
Description=Orion Backend Service (IPC Server and Data Loggers)
After=network.target

[Service]
# Important: Set the working directory so programs can find their files,
# especially the mem_events_processor finding its .bpf.o file.
WorkingDirectory=$INSTALL_DIR

# The 'ExecStart' chain will run all our services.
# We start the server first, then use a little trick with 'sh -c'
# to launch the clients in the background so they don't block each other.
ExecStart=/bin/sh -c '/usr/local/bin/orion/ipc_server & /usr/local/bin/orion/mem_ev_processor & /usr/local/bin/orion/ram_logger & /usr/local/bin/orion/pp_ram_log && wait'

# Run the service as root, which is required for loading eBPF programs.
User=root
Restart=always
RestartSec=3


[Install]
WantedBy=multi-user.target
EOF

echo "Service file created."

# Reload the systemd daemon, then enable and start our new service.
echo "[5/5] Enabling and starting the Orion service..."
systemctl daemon-reload
systemctl enable orion.service
systemctl start orion.service

systemctl status orion.service --no-pager

echo ""
echo "======================================================================="
echo " Orion backend installation complete!"
echo " The services are now running automatically in the background."
echo "======================================================================="

exit 0