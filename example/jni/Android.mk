# Setup a project
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := example
LOCAL_SRC_FILES := example.cpp
LOCAL_CPPFLAGS += -Wall -Wshadow -Wextra -pedantic -std=c++11 -fPIE -pie
LOCAL_LDFLAGS +=  -fPIE -pie

# Add exception support and set path for spdlog's headers
LOCAL_CPPFLAGS += -fexceptions -I../include
# Use android's log library
LOCAL_LDFLAGS += -llog

include $(BUILD_EXECUTABLE)
