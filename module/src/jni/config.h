#ifndef ZYGISKFRIDA_CONFIG_H
#define ZYGISKFRIDA_CONFIG_H

#include <memory>
#include <string>
#include <vector>

struct target_config{
    std::string app_name;
    uint64_t start_up_delay_ms;
    std::vector<std::string> injected_libraries;
};

std::unique_ptr<target_config> load_config(std::string const& module_dir, std::string const& app_name);

#endif  // ZYGISKFRIDA_CONFIG_H
