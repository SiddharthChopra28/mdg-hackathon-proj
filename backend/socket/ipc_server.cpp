#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
#include <sys/stat.h> 
#include "../cpu/Icpu_optimizer.h"
#include "../../ebpf/cpu/cputracker.skel.h"
#include "../network/api.hpp"

using namespace std;
using json = nlohmann::json;

#define SOCKET_PATH "/tmp/cpu_optimizer.sock"

void handle_command(const json& j, int client_fd) 
{
    string action = j["action"];
    string response;

    if (action == "cpu_optimize") 
    {
        string name = j["name"];    
        string level = j["level"];
        bool success = optimize_process(name, level);
        response = success ? "Optimized\n" : "Failed\n";
    }   
    else if (action == "cpu_restore") 
    {
        string name = j["name"];
        bool success = restore_process(name);
        response = success ? "Restored\n" : "Failed\n";
    }
    else if (action == "cpu_restore_all") 
    {
        restore_all();
        response = "All Restored\n";
    }
    else if (action == "cpu_add_whitelist") 
    {
        add_to_whitelist_interface(j["name"]);
        response = "Added to whitelist\n";
    }
    else if (action == "cpu_remove_whitelist") 
    {
        remove_from_whitelist_interface(j["name"]);
        response = "Removed from whitelist\n";
    }
    else if (action == "cpu_list_whitelist") 
    {
        auto w = show_all_whitelist();
        json out = json(w);
        response = out.dump() + "\n";
    }else if(action == "cpu_print_processes")
    {
        json out = get_process_data();
        response = out.dump(2) + "\n";
    }else if (action == "cpu_processor_info")
    {
        json out = get_processor_data();
        response = out.dump(2) + '\n';
    }   
    else 
    {

        response = "Unknown command\n";
    }

    write(client_fd, response.c_str(), response.size());
}



int main() {

    std::thread t (setup_network_prog);
    t.join();

    struct cputracker_bpf *skel = cputracker_bpf__open_and_load();
    if (!skel) {
        std::cerr << "Error loading BPF object" << std::endl;
        return 1;
    }

    int err = cputracker_bpf__attach(skel);
    if (err) {
        std::cerr << "Failed to attach BPF program" << std::endl;
        cputracker_bpf__destroy(skel);
        return 1;
    }

    std::cout << "BPF program loaded and attached" << std::endl;

    std::cout<<"Program loaded and attached"<<std::endl;
    unlink(SOCKET_PATH);  // Remove existing

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    chmod(SOCKET_PATH, 0666);
    listen(server_fd, 5);
    cout << "IPC Server listening on " << SOCKET_PATH << "\n";

    while (true) {      
        int client_fd = accept(server_fd, nullptr, nullptr);
        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer));

        try {
            json j = json::parse(buffer);
            handle_command(j, client_fd);
        } catch (...) {
            string err = "Invalid JSON\n";
            write(client_fd, err.c_str(), err.size());
        }

        close(client_fd);
        // cputracker_bpf__destroy(skel);  
    }
}
