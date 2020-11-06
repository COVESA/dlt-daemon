/**
 * @licence app begin@
 * Copyright (C) 2019 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT logd converter: Retrieve log entries from logd and forward them to DLT.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file: dlt-logd-convert
 * For further information see http://www.genivi.org/.
 * @licence end@
 */
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include <log/log_read.h>
#include <log/logprint.h>

#include <dlt.h>

DLT_DECLARE_CONTEXT(dlt_ctx_self)
DLT_DECLARE_CONTEXT(dlt_ctx_main)
DLT_DECLARE_CONTEXT(dlt_ctx_rdio)
DLT_DECLARE_CONTEXT(dlt_ctx_evnt) /* Binary Buffer */
DLT_DECLARE_CONTEXT(dlt_ctx_syst)
DLT_DECLARE_CONTEXT(dlt_ctx_crsh)
DLT_DECLARE_CONTEXT(dlt_ctx_stat) /* Binary Buffer */
DLT_DECLARE_CONTEXT(dlt_ctx_secu) /* Binary Buffer */
DLT_DECLARE_CONTEXT(dlt_ctx_krnl)

volatile sig_atomic_t exit_parser_loop = false;

static inline struct logger *init_logger(struct logger_list *logger_list, log_id_t log_id)
{
    struct logger *logger;
    logger = android_logger_open(logger_list, log_id);
    if (logger == nullptr) {
        DLT_LOG(dlt_ctx_self, DLT_LOG_WARN, DLT_STRING("could not open logd buffer id="), DLT_INT64(log_id));
    }
    return logger;
}

static struct logger_list *init_logger_list(bool skip_binary_buffers)
{
    struct logger_list *logger_list;
    logger_list = android_logger_list_alloc(ANDROID_LOG_RDONLY, 0, 0);
    if (logger_list == nullptr) {
        DLT_LOG(dlt_ctx_self, DLT_LOG_FATAL, DLT_STRING("could not allocate logger list"));
        return nullptr;
    }

    /**
     * logd buffer types are defined in:
     * system/core/include/log/android/log.h
     */
    init_logger(logger_list, LOG_ID_MAIN);
    init_logger(logger_list, LOG_ID_RADIO);
    init_logger(logger_list, LOG_ID_SYSTEM);
    init_logger(logger_list, LOG_ID_KERNEL);
    init_logger(logger_list, LOG_ID_CRASH);

    if (!skip_binary_buffers) {
        init_logger(logger_list, LOG_ID_EVENTS);
        init_logger(logger_list, LOG_ID_STATS);
        init_logger(logger_list, LOG_ID_SECURITY);
    }

    return logger_list;
}

static DltContext *get_log_context_from_log_msg(struct log_msg *log_msg)
{
    switch (log_msg->id()) {
    case LOG_ID_MAIN:
        return &dlt_ctx_main;
    case LOG_ID_RADIO:
        return &dlt_ctx_rdio;
    case LOG_ID_EVENTS:
        return &dlt_ctx_evnt;
    case LOG_ID_SYSTEM:
        return &dlt_ctx_syst;
    case LOG_ID_CRASH:
        return &dlt_ctx_crsh;
    case LOG_ID_STATS:
        return &dlt_ctx_stat;
    case LOG_ID_SECURITY:
        return &dlt_ctx_secu;
    case LOG_ID_KERNEL:
        return &dlt_ctx_krnl;
    default:
        return &dlt_ctx_self;
    }
}

static uint32_t get_timestamp_from_log_msg(struct log_msg *log_msg)
{
    /* in 0.1 ms = 100 us */
    return (uint32_t)log_msg->entry.sec * 10000 + (uint32_t)log_msg->entry.nsec / 100000;
}

static DltLogLevelType get_log_level_from_log_msg(struct log_msg *log_msg)
{
    android_LogPriority priority = static_cast<android_LogPriority>(log_msg->buf[0]);
    switch (priority) {
    case ANDROID_LOG_VERBOSE:
        return DLT_LOG_VERBOSE;
    case ANDROID_LOG_DEBUG:
        return DLT_LOG_DEBUG;
    case ANDROID_LOG_INFO:
        return DLT_LOG_INFO;
    case ANDROID_LOG_WARN:
        return DLT_LOG_WARN;
    case ANDROID_LOG_ERROR:
        return DLT_LOG_ERROR;
    case ANDROID_LOG_FATAL:
        return DLT_LOG_FATAL;
    case ANDROID_LOG_SILENT:
        return DLT_LOG_OFF;
    case ANDROID_LOG_UNKNOWN:
    case ANDROID_LOG_DEFAULT:
    default:
        return DLT_LOG_DEFAULT;
    }
}

