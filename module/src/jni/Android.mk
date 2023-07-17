LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := zygiskfrida
LOCAL_SRC_FILES := main.cpp inject.cpp
LOCAL_EXPORT_STATIC_LIBRARIES := libcxx
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include src/jni/libcxx/Android.mk
