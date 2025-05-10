LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := traceserver
LOCAL_SRC_FILES := traceserver.c

# Add required libraries
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_EXECUTABLE) 