void signal_handler(int signal)
{
    (void) signal;
    if (signal == SIGTERM) {
        exit_parser_loop = true;
    }
}

static int logd_parser_loop(struct logger_list *logger_list)
{
    struct log_msg log_msg;
    int ret;

    DLT_LOG(dlt_ctx_self, DLT_LOG_VERBOSE, DLT_STRING("Entering parsing loop"));

    while (!exit_parser_loop) {
        ret = android_logger_list_read(logger_list, &log_msg);
        if (ret == -EAGAIN || ret == -EINTR) {
            if (exit_parser_loop == true) {
                break;
            }
            continue;
        } else if (ret == -EINVAL || ret == -ENOMEM || ret == -ENODEV || ret == -EIO) {
            DLT_LOG(dlt_ctx_self, DLT_LOG_FATAL, DLT_STRING("Could not cannot retrieve logs, permanent error="), DLT_INT32(ret));
            return ret;
        } else if (ret <= 0) {
            DLT_LOG(dlt_ctx_self, DLT_LOG_ERROR, DLT_STRING("android_logger_list_read unexpected return="), DLT_INT32(ret));
            return ret;
        }

        DltContext *ctx = nullptr;
        ctx = get_log_context_from_log_msg(&log_msg);

        DltLogLevelType log_level;
        log_level = get_log_level_from_log_msg(&log_msg);

        /* Look into system/core/liblog/logprint.c for buffer format */
        auto tag = log_msg.msg()+1;
        auto message = tag+strlen(tag)+1;

        uint32_t ts;
        ts = get_timestamp_from_log_msg(&log_msg);

        /* Binary buffers are not supported by DLT_STRING DLT_RAW would need the message length */
        DLT_LOG_TS(*ctx, log_level, ts,
                    DLT_STRING(tag),
                    DLT_INT32(log_msg.entry.pid),
                    DLT_UINT32(log_msg.entry.tid),
                    DLT_STRING(message));
    }

    DLT_LOG(dlt_ctx_self, DLT_LOG_VERBOSE, DLT_STRING("Exited parsing loop"));

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    bool skip_binary_buffers = true;

    DLT_REGISTER_APP("LOGD", "logd -> dlt adapter");
    DLT_REGISTER_CONTEXT(dlt_ctx_self, "LOGF", "logd retriever");
    DLT_REGISTER_CONTEXT(dlt_ctx_main, "MAIN", "logd type: main");
    DLT_REGISTER_CONTEXT(dlt_ctx_rdio, "RDIO", "logd type: rdio");
    DLT_REGISTER_CONTEXT(dlt_ctx_syst, "SYST", "logd type: syst");
    DLT_REGISTER_CONTEXT(dlt_ctx_crsh, "CRSH", "logd type: crsh");
    DLT_REGISTER_CONTEXT(dlt_ctx_krnl, "KRNL", "logd type: krnl");
    if(!skip_binary_buffers){
        DLT_REGISTER_CONTEXT(dlt_ctx_evnt, "EVNT", "logd type: evnt");
        DLT_REGISTER_CONTEXT(dlt_ctx_stat, "STAT", "logd type: stat");
        DLT_REGISTER_CONTEXT(dlt_ctx_secu, "SECU", "logd type: secu");
    }

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTERM, &act, 0);

    struct logger_list *logger_list;
    /* Binary buffers are currently not supported */
    logger_list = init_logger_list(skip_binary_buffers);
    if (logger_list == nullptr)
        return EXIT_FAILURE;

    int ret;
    ret = logd_parser_loop(logger_list); /* Main loop */

    android_logger_list_free(logger_list);

    DLT_UNREGISTER_CONTEXT(dlt_ctx_krnl);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_secu);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_stat);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_crsh);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_syst);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_evnt);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_rdio);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_main);
    DLT_UNREGISTER_CONTEXT(dlt_ctx_self);
    if(!skip_binary_buffers){
        DLT_UNREGISTER_CONTEXT(dlt_ctx_evnt);
        DLT_UNREGISTER_CONTEXT(dlt_ctx_stat);
        DLT_UNREGISTER_CONTEXT(dlt_ctx_secu);
    }

    DLT_UNREGISTER_APP_FLUSH_BUFFERED_LOGS();

    return ret;
}
