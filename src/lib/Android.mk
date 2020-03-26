LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := dlt

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include/dlt \
	$(LOCAL_PATH)/../shared \
	$(LOCAL_PATH)/../../musl/include

LOCAL_CFLAGS := $(JOKER_CFLAGS) -Wno-date-time -DDLT_USER_IPC_PATH=\"/tmp\"

DLT_LIB_SRCS := \
	dlt_user.c \
	dlt_client.c \
	dlt_filetransfer.c \
	dlt_env_ll.c \
	../shared/dlt_common.c \
	../shared/dlt_user_shared.c \
	../shared/dlt_protocol.c

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(foreach F, $(DLT_LIB_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))
LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES := musl
LOCAL_LDLIBS := -ldl

include $(BUILD_SHARED_LIBRARY)


