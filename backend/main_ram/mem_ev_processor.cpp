//g++ mem_ev_processor.cpp -o mem_ev_processor -lbpf -lelf
#include <iostream>
#include <csignal>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h> // For sleep
#include "nlohmann_json.hpp"
using json = nlohmann::json;
//=========sockets========
// This function sends a JSON string to our central server
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

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
//=========================


struct ram_event_t {
    int pid;
    char comm[16];
    int event_type;
};

static volatile bool running = true;

void signal_handler(int signum) {
    running = false;
}

// This is our callback function. It gets called by libbpf
// every time a new event arrives from the kernel.
std::string handle_event(void *ctx, int cpu, void *data, __u32 size) {
    struct ram_event_t *event = (ram_event_t *)data;
    json j;
    std::string json_output;

    if (event->event_type == 1) { // 1 = Page Fault

        j["name"] = "page_fault";
        j["pid"] = event->pid;
        j["comm"] = event->comm;
        json_output = j.dump();

    } else if (event->event_type == 2) { // 2 = OOM Kill
        j["name"] = "oom_kill";
        j["pid"] = event->pid;
        j["comm"] = event->comm;
        json_output = j.dump();
    } else {
        j["name"] = "unknown_event";
        j["pid"] = event->pid;
        j["comm"] = event->comm;
        json_output = j.dump();
    }
    std::cout << "Event Data: " << json_output << std::endl;
    // send_data_to_server(json_output);
    return json_output;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const char* obj_file = "./mem_events.bpf.o";
    struct bpf_object* obj = nullptr;
    struct bpf_map* map = nullptr;
    struct perf_buffer* pb = nullptr;

    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

    obj = bpf_object__open_file(obj_file, nullptr);
    if (!obj) {
        std::cerr << "ERROR: Failed to open BPF object file: " << obj_file << std::endl;
        return 1;
    }

    if (bpf_object__load(obj)) {
        std::cerr << "ERROR: Failed to load BPF object" << std::endl;
        bpf_object__close(obj);
        return 1;
    }

    struct bpf_program *prog;
    bpf_object__for_each_program(prog, obj) {
        struct bpf_link *link = bpf_program__attach(prog);
        if (!link) {
            std::cerr << "ERROR: Failed to attach BPF program" << std::endl;
            bpf_object__close(obj);
            return 1;
        }
    }

    map = bpf_object__find_map_by_name(obj, "ram_events");
    if (!map) {
        std::cerr << "ERROR: Failed to find 'ram_events' map" << std::endl;
        bpf_object__close(obj);
        return 1;
    }


    pb = perf_buffer__new(bpf_map__fd(map), 8, handle_event, nullptr, nullptr, nullptr);
    if (!pb) {
        std::cerr << "ERROR: Failed to create perf buffer" << std::endl;
        bpf_object__close(obj);
        return 1;
    }



    while (running) {
        // Poll for events with a 100ms timeout
        int err = perf_buffer__poll(pb, 100);
        if (err < 0) {
            // error or Ctrl-C was pressed
            break;
        }
    }


    perf_buffer__free(pb);
    bpf_object__close(obj); // automatically detaches links

    return 0;
}