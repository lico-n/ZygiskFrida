#ifndef ZYGISKFRIDA_INJECT_H
#define ZYGISKFRIDA_INJECT_H

#include <string>

void inject_lib(std::string const& lib_path, std::string const& logContext);
bool check_and_inject(std::string const& app_name, int pid);

#endif  // ZYGISKFRIDA_INJECT_H
