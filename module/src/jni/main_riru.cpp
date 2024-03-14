#define RIRU_MODULE

#include <riru.h>
#include <string>

#include "riru_config.h"
#include "inject.h"

static std::string app_name = ""; // NOLINT

static void forkAndSpecializePre(
    JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids,
    jint *runtimeFlags, jobjectArray *rlimits, jint *mountExternal, jstring *seInfo,
    jstring *niceName, jintArray *fdsToClose, jintArray *fdsToIgnore,
    jboolean *is_child_zygote, jstring *instructionSet, jstring *appDataDir,
    jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
    jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {

    const char* raw_app_name = env->GetStringUTFChars(*niceName, nullptr);
    app_name = std::string(raw_app_name);
    env->ReleaseStringUTFChars(*niceName, raw_app_name);
}

static void forkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    if (res != 0) {
        return;
    }

    if (check_and_inject(app_name)) {
        riru_set_unload_allowed(false);
    }
}

static void specializeAppProcessPre(
    JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
    jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
    jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
    jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
    jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {}

static void specializeAppProcessPost(JNIEnv *env, jclass clazz) {
    riru_set_unload_allowed(true);
}

static void forkSystemServerPre(
    JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *runtimeFlags,
    jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {}

static void forkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {}
static void onModuleLoaded() {}

extern "C" {

int riru_api_version;
const char *riru_magisk_module_path = nullptr;
int *riru_allow_unload = nullptr;

static auto module = RiruVersionedModuleInfo{
    .moduleApiVersion = riru_config::api_version,
    .moduleInfo = RiruModuleInfo{
        .supportHide = true,
        .version = riru_config::version_code,
        .versionName = riru_config::version_name,
        .onModuleLoaded = onModuleLoaded,
        .forkAndSpecializePre = forkAndSpecializePre,
        .forkAndSpecializePost = forkAndSpecializePost,
        .forkSystemServerPre = forkSystemServerPre,
        .forkSystemServerPost = forkSystemServerPost,
        .specializeAppProcessPre = specializeAppProcessPre,
        .specializeAppProcessPost = specializeAppProcessPost
    }
};

RiruVersionedModuleInfo *init(Riru *riru) {
    auto core_max_api_version = riru->riruApiVersion;
    riru_api_version = core_max_api_version <= riru_config::api_version ?
        core_max_api_version : riru_config::api_version;
    module.moduleApiVersion = riru_api_version;

    riru_magisk_module_path = strdup(riru->magiskModulePath);
    if (riru_api_version >= 25) {
        riru_allow_unload = riru->allowUnload;
    }
    return &module;
}
}
