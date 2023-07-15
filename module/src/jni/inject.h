#ifndef ZYGISKFRIDA_INJECT_H
#define ZYGISKFRIDA_INJECT_H

#include <string>

bool should_inject(std::string app_name);
void inject_gadget(std::string gadget_path);

#endif  // ZYGISKFRIDA_INJECT_H
