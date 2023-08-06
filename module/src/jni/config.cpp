#include "config.h"

#include <string>
#include <fstream>
#include <sstream>

static std::vector<std::string> split(std::string const &str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);

    std::string tmp;
    while (getline(ss, tmp, delimiter)) {
        result.push_back(tmp);
    }

    return result;
}

// parse_injected_libraries parses a config line from the `target_packages` file.
// Each line consist of the app package name for the app that will be targeted by this module.
// Optionally there might be a start up delay in milliseconds specified separated by a comma
// from the package name.
static std::unique_ptr<target_config> parse_target_config(std::string const &line) {
    std::unique_ptr<target_config> tcfg(new target_config);

    auto parts = split(line, ',');

    tcfg->app_name = parts[0];

    if (parts.size() >= 2) {
        tcfg->start_up_delay_ms = std::stoul(parts[1]);
    } else {
        tcfg->start_up_delay_ms = 0;
    }

    return tcfg;
}

// parse_injected_libraries returns a vector with all libraries that will be injected
// into the process. By default this is the bundled frida-gadget.
// If the file `injected_libraries` exists in the re.zyg.fri directory, then
// it will load the libraries specified in that file. One lib file path per line.
static std::vector<std::string> parse_injected_libraries(std::string const &module_dir) {
    auto config_file_path = module_dir + "/injected_libraries";

    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        return {module_dir + "/libgadget.so"};
    }

    std::vector<std::string> injected_libraries;

    std::string libpath;
    while (getline(config_file, libpath)) {
        if (!libpath.empty()) {
            injected_libraries.push_back(libpath);
        }
    }

    return injected_libraries;
}

std::unique_ptr<target_config> load_config(std::string const &module_dir, std::string const &app_name) {
    auto config_file_path = module_dir + "/target_packages";

    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        return nullptr;
    }

    std::string line;
    while (getline(config_file, line)) {
        auto cfg = parse_target_config(line);
        if (cfg->app_name == app_name) {
            cfg->injected_libraries = parse_injected_libraries(module_dir);
            return cfg;
        }
    }

    return nullptr;
}
