#include "inject.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "log.h"
#include "xdl.h"

bool should_inject(std::string const& module_dir, std::string const& app_name) {
    auto targetFilePath = module_dir + "/target_packages";

    std::ifstream target_file(targetFilePath);
    if (!target_file.is_open()) {
        return false;
    }

    std::string line;
    while (getline(target_file, line)) {
        if (line == app_name) {
            return true;
        }
    }

    return false;
}

std::string get_process_name() {
    auto path = "/proc/self/cmdline";

    std::ifstream file(path);
    std::stringstream buffer;

    buffer << file.rdbuf();
    return buffer.str();
}

void wait_for_init(std::string const& app_name) {
    // wait until the process is renamed to the package name
    while (get_process_name().find(app_name) == std::string::npos) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // additional tolerance for the init to complete after process rename
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void inject_gadget(std::string const& gadget_path, std::string const& app_name) {
    LOGI("Wait for process to complete init");

    // We need to wait for process initialization to complete.
    // Loading the gadget before that will freeze the process
    // before the init has completed. This make the process
    // undiscoverable or otherwise cause issue attaching.
    wait_for_init(app_name);

    LOGI("Starting gadget %s", gadget_path.c_str());
    auto *handle = xdl_open(gadget_path.c_str(), XDL_ALWAYS_FORCE_LOAD);
    if (handle) {
        LOGI("Gadget connected");
    } else {
        LOGE("Failed to start gadget: %s", dlerror());
    }
}

