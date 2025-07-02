#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
// sudo apt install nlohmann-json3-dev
// #include <nlohmann/json.hpp>

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
    std::cout << "--------------------------------\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Percent Used: " << percent_used
          << "  USED: " << used_gb << "GB of " << total_gb << "GB\n";

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
    while (true) {
        log_system_ram();
    }
    return 0;
}
