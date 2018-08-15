LOCAL_PATH:= $(call my-dir)
 
 
include $(CLEAR_VARS)
#LOCAL_C_INCLUDES:= external/tinyalsa/include
LOCAL_SRC_FILES:= list_file.c
LOCAL_MODULE := listfile
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false


include $(BUILD_EXECUTABLE)