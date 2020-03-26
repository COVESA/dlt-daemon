LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := musl

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_CFLAGS := $(JOKER_CFLAGS) 

MUSL_SRCS := src/mq/mq_unlink.c \
	src/mq/mq_open.c \
	src/mq/mq_close.c \
	src/mq/mq_send.c \
	src/mq/mq_receive.c \
	src/mq/mq_timedsend.c \
	src/mq/mq_timedreceive.c \
	src/ipc/shmctl.c \
	src/ipc/shmdt.c \
	src/ipc/shmget.c \
	src/ipc/shmat.c \
	src/ipc/semctl.c \
	src/ipc/semget.c \
	src/ipc/semop.c \
	src/linux/timerfd.c

LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(foreach F, $(MUSL_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))
LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES :=
LOCAL_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)
