LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := trace_server
LOCAL_SRC_FILES := trace_server.c
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY) 