#ifndef ZYGISKFRIDA_INJECT_H
#define ZYGISKFRIDA_INJECT_H

#include <string>

bool should_inject(std::string const& module_dir, std::string const& app_name);

void inject_gadget(std::string const& gadget_path, std::string const& app_name);

#endif  // ZYGISKFRIDA_INJECT_H
