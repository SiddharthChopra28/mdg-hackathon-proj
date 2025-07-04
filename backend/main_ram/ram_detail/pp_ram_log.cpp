#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // For reading directory contents
#include <ctype.h>  // For isdigit()
#include <unistd.h> // For sleep()
#include <sstream>  // For stringstream
//----------------
// This function sends a JSON string to our central server
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

void send_data_to_server(const std::string& json_output) {
    const char* SOCKET_PATH = "/tmp/ram_orion.sock"; 
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return; 

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        send(fd, json_output.c_str(), json_output.length(), 0);
    }
    
    close(fd);
}
//--------------------
struct ProcessInfo {
    int pid;
    char name[256];
    long ram_kb; 
};

int compareByRam(const void *a, const void *b) {
    ProcessInfo *proc_a = (ProcessInfo *)a; // (ProcessInfo *)a means take a as a pointer to ProcessInfo
    ProcessInfo *proc_b = (ProcessInfo *)b;

    // We want descending order, so subtract b from a
    if (proc_b->ram_kb > proc_a->ram_kb) return 1;
    if (proc_b->ram_kb < proc_a->ram_kb) return -1; // QSORT--> 1 for a > b, -1 for a < b, 0 for a == b
    return 0;
}

int is_numeric(const char *s) {
    if (s == NULL || *s == '\0') {
        return 0;
    }
    while (*s) { // *s is character at that address, and this array will always end at \0
        if (!isdigit(*s)) {
            return 0;
        }
        s++; // since string is stored in array, increasing address index will move to next character
    }
    return 1;
}

// int main() {
//     int capacity = 256;

//     struct ProcessInfo *processes = (ProcessInfo *)malloc(capacity * sizeof(struct ProcessInfo));
//         if (!processes) {
//             perror("Failed to allocate initial memory");
//             return 1;
//         }

//     while (1) {
//         int process_count = 0;
//         DIR *proc_dir; // handle to a directory
//         struct dirent *entry; // (directory entry) predefined struct to hold all info about a subdir or file in a directory
        
//         proc_dir = opendir("/proc");
//         if (!proc_dir) {
//             perror("Failed to open /proc");
//             return 1;
//         }

//         while ((entry = readdir(proc_dir)) != NULL) {
//             if (is_numeric(entry->d_name)) {//d_name is the name of the directory entry(file or subdir)
// //==================================================
//                 if (process_count >= capacity) {
//                     // We are full. Double the capacity.
//                     capacity *= 2;
//                     struct ProcessInfo *new_processes = (ProcessInfo *)realloc(processes, capacity * sizeof(struct ProcessInfo));
//                     if (!new_processes) {
//                         perror("Failed to reallocate memory");
//                         free(processes); // Free the old buffer
//                         closedir(proc_dir);
//                         return 1;
//                     }
//                     processes = new_processes; // Point to the new, larger buffer
//                 }
// //================================================== 
//                 char path[512];
//                 FILE *fp;
                
//                 // 1. Get RAM Usage from /proc/[pid]/status
//                 snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
//                 fp = fopen(path, "r");
//                 if (fp) {
//                     char line[256];
//                     long vmrss = 0;
//                     while (fgets(line, sizeof(line), fp)) {
//                         // VmRSS is the "Resident Set Size", the actual physical RAM used
//                         if (strncmp(line, "VmRSS:", 6) == 0) { // takes the first 6 characters of line and compares with "VmRSS:"
//                             sscanf(line, "VmRSS:\t%ld kB", &vmrss); //parses the line to extract the VmRSS value in vmrss
//                             break;
//                         }
//                     }
//                     fclose(fp);
//                     processes[process_count].ram_kb = vmrss;
//                 } else {
//                     continue; // Skip if we can't read status file
//                 }

//                 // 2. Get Process Name from /proc/[pid]/comm
//                 snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
//                 fp = fopen(path, "r");
//                 if (fp) {
//                     fgets(processes[process_count].name, 
//                           sizeof(processes[process_count].name), fp);
//                     // Remove the trailing newline character from the name
//                     strtok(processes[process_count].name, "\n");
//                     fclose(fp);
//                 } else {
//                     strcpy(processes[process_count].name, "N/A");
//                 }

//                 processes[process_count].pid = atoi(entry->d_name);
//                 process_count++;
//             }
//         }
//         closedir(proc_dir);

//         qsort(processes, process_count, sizeof(struct ProcessInfo), compareByRam);


//         int display_count = (process_count < 20) ? process_count : 20;
//         json j;
//         j["name"] = "top 20 processes";
//         for (int i = 0; i < display_count; ++i) {
//             j[std::to_string(i + 1)] = {
//                 {"pid", processes[i].pid},
//                 {"ram_kb", processes[i].ram_kb},
//                 {"name", processes[i].name}
//             };
//         }
//         std::string json_output = j.dump();
        
//         std::cout << json_output << std::endl;
        
//         send_data_to_server(json_output);
//         fflush(stdout); // Ensure all output is flushed immediately
//         sleep(2);       
        
//     }
//     free(processes); 
//     return 0;
// }
