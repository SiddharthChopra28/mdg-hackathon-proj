#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "nlohmann_json.hpp"

using json = nlohmann::json;

void send_request(const json& j_request) {
    const char* SOCKET_PATH = "/tmp/network_optimizer.sock";

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return;
    }

    std::string request_str = j_request.dump();
    if (write(sock_fd, request_str.c_str(), request_str.size()) < 0) {
        perror("write");
        close(sock_fd);
        return;
    }

    char buffer[8192] = {0};
    ssize_t len = read(sock_fd, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        std::cout << "Response: " << std::string(buffer, len) << std::endl;
    } else {
        perror("read");
    }

    close(sock_fd);
}

int main() {
    std::cout << "Sending network_get_usage..." << std::endl;
    send_request({{"action", "network_get_usage"}});

    std::cout << "\nSending network_set_speed_cap..." << std::endl;
    send_request({{"action", "network_set_speed_cap"}, {"app_name", "firefox"}, {"speed_mbps", 10}});

    std::cout << "\nSending network_reset_cap..." << std::endl;
    send_request({{"action", "network_reset_cap"}, {"app_name", "firefox"}});

    return 0;   
}
