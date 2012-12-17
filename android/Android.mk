LOCAL_PATH:= $(call my-dir)

libsqlite3_android_local_src_files := \
	PhoneNumberUtils.cpp \
	OldPhoneNumberUtils.cpp \
	PhonebookIndex.cpp \
	sqlite3_android.cpp

libsqlite3_android_c_includes := \
        external/sqlite/dist \
        external/icu4c/i18n \
        external/icu4c/common \
        frameworks/native/include

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(libsqlite3_android_local_src_files)
LOCAL_C_INCLUDES := $(libsqlite3_android_c_includes)
LOCAL_MODULE:= libsqlite3_android
include $(BUILD_STATIC_LIBRARY)

ifeq ($(WITH_HOST_DALVIK),true)
    include $(CLEAR_VARS)
    LOCAL_SRC_FILES:= $(libsqlite3_android_local_src_files)
    LOCAL_C_INCLUDES := $(libsqlite3_android_c_includes)
    LOCAL_MODULE:= libsqlite3_android
    include $(BUILD_HOST_STATIC_LIBRARY)
endif

# Test for PhoneNumberUtils
#
# You can also test this in Unix, like this:
# > g++ -Wall external/sqlite/android/PhoneNumberUtils.cpp \
#   external/sqlite/android/PhoneNumberUtilsTest.cpp
# > ./a.out
#
# Note: This "test" is not recognized as a formal test. This is just for enabling developers
#       to easily check what they modified works well or not.
#       The formal test for phone_number_compare() is in DataBaseGeneralTest.java
#       (as of 2009-08-02), in which phone_number_compare() is tested via sqlite's custom
#       function "PHONE_NUMBER_COMPARE".
#       Please add tests if you modify the implementation of PhoneNumberUtils.cpp and add
#       test cases in PhoneNumberUtilsTest.cpp.
include $(CLEAR_VARS)

LOCAL_MODULE:= libsqlite3_phone_number_utils_test

LOCAL_CFLAGS += -Wall -Werror

LOCAL_SRC_FILES := \
	PhoneNumberUtils.cpp \
	PhoneNumberUtilsTest.cpp

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

ifeq ($(WITH_HOST_DALVIK),true)
  include $(CLEAR_VARS)

  LOCAL_MODULE:= libsqlite3_phone_book_index_test

  LOCAL_SRC_FILES := \
	PhonebookIndex.cpp \
	PhonebookIndexTest.cpp

  LOCAL_C_INCLUDES := \
        external/icu4c/i18n \
        external/icu4c/common \
        frameworks/native/include

  LOCAL_MODULE_TAGS := optional

  LOCAL_SHARED_LIBRARIES := \
	libicui18n libicuuc

  LOCAL_STATIC_LIBRARIES := \
	libutils libcutils

  include $(BUILD_HOST_EXECUTABLE)
endif
