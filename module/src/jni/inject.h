#ifndef ZYGISKFRIDA_INJECT_H
#define ZYGISKFRIDA_INJECT_H

#include <string>
#include "config.h"

void inject_gadget(std::string const& gadget_path, std::unique_ptr<target_config> cfg);

#endif  // ZYGISKFRIDA_INJECT_H
