LOCAL_PATH:= $(call my-dir)

libsqlite3_android_local_src_files := \
	PhoneNumberUtils.cpp \
	OldPhoneNumberUtils.cpp \
	sqlite3_android.cpp

libsqlite3_android_c_includes := external/sqlite/dist

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(libsqlite3_android_local_src_files)
LOCAL_C_INCLUDES += $(libsqlite3_android_c_includes)
LOCAL_STATIC_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES := libicuuc libicui18n
LOCAL_MODULE:= libsqlite3_android
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(libsqlite3_android_local_src_files)
LOCAL_C_INCLUDES += $(libsqlite3_android_c_includes)
LOCAL_STATIC_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES := libicuuc libicui18n
LOCAL_MODULE:= libsqlite3_android
include $(BUILD_HOST_STATIC_LIBRARY)

#       The formal test for phone_number_compare() is in DataBaseGeneralTest.java
#       (as of 2009-08-02), in which phone_number_compare() is tested via sqlite's custom
#       function "PHONE_NUMBER_COMPARE".
include $(CLEAR_VARS)
LOCAL_MODULE:= libsqlite3_phone_number_utils_test
LOCAL_CFLAGS += -Wall -Werror
LOCAL_SRC_FILES := PhoneNumberUtils.cpp PhoneNumberUtilsTest.cpp
include $(BUILD_NATIVE_TEST)
