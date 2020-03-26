LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := dlt-daemon

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../include/dlt \
	$(LOCAL_PATH)/../shared \
	$(LOCAL_PATH)/../../musl/include \
	$(LOCAL_PATH)/../offlinelogstorage \
	$(LOCAL_PATH)/../gateway \


LOCAL_CFLAGS := $(JOKER_CFLAGS) -Wno-date-time -DCONFIGURATION_FILES_DIR=\"/data/tmp\" -DDLT_USER_IPC_PATH=\"/data/tmp/ipc\" -DDLT_USE_UNIX_SOCKET_IPC
# -DDLT_GATEWAY_CONFIG_PATH=\"/data/tmp\"

DLT_DAEMON_SRCS := \
	dlt-daemon.c \
	dlt_daemon_common.c \
	dlt_daemon_connection.c \
	dlt_daemon_event_handler.c \
	dlt_daemon_offline_logstorage.c \
	dlt_daemon_socket.c \
	dlt_daemon_unix_socket.c \
	dlt_daemon_serial.c \
	dlt_daemon_client.c \
	../gateway/dlt_gateway.c \
	../lib/dlt_client.c \
	../shared/dlt_config_file_parser.c \
	../shared/dlt_protocol.c \
	../shared/dlt_user_shared.c \
	../shared/dlt_common.c \
	../shared/dlt_offline_trace.c \
	../offlinelogstorage/dlt_offline_logstorage.c \
	../offlinelogstorage/dlt_offline_logstorage_behavior.c

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(foreach F, $(DLT_DAEMON_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))
LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES := musl
LOCAL_LDLIBS := -ldl

include $(BUILD_EXECUTABLE)


