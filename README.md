# Sysopt - Advanced Linux System Monitor


An advanced system monitoring tool designed exclusively for Linux. It provides a beautiful Electron-based graphical interface and powerful command-line tools to inspect CPU, RAM, and Network activity in real-time, all powered by the incredible performance of eBPF.

### Demo Video: 
- https://github.com/SiddharthChopra28/mdg-hackathon-proj/blob/main/final_final.webm

Features

📊 Real-time Metrics: Monitor CPU usage, process activity, memory consumption, and network traffic as it happens.

🚀 Blazing Fast: Leverages eBPF to collect data directly from the Linux kernel, ensuring minimal performance overhead.

🖥️ Intuitive GUI: A clean and modern user interface built with Electron provides a visual overview of your system's health.

💻 Powerful CLI: For terminal enthusiasts, access all monitoring data through dedicated command-line daemons.

🔧 Simple Installation: A single script handles dependency installation, compilation, and system service setup.

🐧 Linux First: Built from the ground up to take advantage of modern Linux kernel capabilities.

Why eBPF?

eBPF (Extended Berkeley Packet Filter) is a revolutionary technology that allows us to run sandboxed programs directly inside the Linux kernel. This is the "secret sauce" of our project.

Unprecedented Insight: By running in the kernel, our probes can access data at its source, providing a level of detail and accuracy that traditional tools can't match.

High Performance: eBPF programs are JIT-compiled and optimized, introducing negligible overhead. Your system won't slow down, even while under heavy monitoring.

Safe & Secure: The eBPF verifier statically analyzes any program before it's loaded, guaranteeing that it won't crash or compromise the kernel.

Installation

The installation process is streamlined into a single script. You must run this from the root of the project directory.

Prerequisites:
You will need git to clone the repository. The installation script will automatically handle the installation of all other required dependencies, such as cmake, g++, clang, and libbpf-dev.

Steps:

1. Clone the repository:

git clone https://github.com/SiddharthChopra28/mdg-hackathon-proj/tree/cpu-ebpf/ryuga


2. Navigate into the project directory



3. Run the installer with sudo:
You must stay in the root folder of the project for the script to find all necessary files.

sudo ./install.sh


The script will ask for confirmation before proceeding. It needs sudo privileges to:

Install system-wide dependencies using apt.

Copy the compiled backend binary to /usr/local/bin.

Set up and start the systemd service for background monitoring.

# Usage

Once installed, you can monitor your system through either the GUI or the CLI.

# Graphical User Interface (GUI)

The installation script sets up the backend services. To run the frontend GUI:

1. Navigate to the frontend directory:

cd frontend


2. Install frontend dependencies:

npm install


3. Launch the app:

npm start

# Command-Line Interface (CLI)

The backend daemons can also be queried directly from any terminal for quick, scriptable access to system data.


-=CPU Monitoring:

# Get the top 5 CPU-consuming processes
cpu-cli --top 5

# Get a summary of CPU core usage
cpu-cli --summary

-=RAM Monitoring:

# See a list of processes sorted by memory usage
ram-cli --list

# Get total, used, and free memory
ram-cli --usage


-=Network Monitoring:

# Monitor incoming and outgoing traffic per process
net-cli --proc

# Uninstallation

We provide a simple script to cleanly remove all components of this project from your system.

To uninstall, run the following command from the project's root directory:

sudo ./uninstall.sh


This will:

Stop and disable the systemd service.

Remove the service file from /etc/systemd/system.

Delete the binary from /usr/local/bin.

Project Structure
.
├── backend/         # C++ source code for the eBPF probes and IPC server
├── frontend/        # Electron.js and React source code for the GUI
├── system/          # systemd service files
├── install.sh       # The main installation script
└── uninstall.sh     # The uninstallation script


## About UI
Given below is the link to detailed Design Documentation for the project.
- https://docs.google.com/document/d/19uo_4_5VD_2j-vM7-A1TqnLOeI1j2yVPNYDVGxjiOgY/edit?usp=sharing
