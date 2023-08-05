#include "inject.h"

#include <chrono>
#include <cinttypes>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include "config.h"
#include "log.h"
#include "xdl.h"

static std::string get_process_name() {
    auto path = "/proc/self/cmdline";

    std::ifstream file(path);
    std::stringstream buffer;

    buffer << file.rdbuf();
    return buffer.str();
}

static void wait_for_init(std::string const &app_name) {
    LOGI("Wait for process to complete init");

    // wait until the process is renamed to the package name
    while (get_process_name().find(app_name) == std::string::npos) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // additional tolerance for the init to complete after process rename
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    LOGI("Process init completed");
}

static void delay_start_up(uint64_t start_up_delay_ms) {
    if (start_up_delay_ms <= 0) {
        return;
    }

    LOGI("Waiting for configured start up delay %" PRIu64"ms", start_up_delay_ms);

    int countdown = 0;
    uint64_t delay = start_up_delay_ms;

    for (int i = 0; i < 10 && delay > 1000; i++) {
        delay -= 1000;
        countdown++;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    for (int i = countdown; i > 0; i--) {
        LOGI("Injecting libs in %d seconds", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void inject_libs(std::unique_ptr<target_config> cfg) {
    // We need to wait for process initialization to complete.
    // Loading the gadget before that will freeze the process
    // before the init has completed. This make the process
    // undiscoverable or otherwise cause issue attaching.
    wait_for_init(cfg->app_name);

    delay_start_up(cfg->start_up_delay_ms);

    for (auto & lib_path : cfg->injected_libraries) {
        LOGI("Injecting %s", lib_path.c_str());
        auto *handle = xdl_open(lib_path.c_str(), XDL_TRY_FORCE_LOAD);
        if (handle) {
            LOGI("Injected %s", lib_path.c_str());
        } else {
            LOGE("Failed to inject %s : %s", lib_path.c_str(), dlerror());
        }
    }
}

bool check_and_inject(std::string const &app_name) {
    std::string module_dir = std::string("/data/local/tmp/re.zyg.fri");

    std::unique_ptr<target_config> cfg = load_config(module_dir, app_name);
    if (cfg == nullptr) {
        return false;
    }

    LOGI("App detected: %s", app_name.c_str());

    if (cfg->injected_libraries.empty()) {
        LOGI("No libraries configured for injection. "
             "Check the content of the `injected_libraries` file or delete it.");
        return false;
    }

    std::thread inject_thread(inject_libs, std::move(cfg));
    inject_thread.detach();

    return true;
}
