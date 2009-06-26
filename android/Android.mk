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

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libutils

include $(BUILD_EXECUTABLE)

# Test for PhoneNumberUtils
#
# You can also test this in Unix, like this:
# > g++ -Wall external/sqlite/android/PhoneNumberUtils.cpp \
#   external/sqlite/android/PhoneNumberUtilsTest.cpp
# > ./a.out
#
# Note: tests related to PHONE_NUMBERS_EQUAL also exists in AndroidTests in
# java space. Add tests if you modify this.

include $(CLEAR_VARS)

LOCAL_MODULE:= libsqlite3_phone_number_utils_test

LOCAL_CFLAGS += -Wall -Werror

LOCAL_SRC_FILES := \
	PhoneNumberUtils.cpp \
	PhoneNumberUtilsTest.cpp

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
