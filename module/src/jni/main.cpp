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

    void preAppSpecialize(AppSpecializeArgs *args) override {
        auto raw_app_name = env->GetStringUTFChars(args->nice_name, nullptr);
        this->app_name = std::string(raw_app_name);
        this->env->ReleaseStringUTFChars(args->nice_name, raw_app_name);

        std::string module_dir = std::string("/data/local/tmp/") + ModulePackageName;
        this->gadget_path = module_dir + "/" + GadgetLibraryName;

        this->inject = should_inject(module_dir, this->app_name);
        if (!this->inject) {
            this->api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        LOGI("App detected: %s", this->app_name.c_str());
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (this->inject) {
            std::thread inject_thread(inject_gadget, this->gadget_path, this->app_name);
            inject_thread.detach();
        }
    }

 private:
    Api *api;
    JNIEnv *env;
    bool inject;
    std::string gadget_path;
    std::string app_name;
};

REGISTER_ZYGISK_MODULE(MyModule)
