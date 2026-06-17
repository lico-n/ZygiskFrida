#ifndef ZYGISKFRIDA_CONFIG_H
#define ZYGISKFRIDA_CONFIG_H

#include <memory>
#include <string>
#include <vector>
#include <optional>

struct child_gating_config {
    bool enabled;
    std::string mode;
    std::vector<std::string> injected_libraries;
};

struct target_config{
    bool enabled;
    std::string app_name;
    uint64_t start_up_delay_ms;
    std::vector<std::string> injected_libraries;
    child_gating_config child_gating;
    // When true, inject synchronously inside postAppSpecialize (before the app
    // runs Application.onCreate) instead of waiting for process init on a
    // detached thread. Lets the gadget's top-level hooks land before the app's
    // first anti-tamper check. Optional; defaults to false. Safe only for
    // script-interaction gadgets (they load their script and return).
    bool inject_on_specialize;
};

std::optional<target_config> load_config(std::string const& module_dir, std::string const& app_name);
std::optional<target_config> parse_advanced_config(std::string const& config, std::string const& app_name);

#endif  // ZYGISKFRIDA_CONFIG_H
