LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := dlt-convert

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include/dlt \
	$(LOCAL_PATH)/../shared \
	$(LOCAL_PATH)/../../musl/include

LOCAL_CFLAGS := $(JOKER_CFLAGS) -Wno-date-time

DLT_DAEMON_SRCS := \
	dlt-convert.c \
	../shared/dlt_common.c

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(foreach F, $(DLT_DAEMON_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))
#LOCAL_SHARED_LIBRARIES :=
#LOCAL_STATIC_LIBRARIES := musl
LOCAL_LDLIBS := -ldl

include $(BUILD_EXECUTABLE)

#===================================
include $(CLEAR_VARS)

LOCAL_MODULE := dlt-receive

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include/dlt \
	$(LOCAL_PATH)/../shared \
	$(LOCAL_PATH)/../../musl/include

LOCAL_CFLAGS := $(JOKER_CFLAGS) -Wno-date-time

DLT_DAEMON_SRCS := \
	$(LOCAL_MODULE).c \
	../shared/dlt_common.c \
	../lib/dlt_client.c

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(foreach F, $(DLT_DAEMON_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))
#LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES := musl
LOCAL_LDLIBS := -ldl

include $(BUILD_EXECUTABLE)
#===================================


#===================================
include $(CLEAR_VARS)

LOCAL_MODULE := dlt-control

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include/dlt \
	$(LOCAL_PATH)/../shared \
	$(LOCAL_PATH)/../../musl/include

LOCAL_CFLAGS := $(JOKER_CFLAGS) -Wno-date-time -DCONFIGURATION_FILES_DIR=\"/system/etc/dlt\"

DLT_DAEMON_SRCS := \
	$(LOCAL_MODULE).c \
	dlt-control-common.c \
	../shared/dlt_common.c \
	../lib/dlt_client.c \
	../shared/dlt_protocol.c

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(foreach F, $(DLT_DAEMON_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))
#LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES := musl
LOCAL_LDLIBS := -ldl

include $(BUILD_EXECUTABLE)
#===================================


