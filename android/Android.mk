LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	PhoneNumberUtils.cpp \
	PhoneticStringUtils.cpp \
	sqlite3_android.cpp

LOCAL_C_INCLUDES := \
        external/sqlite/dist \
        external/icu4c/i18n \
        external/icu4c/common


LOCAL_MODULE:= libsqlite3_android

include $(BUILD_STATIC_LIBRARY)

# Test for PhoneticStringUtils
include $(CLEAR_VARS)

LOCAL_MODULE:= libsqlite3_phonetic_string_utils_test

LOCAL_CFLAGS += -Wall -Werror

LOCAL_SRC_FILES := \
	PhoneticStringUtils.cpp \
	PhoneticStringUtilsTest.cpp

LOCAL_MODULE_TAGS := tests optional

include $(BUILD_EXECUTABLE)
