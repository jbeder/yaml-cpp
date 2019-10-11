LOCAL_PATH := $(call my-dir)
# --------------------------------------
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/src/*.cpp)

LOCAL_C_INCLUDES += src/
LOCAL_C_INCLUDES += include/

LOCAL_LDLIBS := -llog

LOCAL_MODULE := yamlcpp
include $(BUILD_SHARED_LIBRARY)

