#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> // For reading directory contents
#include <ctype.h>  // For isdigit()
#include <unistd.h> // For sleep()

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

int main() {
    int capacity = 256;

    struct ProcessInfo *processes = (ProcessInfo *)malloc(capacity * sizeof(struct ProcessInfo));
        if (!processes) {
            perror("Failed to allocate initial memory");
            return 1;
        }

    while (1) {
        int process_count = 0;
        DIR *proc_dir; // handle to a directory
        struct dirent *entry; // (directory entry) predefined struct to hold all info about a subdir or file in a directory
        
        proc_dir = opendir("/proc");
        if (!proc_dir) {
            perror("Failed to open /proc");
            return 1;
        }

        while ((entry = readdir(proc_dir)) != NULL) {
            if (is_numeric(entry->d_name)) {//d_name is the name of the directory entry(file or subdir)
//==================================================
                if (process_count >= capacity) {
                    // We are full. Double the capacity.
                    capacity *= 2;
                    struct ProcessInfo *new_processes = (ProcessInfo *)realloc(processes, capacity * sizeof(struct ProcessInfo));
                    if (!new_processes) {
                        perror("Failed to reallocate memory");
                        free(processes); // Free the old buffer
                        closedir(proc_dir);
                        return 1;
                    }
                    processes = new_processes; // Point to the new, larger buffer
                }
//================================================== 
                char path[512];
                FILE *fp;
                
                // 1. Get RAM Usage from /proc/[pid]/status
                snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
                fp = fopen(path, "r");
                if (fp) {
                    char line[256];
                    long vmrss = 0;
                    while (fgets(line, sizeof(line), fp)) {
                        // VmRSS is the "Resident Set Size", the actual physical RAM used
                        if (strncmp(line, "VmRSS:", 6) == 0) { // takes the first 6 characters of line and compares with "VmRSS:"
                            sscanf(line, "VmRSS:\t%ld kB", &vmrss); //parses the line to extract the VmRSS value in vmrss
                            break;
                        }
                    }
                    fclose(fp);
                    processes[process_count].ram_kb = vmrss;
                } else {
                    continue; // Skip if we can't read status file
                }

                // 2. Get Process Name from /proc/[pid]/comm
                snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
                fp = fopen(path, "r");
                if (fp) {
                    fgets(processes[process_count].name, 
                          sizeof(processes[process_count].name), fp);
                    // Remove the trailing newline character from the name
                    strtok(processes[process_count].name, "\n");
                    fclose(fp);
                } else {
                    strcpy(processes[process_count].name, "N/A");
                }

                processes[process_count].pid = atoi(entry->d_name);
                process_count++;
            }
        }
        closedir(proc_dir);

        // Sort the list of processes by RAM usage
        qsort(processes, process_count, sizeof(struct ProcessInfo), compareByRam);

        // --- Display the results ---
        
        // ANSI escape code to clear the screen and move cursor to top-left
        printf("\033[H\033[J");
        
        printf("--- Per-Process RAM Usage Monitor (Top 20) ---\n");
        printf("%-10s %-12s %s\n", "PID", "RAM (MB)", "COMMAND");
        printf("----------------------------------------------\n");

        // Print the top 20 processes (or fewer if there aren't that many)
        int display_count = process_count > 20 ? 20 : process_count;
        for (int i = 0; i < display_count; i++) {
            if (processes[i].ram_kb > 0) { // Only show processes actively using RAM
                printf("%-10d %-12.2f %s\n",
                       processes[i].pid,
                       processes[i].ram_kb / 1024.0, // Convert KB to MB for display
                       processes[i].name);
            }
        }
        
        fflush(stdout); // Ensure the output is printed immediately
        sleep(2);       // Wait for 2 seconds before the next update
        
    }
    free(processes); 
    return 0;
}
