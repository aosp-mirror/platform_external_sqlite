LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	PhoneNumberUtils.cpp \
	sqlite3_android.cpp

LOCAL_C_INCLUDES := \
        external/sqlite/dist \
        external/icu4c/i18n \
        external/icu4c/common


LOCAL_MODULE:= libsqlite3_android

include $(BUILD_STATIC_LIBRARY)
