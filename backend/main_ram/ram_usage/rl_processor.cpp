#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
// sudo apt install nlohmann-json3-dev
#include <nlohmann/json.hpp>
//-----sockets-----
// This function sends a JSON string to our central server
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

using json = nlohmann::json;

void send_data_to_server(const std::string& json_output) {
    const char* SOCKET_PATH = "/tmp/ram_orion.sock"; // Must match the server's path
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "ERROR: Failed to create socket." << std::endl;   
        return;
    } // Don't crash if server isn't running

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        send(fd, json_output.c_str(), json_output.length(), 0);
    } else {    
        std::cerr << "ERROR: Failed to connect to server." << std::endl;
    }
    
    close(fd);
}
//--------------------

void log_system_ram(){
FILE *fp_meminfo, *fp_stats;
    char line[256];
    long total_kb = 0, available_kb = 0;

    fp_meminfo = fopen("/proc/meminfo", "r");
    if (!fp_meminfo) return;
    while (fgets(line, sizeof(line), fp_meminfo)) {// Read each line from /proc/meminfo and passes into line
        if (sscanf(line, "MemTotal: %ld kB", &total_kb) == 1) continue; // sscanf reads formatted input from a string
        if (sscanf(line, "MemAvailable: %ld kB", &available_kb) == 1) break;
    }
    fclose(fp_meminfo);

    
    if (total_kb == 0) return;
    
    double used_gb = (total_kb - available_kb)/1024.0 / 1024.0; 
    double total_gb = total_kb / 1024.0 / 1024.0; 
    double percent_used = used_gb / total_gb * 100.0;
    
    json j;
    j["name"] = "system_ram";
    j["total_gb"] = std::round(total_gb * 100.0) / 100.0; // Round to 2 decimal places
    j["used_gb"] = std::round(used_gb * 100.0) / 100.0;
    j["percent_used"] = std::round(percent_used * 100.0) / 100.0;
    std::string json_output = j.dump();
    std::cout << json_output << std::endl;
    send_data_to_server(json_output);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// int main() {
//     while (true) {
//         log_system_ram();
//     }
//     return 0;
// }
