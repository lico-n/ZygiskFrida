#include "child_gating.h"

#include <dobby.h>
#include <log.h>
#include <unistd.h>
#include <xdl.h>

#include <cstdlib>
#include <future>
#include <string>
#include <vector>

#include "config.h"

static std::string child_gating_mode;  // NOLINT
static std::vector<std::string> injected_libraries;

pid_t (*orig_fork)();

pid_t (*orig_vfork)();

pid_t fork_replacement() {
    pid_t parent_pid = getpid();
    LOGI("[child_gating][pid %d] detected fork/vfork", parent_pid);

    pid_t child_pid = orig_fork();
    if (child_pid != 0) {
        LOGI("[child_gating][pid %d] returning from forking %d", parent_pid, child_pid);
        return child_pid;
    }

    child_pid = getpid();

    if (child_gating_mode == "kill") {
        LOGI("[child_gating][pid %d] killing child process", child_pid);
        exit(0);
    }

    if (child_gating_mode == "freeze") {
        LOGI("[child_gating][pid %d] freezing child process", child_pid);
        std::promise<void>().get_future().wait();
        return 0;
    }

    if (child_gating_mode != "inject") {
        LOGI("[child_gating][pid %d] unknown child_gating_mode %s", child_pid, child_gating_mode.c_str());
        return 0;
    }

    for (auto &lib_path : injected_libraries) {
        LOGI("[child_gating][pid %d] Injecting %s", child_pid, lib_path.c_str());
        auto *handle = xdl_open(lib_path.c_str(), XDL_TRY_FORCE_LOAD);
        if (handle) {
            LOGI("[child_gating][pid %d] Injected %s with handle %p", child_pid, lib_path.c_str(), handle);
        } else {
            LOGE("[child_gating][pid %d]Failed to inject %s : %s", child_pid, lib_path.c_str(), dlerror());
        }
    }

    return 0;
}


void enable_child_gating(child_gating_config const &cfg) {
    child_gating_mode = cfg.mode;
    injected_libraries = cfg.injected_libraries;

    LOGI("[child_gating] enabling child gating");

    void *forkAddr = dlsym(RTLD_DEFAULT, "fork");
    LOGI("[child_gating] fork address %p", forkAddr);
    void *vforkAddr = dlsym(RTLD_DEFAULT, "vfork");
    LOGI("[child_gating] vfork address %p", vforkAddr);

    DobbyHook(
        forkAddr,
        reinterpret_cast<void *>(fork_replacement),
        reinterpret_cast<void **>(&orig_fork));
    LOGI("[child_gating] fork hook installed");
    DobbyHook(
        vforkAddr,
        reinterpret_cast<void *>(fork_replacement),
        reinterpret_cast<void **>(&orig_vfork));
    LOGI("[child_gating] vfork hook installed");

    LOGI("[child_gating] child gating enabled");
}

