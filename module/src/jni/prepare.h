#ifndef ZYGISKFRIDA_PREPARE_H
#define ZYGISKFRIDA_PREPARE_H

#include <string>

bool should_inject(std::string const& module_dir, std::string const& app_name);

std::string prepare_gadget(std::string const& module_dir);

#endif  // ZYGISKFRIDA_PREPARE_H
