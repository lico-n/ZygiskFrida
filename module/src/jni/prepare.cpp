#include "prepare.h"

#include <sys/stat.h>

#include <filesystem>
#include <fstream>
#include <map>

#include "log.h"
#include "config.h"

namespace fs = std::filesystem;

bool should_inject(std::string const& module_dir, std::string const& app_name) {
    std::ifstream target_file(module_dir + "/target_packages");
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

void sync_directories(std::string destination, std::string source) {
    fs::copy(
        source,
        destination,
        fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    // delete old files
    std::map<std::string, bool> new_file_map;
    for (const auto &entry : fs::recursive_directory_iterator(source)) {
        auto path = fs::relative(entry, source);
        new_file_map[path]= true;
    }

    for (const auto &entry : fs::recursive_directory_iterator(destination)) {
        auto path = fs::relative(entry, destination);
        if (!new_file_map[path]) {
            fs::remove_all(entry.path());
        }
    }
}

std::string prepare_gadget(std::string const& module_dir) {
    std::string tmp_dir = "/data/local/tmp/";
    std::string destination = tmp_dir + ModulePackageName;
    std::string source = module_dir + "/gadget";
    std::string gadget_path = destination + "/" + GadgetPath;

    mkdir(destination.c_str(), 0755);
    fs::permissions(
        destination,
        fs::perms::owner_all | fs::perms::group_read |
        fs::perms::group_exec | fs::perms::others_read |
        fs::perms::others_exec,
        fs::perm_options::replace);


    sync_directories(destination, source);

    LOGI("Copied gadget files to %s", destination.c_str());

    return gadget_path;
}
