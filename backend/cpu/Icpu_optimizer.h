#pragma once

#include <string>
#include <set>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

// These are the exposed API functions to Electron/CLI

bool optimize_process(const std::string& name, const std::string& level);
bool restore_process(const std::string& name);
void restore_all();
void optimize_all_heavy_processes();

// Whitelist manipulation
void add_to_whitelist_interface(const string& name);
void remove_from_whitelist_interface(const string& name);
set<string> show_all_whitelist();
json get_process_data();
