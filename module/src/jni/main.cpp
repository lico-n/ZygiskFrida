#include <string>
#include <thread>

#include "inject.h"
#include "log.h"
#include "zygisk.h"
#include "config.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
 public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        const char* raw_app_name = env->GetStringUTFChars(args->nice_name, nullptr);
        std::string app_name = std::string(raw_app_name);
        this->env->ReleaseStringUTFChars(args->nice_name, raw_app_name);

        std::string module_dir = std::string("/data/local/tmp/") + ModulePackageName;
        std::string gadget_path = module_dir + "/" + GadgetLibraryName;

        if (!should_inject(module_dir, app_name)) {
            this->api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("App detected: %s", app_name.c_str());

        std::thread inject_thread(inject_gadget, gadget_path, app_name);
        inject_thread.detach();
    }

 private:
    Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(MyModule)
