#include <string>
#include <thread>

#include "inject.h"
#include "log.h"
#include "prepare.h"
#include "zygisk.h"

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
        auto app_name = env->GetStringUTFChars(args->nice_name, nullptr);

        this->inject = should_inject(app_name);
        if (!this->inject) {
            this->api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            this->env->ReleaseStringUTFChars(args->nice_name, app_name);
            return;
        }

        LOGI("App detected: %s", app_name);
        LOGI("Preparing for gadget injection");

        this->gadget_path = prepare_gadget();
        if (this->gadget_path.empty()) {
            LOGE("unexpected error preparing gadget");
            this->inject = false;
            this->api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            this->env->ReleaseStringUTFChars(args->nice_name, app_name);
            return;
        }

        LOGI("Preparation completed");
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (this->inject) {
            std::thread inject_thread(inject_gadget, this->gadget_path);
            inject_thread.detach();
        }
    }

 private:
    Api *api;
    JNIEnv *env;
    bool inject;
    std::string gadget_path;
};

REGISTER_ZYGISK_MODULE(MyModule)
