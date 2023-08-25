#include "config.h"

#include <string>
#include <fstream>
#include <sstream>
#include <optional>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"
#include "log.h"

// This must work in a non-exception environment as the libcxx we use don't have support for exception.
// Should avoid any libraries aborting on parse error as this will run on every app start and
// might otherwise cause issues for any app starting if misconfigured.

static std::optional<std::vector<std::string>> deserialize_libraries(const rapidjson::Value &doc) {
    if (!doc.IsArray()) {
        LOGE("invalid config: expected injected_libraries to be an array");
        return std::nullopt;
    }

    std::vector<std::string> result;

    for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
        auto &library = doc[i];
        if (!library.IsObject()) {
            LOGE("invalid config: expected injected_libraries members to be objects");
            return std::nullopt;
        }

        auto &path = library["path"];
        if (!path.IsString()) {
            LOGE("invalid config: expected injected_libraries.path to be a string");
            return std::nullopt;
        }

        result.emplace_back(path.GetString());
    }

    return result;
}

static std::optional<child_gating_config> deserialize_child_gating_config(const rapidjson::Value &doc) {
    if (!doc.IsObject()) {
        return std::nullopt;
    }

    child_gating_config result = {};

    auto &enabled = doc["enabled"];
    if (!enabled.IsBool()) {
        LOGE("invalid config: expected child_gating.enabled members to be a bool");
        return std::nullopt;
    }
    result.enabled = enabled.GetBool();

    auto &mode = doc["mode"];
    if (!mode.IsString()) {
        LOGE("invalid config: expected child_gating.mode members to be a string");
        return std::nullopt;
    }
    result.mode = mode.GetString();

    if (doc.HasMember("injected_libraries")) {
        auto injected_libraries = deserialize_libraries(doc["injected_libraries"]);
        if (!injected_libraries.has_value()) {
            return std::nullopt;
        }
        result.injected_libraries = injected_libraries.value();
    }

    return result;
}

static std::optional<target_config> deserialize_target_config(const rapidjson::Value &doc) {
    if (!doc.IsObject()) {
        LOGE("expected config targets array to contain objects");
        return std::nullopt;
    }

    target_config result = {};

    auto &app_name = doc["app_name"];
    if (!app_name.IsString()) {
        LOGE("expected config target to have a valid app_name");
        return std::nullopt;
    }
    result.app_name = app_name.GetString();

    auto &enabled = doc["enabled"];
    if (!enabled.IsBool()) {
        LOGE("invalid config: expected targets.enabled members to be a bool");
        return std::nullopt;
    }
    result.enabled = enabled.GetBool();

    auto &start_up_delay_ms = doc["start_up_delay_ms"];
    if (!start_up_delay_ms.IsUint64()) {
        LOGE("expected config target start_up_delay_ms to be an uint64");
        return std::nullopt;
    }
    result.start_up_delay_ms = start_up_delay_ms.GetUint64();

    auto &injected_libaries = doc["injected_libraries"];
    auto deserialized_libraries = deserialize_libraries(injected_libaries);
    if (!deserialized_libraries.has_value()) {
        return std::nullopt;
    }
    result.injected_libraries = deserialized_libraries.value();

    if (doc.HasMember("child_gating")) {
        auto child_gating = deserialize_child_gating_config(doc["child_gating"]);
        if (!child_gating.has_value()) {
            return std::nullopt;
        }
        result.child_gating = child_gating.value();
    }

    return result;
}

static std::vector<std::string> split(std::string const &str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);

    std::string tmp;
    while (getline(ss, tmp, delimiter)) {
        result.push_back(tmp);
    }

    return result;
}

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

    config_file.close();

    return injected_libraries;
}

static std::optional<target_config> load_simple_config(std::string const &module_dir, std::string const &app_name) {
    std::ifstream config_file(module_dir + "/target_packages");
    if (!config_file.is_open()) {
        return std::nullopt;
    }

    std::string line;
    while (getline(config_file, line)) {
        if (line.empty()) {
            continue;
        }

        auto splitted = split(line, ',');
        if (splitted[0] != app_name) {
            continue;
        }

        target_config cfg = {};
        cfg.app_name = splitted[0];
        cfg.enabled = true;
        if (splitted.size() >= 2) {
            cfg.start_up_delay_ms = std::strtoul(splitted[1].c_str(), nullptr, 10);
        }
        cfg.injected_libraries = parse_injected_libraries(module_dir);

        config_file.close();
        return cfg;
    }

    config_file.close();
    return std::nullopt;
}

static std::optional<target_config> load_advanced_config(std::string const &module_dir, std::string const &app_name) {
    std::ifstream config_file(module_dir + "/config.json");
    if (!config_file.is_open()) {
        return std::nullopt;
    }

    rapidjson::IStreamWrapper config_stream{config_file};

    rapidjson::Document doc;
    doc.ParseStream(config_stream);
    config_file.close();

    if (doc.HasParseError()) {
        LOGE("config is not a valid json file offset %u: %s",
             (unsigned) doc.GetErrorOffset(),
             GetParseError_En(doc.GetParseError()));
        return std::nullopt;
    }

    if (!doc.IsObject()) {
        LOGE("config expected a json root object");
        return std::nullopt;
    }

    auto &targets = doc["targets"];
    if (!targets.IsArray()) {
        LOGE("expected config targets to be an array");
        return std::nullopt;
    }

    for (rapidjson::SizeType i = 0; i < targets.Size(); i++) {
        auto deserialized_target = deserialize_target_config(targets[i]);
        if (!deserialized_target.has_value()) {
            return std::nullopt;
        }

        auto target = deserialized_target.value();
        if (target.app_name == app_name) {
            return target;
        }
    }

    return std::nullopt;
}

std::optional<target_config> load_config(std::string const &module_dir, std::string const &app_name) {
    auto cfg = load_advanced_config(module_dir, app_name);
    if (cfg.has_value()) {
        return cfg;
    }

    return load_simple_config(module_dir, app_name);
}
