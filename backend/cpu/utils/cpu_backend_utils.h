// cpu_backend_utils.h

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <sys/types.h>
#include <bits/stdc++.h>

using namespace std;

struct ProcessGroup
{
    string name;
    vector<pid_t> pids;
    uint64_t total_cpu_time;
};

bool is_kernel_thread(pid_t pid);
bool is_root_process(pid_t pid);
map<string, ProcessGroup> read_process_groups_from_map();
set<string> load_whitelist();
bool add_to_whitelist(const string& name);
bool remove_from_whitelist(const string& name);

