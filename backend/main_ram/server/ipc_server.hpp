#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <csignal>
#include <dirent.h>
#include <cstring>
#include <cctype>
#include <cmath>
#include <sys/stat.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

json handle_ram_usage();
json handle_top_processes();