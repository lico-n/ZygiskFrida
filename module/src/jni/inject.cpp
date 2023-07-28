#include "inject.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "config.h"
#include "inttypes.h"
#include "log.h"
#include "xdl.h"


std::string get_process_name() {
    auto path = "/proc/self/cmdline";

    std::ifstream file(path);
    std::stringstream buffer;

    buffer << file.rdbuf();
    return buffer.str();
}

void wait_for_init(std::string const& app_name) {
    LOGI("Wait for process to complete init");

    // wait until the process is renamed to the package name
    while (get_process_name().find(app_name) == std::string::npos) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // additional tolerance for the init to complete after process rename
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    LOGI("Process init completed");
}

void delay_start_up(target_config *cfg) {
    if (cfg->start_up_delay <= 0) {
        return;
    }

    LOGI("Waiting for configured start up delay %" PRIu64"ms", cfg->start_up_delay);

    int countdown = 0;
    uint64_t delay = cfg->start_up_delay;

    for (int i = 0; i < 10 && delay > 1000; i++) {
        delay -= 1000;
        countdown++;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    for (int i = countdown; i > 0; i--) {
        LOGI("Starting gadget in %d seconds", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void inject_gadget(std::string const& gadget_path, target_config* cfg) {
    // We need to wait for process initialization to complete.
    // Loading the gadget before that will freeze the process
    // before the init has completed. This make the process
    // undiscoverable or otherwise cause issue attaching.
    wait_for_init(cfg->app_name);

    delay_start_up(cfg);

    LOGI("Starting gadget %s", gadget_path.c_str());
    auto *handle = xdl_open(gadget_path.c_str(), XDL_ALWAYS_FORCE_LOAD);
    if (handle) {
        LOGI("Gadget connected");
    } else {
        LOGE("Failed to start gadget: %s", dlerror());
    }

    delete cfg;
}

