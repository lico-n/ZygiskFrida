#include "inject.h"

#include <android/dlext.h>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "log.h"
#include "config.h"

extern "C" [[gnu::weak]] struct android_namespace_t *

__loader_android_create_namespace(
    [[maybe_unused]] const char *name,
    [[maybe_unused]] const char *ld_library_path,
    [[maybe_unused]] const char *default_library_path,
    [[maybe_unused]] uint64_t type,
    [[maybe_unused]] const char *permitted_when_isolated_path,
    [[maybe_unused]] android_namespace_t *parent,
    [[maybe_unused]] const void *caller_addr
);

void *open_gadget(const char *path) {
    auto info = android_dlextinfo{};

    auto *dir = dirname(path);

    if (&__loader_android_create_namespace != nullptr) {
        auto *ns = __loader_android_create_namespace(
            path,
            dir,
            nullptr,
            2/*ANDROID_NAMESPACE_TYPE_SHARED*/,
            nullptr,
            nullptr,
            reinterpret_cast<void *>(&open_gadget));

        info.flags = ANDROID_DLEXT_USE_NAMESPACE;
        info.library_namespace = ns;
    }

    return android_dlopen_ext(path, 0, &info);;
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
    auto *handle = open_gadget(gadget_path.c_str());
    if (handle) {
        LOGI("Gadget connected");
    } else {
        LOGE("Failed to start gadget: %s", dlerror());
    }
}

