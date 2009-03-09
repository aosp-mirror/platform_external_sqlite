##
##
## Build the library
##
##

LOCAL_PATH:= $(call my-dir)

common_src_files := sqlite3.c

# the device library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_src_files)

ifneq ($(TARGET_ARCH),arm)
LOCAL_LDLIBS += -lpthread -ldl
endif

LOCAL_CFLAGS += -DHAVE_USLEEP=1 -DSQLITE_DEFAULT_JOURNAL_SIZE_LIMIT=1048576 -DSQLITE_THREADSAFE=1 -DNDEBUG=1 -DSQLITE_ENABLE_MEMORY_MANAGEMENT=1 -DSQLITE_DEFAULT_AUTOVACUUM=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_BACKWARDS -DSQLITE_ENABLE_POISON

ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES := libdl
endif

LOCAL_MODULE:= libsqlite
#new sqlite 3.5.6 no longer support external allocator
#LOCAL_CFLAGS += -DSQLITE_OMIT_MEMORY_ALLOCATION
LOCAL_C_INCLUDES += $(call include-path-for, system-core)/cutils
LOCAL_SHARED_LIBRARIES += liblog \
            libicuuc \
            libicui18n

# include android specific methods
LOCAL_WHOLE_STATIC_LIBRARIES := libsqlite3_android

## Choose only one of the allocator systems below
# new sqlite 3.5.6 no longer support external allocator 
#LOCAL_SRC_FILES += mem_malloc.c
#LOCAL_SRC_FILES += mem_mspace.c


include $(BUILD_SHARED_LIBRARY)

##
##
## Build the device command line tool sqlite3
##
##
ifneq ($(SDK_ONLY),true)  # SDK doesn't need device version of sqlite3

include $(CLEAR_VARS)

LOCAL_SRC_FILES := shell.c

LOCAL_SHARED_LIBRARIES := libsqlite

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../android


ifneq ($(TARGET_ARCH),arm)
LOCAL_LDLIBS += -lpthread -ldl
endif

LOCAL_CFLAGS += -DHAVE_USLEEP=1 -DTHREADSAFE=1 -DNDEBUG=1

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := sqlite3

include $(BUILD_EXECUTABLE)

endif # !SDK_ONLY


##
##
## Build the host command line tool sqlite3
##
##

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_src_files) shell.c

LOCAL_CFLAGS += -DHAVE_USLEEP=1 -DSQLITE_DEFAULT_JOURNAL_SIZE_LIMIT=1048576 -DSQLITE_THREADSAFE=1 -DNDEBUG=1 -DSQLITE_ENABLE_MEMORY_MANAGEMENT=1 -DNO_ANDROID_FUNCS=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_BACKWARDS -DSQLITE_ENABLE_POISON

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../android

# sqlite3MemsysAlarm uses LOG()
LOCAL_STATIC_LIBRARIES += liblog


have_readline := $(wildcard /usr/include/readline/readline.h)
have_history := $(wildcard /usr/lib/libhistory*)
ifneq ($(strip $(have_readline)),)
LOCAL_CFLAGS += -DHAVE_READLINE=1
endif

LOCAL_LDLIBS += -lpthread -ldl

ifneq ($(strip $(have_readline)),)
LOCAL_LDLIBS += -lreadline
endif
ifneq ($(strip $(have_history)),)
LOCAL_LDLIBS += -lhistory
endif

LOCAL_MODULE := sqlite3

include $(BUILD_HOST_EXECUTABLE)

