LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_STL=none
LOCAL_MODULE := zygiskfrida
LOCAL_SRC_FILES := main.cpp inject.cpp
LOCAL_STATIC_LIBRARIES := cxx
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

$(call import-module,prefab/cxx)
