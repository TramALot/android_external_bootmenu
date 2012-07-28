LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bootmenu.c\
	bootmenu_ui.c\
	bootmenu_action.c

LOCAL_MODULE := bootmenu
LOCAL_MODULE_TAGS := eng
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(EXECUTABLES)/binary
LOCAL_STATIC_LIBRARIES := libminui_bm libpixelflinger_static libpng libz libstdc++ libcutils libc
LOCAL_CFLAGS += -DRECOVERY_API_VERSION=$(VERSION)

include $(BUILD_EXECUTABLE)
include $(LOCAL_PATH)/minui/Android.mk
