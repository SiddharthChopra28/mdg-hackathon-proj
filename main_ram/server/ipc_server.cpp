//g++ -o ipc_server ipc_server.cpp -std=c++17
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>     // For unlink, close, read, write
#include <sys/socket.h> // For socket, bind, listen, accept
#include <sys/un.h>     // For the sockaddr_un struct
#include <csignal>      // For signal handling (Ctrl+C)


const char* SOCKET_PATH = "/tmp/ram_orion.sock";


int server_fd = -1;

static volatile bool running = true;

void signal_handler(int signum) {
    std::cout << "\nCaught signal, shutting down server..." << std::endl;
    running = false;
}

void cleanup() {
    if (server_fd != -1) {
        close(server_fd);
    }
    
    unlink(SOCKET_PATH);
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);


    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  
    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        cleanup(); 
        return 1;
    }


    if (listen(server_fd, 5) < 0) {
        perror("listen error");
        cleanup();
        return 1;
    }

    std::cout << "IPC Server is listening on socket: " << SOCKET_PATH << std::endl;
    std::cout << "Waiting for clients to connect... (Press Ctrl-C to stop)" << std::endl;

    while (running) {
        int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
 
        if (!running) {
            break; 
        }
  
        perror("accept error");
        continue;
    }

        std::cout << "\n[INFO] Client connected." << std::endl;

        char buffer[4096] = {0};

        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {

            std::cout << "[DATA] Received: " << buffer << std::endl;
        } else if (bytes_read == 0) {
            std::cout << "[INFO] Client disconnected." << std::endl;
        } else {
            perror("read error");
        }


        close(client_fd);
    }

    cleanup();
    return 0;
}