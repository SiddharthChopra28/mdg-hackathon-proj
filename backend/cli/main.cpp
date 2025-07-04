#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include "nlohmann_json.hpp"

#include "../cpu/Icpu_optimizer.h"
#include "../main_ram/server/ipc_server.hpp"
#include "../network/api.hpp"

using namespace std;
using json = nlohmann::json;

// build command
// g++ ./cli/main.cpp ./cpu/cpu_optimizer.cpp ./cpu/utils/cpu_backend_utils.cpp ./cpu/cpu_reader.cpp  ./main_ram/server/api.cpp ./network/api.cpp -o cliprogran -lbpf

void print_json(const json &j) {
    cout << setw(4) << j << endl;
}

void handle_ram(const vector<string>& args) {
    if (args.empty()) {
        cout << "Commands for domain 'ram':\n  usage\n  top\n";
        return;
    }
    

    if (args[0] == "usage") {
        print_json(handle_ram_usage());
    } else if (args[0] == "top") {
        print_json(handle_top_processes());
    } else {
        cout << "Unknown ram command: " << args[0] << endl;
    }
}

void handle_cpu(const vector<string>& args) {
    if (args.empty()) {
        cout << "Commands for domain 'cpu':\n  get\n  optimize <name> <level>\n  restore <name>\n  restore-all\n  optimize-heavy\n  whitelist add/remove/show <name>\n";
        return;
    }

    string cmd = args[0];

    if (cmd == "get") {
        print_json(get_processor_data());
    } else if (cmd == "optimize" && args.size() >= 3) {
        optimize_process(args[1], args[2]);
        cout << "Optimized process " << args[1] << " with level " << args[2] << endl;
    } else if (cmd == "restore" && args.size() >= 2) {
        restore_process(args[1]);
        cout << "Restored process " << args[1] << endl;
    } else if (cmd == "restore-all") {
        restore_all();
        cout << "All processes restored.\n";
    } else if (cmd == "optimize-heavy") {
        optimize_all_heavy_processes();
        cout << "Optimized all heavy processes.\n";
    } else if (cmd == "whitelist" && args.size() >= 2) {
        string action = args[1];
        if (action == "add" && args.size() >= 3) {
            add_to_whitelist_interface(args[2]);
            cout << "Added " << args[2] << " to whitelist.\n";
        } else if (action == "remove" && args.size() >= 3) {
            remove_from_whitelist_interface(args[2]);
            cout << "Removed " << args[2] << " from whitelist.\n";
        } else if (action == "show") {
            auto wl = show_all_whitelist();
            cout << "Whitelist:\n";
            for (const auto &name : wl)
                cout << "  " << name << endl;
        } else {
            cout << "Invalid whitelist command.\n";
        }
    } else {
        cout << "Unknown or malformed CPU command.\n";
    }
}

void handle_network(const vector<string>& args) {


    // setup_network_prog();

    if (args.empty()) {
        cout << "Commands for domain 'network':\n  stats-overall\n  stats-pid\n  set-limit <appname> <rate-mbps>\n  reset-limit <appname>\n";
        return;
    }

    string cmd = args[0];

    if (cmd == "stats-overall") {
        auto [tx, rx] = sendNetworkStatsOverall();
        cout << "Total TX: " << tx << " bytes, RX: " << rx << " bytes\n";
    } else if (cmd == "stats-pid") {
        auto stats = sendNetworkStatsPerPID();
        for (const auto& [name, pair] : stats) {
            cout << left << setw(20) << name
                 << " TX: " << pair.first << " B"
                 << ", RX: " << pair.second << " B\n";
        }
    } else if (cmd == "set-limit" && args.size() >= 3) {
        string app = args[1];
        int rate = stoi(args[2]);
        setAppRateLimit(app, rate);
        cout << "Rate limit set for " << app << " at " << rate << " Mbps.\n";
    } else if (cmd == "reset-limit" && args.size() >= 2) {
        resetAppRateLimit(args[1]);
        cout << "Rate limit reset for " << args[1] << endl;
    } else {
        cout << "Unknown or malformed network command.\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: sysopt [ram|cpu|network] [command] [options...]\n";
        return 1;
    }

    string domain = argv[1];
    vector<string> args(argv + 2, argv + argc);

    if (domain == "ram") {
        handle_ram(args);
    } else if (domain == "cpu") {
        handle_cpu(args);
    } else if (domain == "network") {
        handle_network(args);
    } else {
        cout << "Unknown domain: " << domain << endl;
    }

    return 0;
}
