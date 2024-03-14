LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

XDL_FILES := $(wildcard $(LOCAL_PATH)/xdl/*.c)

APP_STL=none
LOCAL_MODULE := zygiskfrida
LOCAL_C_INCLUDES := $(LOCAL_PATH)/xdl/include $(LOCAL_PATH)/include
LOCAL_SRC_FILES := inject.cpp config.cpp riru_config.cpp child_gating.cpp remapper.cpp $(XDL_FILES:$(LOCAL_PATH)/%=%)
LOCAL_STATIC_LIBRARIES := cxx dobby
LOCAL_LDLIBS := -llog

ifeq ($(API), riru)
LOCAL_SRC_FILES += main_riru.cpp
LOCAL_STATIC_LIBRARIES += riru
else
LOCAL_SRC_FILES  += main_zygisk.cpp
endif

include $(BUILD_SHARED_LIBRARY)

$(LOCAL_PATH)/riru_config.cpp : FORCE
	$(file > $@,namespace riru_config {)
	$(file >> $@,extern const int version_code = ${MODULE_VERSION_CODE};)
	$(file >> $@,extern const char* const version_name = "${MODULE_VERSION_NAME}";)
	$(file >> $@,extern const int api_version = ${RIRU_API_VERSION};)
	$(file >> $@,})
FORCE: ;

$(call import-module,prefab/cxx)
$(call import-module,prefab/riru)
$(call import-module,prefab/dobby)
