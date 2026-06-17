// Host unit tests for ZygiskFrida config parsing (no device / NDK required).
//
// Focus: the inject_on_specialize target option — default, true, false, and
// invalid-type handling — plus a sanity check that the surrounding target
// fields still parse.
//
// Build & run:  module/src/jni/tests/run.sh

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "config.h"

namespace fs = std::filesystem;

static int g_failures = 0;

#define CHECK(cond, msg)                                                     \
  do {                                                                       \
    if (cond) {                                                             \
      std::printf("  ok   - %s\n", msg);                                    \
    } else {                                                                \
      std::printf("  FAIL - %s (%s:%d)\n", msg, __FILE__, __LINE__);        \
      ++g_failures;                                                         \
    }                                                                       \
  } while (0)

// Minimal valid target body (everything except inject_on_specialize).
static const char *BASE_TARGET =
    "\"app_name\": \"com.test.app\","
    "\"enabled\": true,"
    "\"start_up_delay_ms\": 0,"
    "\"injected_libraries\": [ { \"path\": \"/data/local/tmp/libknox.so\" } ]";

static std::string write_config(const fs::path &dir, const std::string &body) {
  fs::create_directories(dir);
  std::ofstream(dir / "config.json")
      << "{ \"targets\": [ {" << body << "} ] }";
  return dir.string();
}

int main() {
  const fs::path tmp = fs::temp_directory_path() / "zygfri_cfg_test";
  fs::remove_all(tmp);

  std::printf("inject_on_specialize parsing:\n");

  // 1. Absent -> defaults to false, base fields still parse.
  {
    auto dir = write_config(tmp / "absent", BASE_TARGET);
    auto cfg = load_config(dir, "com.test.app");
    CHECK(cfg.has_value(), "absent: config loads");
    CHECK(cfg && cfg->inject_on_specialize == false,
          "absent: inject_on_specialize defaults to false");
    CHECK(cfg && cfg->enabled && cfg->app_name == "com.test.app" &&
              cfg->start_up_delay_ms == 0 && cfg->injected_libraries.size() == 1,
          "absent: base target fields parsed");
  }

  // 2. Explicit true.
  {
    auto dir = write_config(
        tmp / "true", std::string(BASE_TARGET) + ",\"inject_on_specialize\": true");
    auto cfg = load_config(dir, "com.test.app");
    CHECK(cfg.has_value(), "true: config loads");
    CHECK(cfg && cfg->inject_on_specialize == true,
          "true: inject_on_specialize parsed as true");
  }

  // 3. Explicit false.
  {
    auto dir = write_config(
        tmp / "false",
        std::string(BASE_TARGET) + ",\"inject_on_specialize\": false");
    auto cfg = load_config(dir, "com.test.app");
    CHECK(cfg.has_value(), "false: config loads");
    CHECK(cfg && cfg->inject_on_specialize == false,
          "false: inject_on_specialize parsed as false");
  }

  // 4. Wrong type -> whole config rejected (no silent default).
  {
    auto dir = write_config(
        tmp / "wrongtype",
        std::string(BASE_TARGET) + ",\"inject_on_specialize\": \"yes\"");
    auto cfg = load_config(dir, "com.test.app");
    CHECK(!cfg.has_value(), "wrong-type: config rejected");
  }

  fs::remove_all(tmp);

  if (g_failures == 0) {
    std::printf("\nALL PASSED\n");
    return 0;
  }
  std::printf("\n%d FAILURE(S)\n", g_failures);
  return 1;
}
