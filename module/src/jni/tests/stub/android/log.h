// Host stub for <android/log.h> so the JNI sources can be compiled and unit
// tested off-device (the real header ships only with the Android NDK).
#ifndef ZYGISKFRIDA_TEST_STUB_ANDROID_LOG_H
#define ZYGISKFRIDA_TEST_STUB_ANDROID_LOG_H

enum android_LogPriority {
  ANDROID_LOG_DEFAULT = 1,
  ANDROID_LOG_VERBOSE = 2,
  ANDROID_LOG_DEBUG = 3,
  ANDROID_LOG_INFO = 4,
  ANDROID_LOG_WARN = 5,
  ANDROID_LOG_ERROR = 6,
};

// No-op: tests don't care about log output, only parsing behaviour.
static inline int __android_log_print(int /*prio*/, const char * /*tag*/,
                                      const char * /*fmt*/, ...) {
  return 0;
}

#endif  // ZYGISKFRIDA_TEST_STUB_ANDROID_LOG_H
