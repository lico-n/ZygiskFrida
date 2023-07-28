#include "config.h"

#include <string>
#include <fstream>
#include <sstream>

std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);

    std::string tmp;
    while (getline(ss, tmp, delimiter)) {
        result.push_back(tmp);
    }

    return result;
}

target_config* parse_target_config(std::string line) {
    auto tcfg = new target_config;

    auto parts = split(line, ',');

    tcfg->app_name = parts[0];

    if (parts.size() >= 2) {
        tcfg->start_up_delay = std::stoul(parts[1]);
    } else {
        tcfg->start_up_delay = 0;
    }

    return tcfg;
}

target_config* load_config(std::string const& module_dir, std::string const& app_name) {
    auto config_file_path = module_dir + "/target_packages";

    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        return nullptr;
    }

    std::string line;
    while (getline(config_file, line)) {
        target_config* cfg = parse_target_config(line);
        if (cfg->app_name == app_name) {
            return cfg;
        }
    }

    return nullptr;
}
