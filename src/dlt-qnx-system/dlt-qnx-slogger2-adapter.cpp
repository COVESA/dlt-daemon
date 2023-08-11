/**
 * Copyright (C) 2018-2020 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT QNX system functionality source file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Nguyen Dinh Thi <Thi.NguyenDinh@vn.bosch.com> ADIT 2018
 * \author Felix Herrmann <fherrmann@de.adit-jv.com> ADIT 2020
 *
 * \file: dlt-qnx-slogger2-adapter.cpp
 * For further information see http://www.covesa.org/.
 */
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <pthread.h>
#include <sys/slog2.h>
#include <sys/json.h>
#include <slog2_parse.h>
#include <thread>
#include <set>

#include "dlt-qnx-system.h"
#include "dlt_cpp_extension.hpp"
using std::chrono_literals::operator""ms;
using std::chrono_literals::operator""s;

/* Teach dlt about json_decoder_error_t */
template<>
inline int32_t logToDlt(DltContextData &log, const json_decoder_error_t &value)
{
    return logToDlt(log, static_cast<int>(value));
}

extern DltContext dltQnxSystem;

static DltContext dltQnxSlogger2Context;
static std::set<std::string> dltWarnedMissingMappings;

extern DltQnxSystemThreads g_threads;

extern volatile bool g_inj_disable_slog2_cb;

static std::unordered_map<std::string, DltContext*> g_slog2file;

static void dlt_context_map_read(const char *json_filename)
{
    DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_VERBOSE,
            "Loading Slog2Ctxt Map from json file: ", json_filename);

    auto dec = json_decoder_create();
    if (json_decoder_parse_file(dec, json_filename) != JSON_DECODER_OK) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_ERROR,
                "Could not load Slog2Ctxt Map from json file: ", json_filename);
        return;
    }

    const char *ctxtID, *name, *description;

    /* go to first element in dlt-slog2ctxt.json e.g. "ADIO" */
    auto ret = json_decoder_push_object(dec, nullptr, false);
    while (ret == JSON_DECODER_OK) {
        ctxtID = json_decoder_name(dec);

        /* go into the element e.g. { name: "", description: "" } */
        ret = json_decoder_push_object(dec, nullptr, false);
        if (ret != JSON_DECODER_OK) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN, __func__,
                    ": json parser error while descending into context dict. ret=", ret);
            break;
        }

        ret = json_decoder_get_string(dec, "name", &name, false);
        if (ret != JSON_DECODER_OK) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN, __func__,
                    ": json parser error while retrieving 'name' element of ", ctxtID, ". ret=", ret);
            break;
        }

        ret = json_decoder_get_string(dec, "description", &description, false);
        if (ret != JSON_DECODER_OK) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_WARN, __func__,
                    ": json parser error while retrieving 'description' element of ", ctxtID, ". ret=", ret);
            break;
        }

        auto ctxt = new DltContext;
        g_slog2file.emplace(name, ctxt);

        auto search = g_slog2file.find(name);
        if (search == g_slog2file.end()) {
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_INFO,
                    "Could not emplace slog2ctxt map key: ", name);
        } else {
            dlt_register_context(ctxt, ctxtID, description);
        }

        ret = json_decoder_pop(dec);
    }
    DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
            "Added ", g_slog2file.size(), " elements into the mapping table.");
}

/**
 * Map the slog2 logfile name to a dlt context
 * e.g. i2c_service.2948409 -> Context with id "I2CS"
 */
static DltContext *dlt_context_from_slog2file(const char *file_name) {
    auto d = strchr(file_name, '.');

    if (d == nullptr)
        return &dltQnxSlogger2Context;

    auto name = std::string(file_name).substr(0, d - file_name);

    auto search = g_slog2file.find(name);
    if (search == g_slog2file.end()) {
        // Only warn once about missing mapping.
        auto it = dltWarnedMissingMappings.find(name);
        if (it == dltWarnedMissingMappings.end()) {
            dltWarnedMissingMappings.insert(name);
            DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_INFO,
                        "slog2 filename not found in mapping: ", name.c_str());
        }

        return &dltQnxSlogger2Context;
    } else {
        return search->second;
    }
}

template <class time, class period>
static void wait_for_buffer_space(const double max_usage_threshold,
                                  const std::chrono::duration<time, period> max_wait_time) {
    int total_size = 0;
    int used_size = 0;
    double used_percent = 100.0;
    bool timeout = false;
    const auto end_time = std::chrono::steady_clock::now() + max_wait_time;

    do {
        dlt_user_check_buffer(&total_size, &used_size);
        used_percent = static_cast<double>(used_size) / total_size;
        if (used_percent < max_usage_threshold) {
            break;
        }

        dlt_user_log_resend_buffer();

        std::this_thread::sleep_for(10ms);
        timeout = std::chrono::steady_clock::now() < end_time;
    } while (!timeout);

    if (timeout) {
        DLT_LOG(dltQnxSystem, DLT_LOG_ERROR,
                DLT_STRING("failed to get enough buffer space"));

    }
}

/**
 *  Function which is invoked by slog2_parse_all()
 *  See slog2_parse_all api docs on qnx.com for details
 */
static int sloggerinfo_callback(slog2_packet_info_t *info, void *payload, void *param)
{
    DltQnxSystemConfiguration* conf = (DltQnxSystemConfiguration*) param;

    if (param == NULL)
        return -1;

    if (g_inj_disable_slog2_cb == true) {
        DLT_LOG(dltQnxSystem, DLT_LOG_INFO,
                DLT_STRING("Disabling slog2 callback by injection request."));
        return -1;
    }

    DltLogLevelType loglevel;
    switch (info->severity)
    {
        case SLOG2_SHUTDOWN:
        case SLOG2_CRITICAL:
            loglevel = DLT_LOG_FATAL;
            break;
        case SLOG2_ERROR:
            loglevel = DLT_LOG_ERROR;
            break;
        case SLOG2_WARNING:
            loglevel = DLT_LOG_WARN;
            break;
        case SLOG2_NOTICE:
        case SLOG2_INFO:
            loglevel = DLT_LOG_INFO;
            break;
        case SLOG2_DEBUG1:
            loglevel = DLT_LOG_DEBUG;
            break;
        case SLOG2_DEBUG2:
            loglevel = DLT_LOG_VERBOSE;
            break;
        default:
            loglevel = DLT_LOG_INFO;
            break;
    }

    DltContextData log_local; /* Used in DLT_* macros, do not rename */
    DltContext *ctxt = dlt_context_from_slog2file(info->file_name);

    wait_for_buffer_space(0.8, std::chrono::milliseconds(DLT_QNX_SLOG_ADAPTER_WAIT_BUFFER_TIMEOUT_MS));

    int ret;
    ret = dlt_user_log_write_start(ctxt, &log_local, loglevel);

    /* OK means loglevel under threshold */
    if (ret == DLT_RETURN_OK) {
        return 0;
    }

    if (ret != DLT_RETURN_TRUE) {
        fprintf(stderr, "%s: could not log to DLT status=%d\n", __func__, ret);
        return -1;
    }

    if (conf->qnxslogger2.useOriginalTimestamp == 1) {
	    /* convert from ns to .1 ms */
        log_local.user_timestamp = (uint32_t) (info->timestamp / 100000);
        log_local.use_timestamp = DLT_USER_TIMESTAMP;
    } else {
        DLT_UINT64(info->timestamp);
    }

    DLT_UINT16(info->sequence_number);
    DLT_STRING((char *)info->file_name);
    DLT_STRING((char *)info->buffer_name);
    DLT_UINT16(info->thread_id);
    DLT_UINT8(info->severity);
    DLT_STRING((char *)payload);

    dlt_user_log_write_finish(&log_local);

    return 0;
}

static void *slogger2_thread(void *v_conf)
{
    int ret = -1;
    DltQnxSystemConfiguration *conf = (DltQnxSystemConfiguration *)v_conf;

    if (v_conf == NULL)
        return reinterpret_cast<void*>(EINVAL);

    slog2_packet_info_t packet_info = SLOG2_PACKET_INFO_INIT;

    DLT_LOG(dltQnxSystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-qnx-slogger2-adapter, in thread."));

    /**
     * Thread will block inside this function to get new log because
     * flag = SLOG2_PARSE_FLAGS_DYNAMIC
     */
    ret = slog2_parse_all(
            SLOG2_PARSE_FLAGS_DYNAMIC, /* live streaming of all buffers merged */
            NULL, NULL, &packet_info, sloggerinfo_callback, (void*) conf);

    if (ret == -1) {
        DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_ERROR,
                "slog2_parse_all() returned error=", ret);
        ret = EBADMSG;
    }

    DLT_LOG_CXX(dltQnxSystem, DLT_LOG_DEBUG, __func__, ": Exited main loop.");

    DLT_UNREGISTER_CONTEXT(dltQnxSlogger2Context);

    /* process should be shutdown if the callback was not manually disabled */
    if (g_inj_disable_slog2_cb == false) {
        for (auto& x: g_slog2file) {
            if(x.second != NULL) {
                delete(x.second);
                x.second = NULL;
            }
        }
	/* Send a signal to main thread to wake up sigwait */
        pthread_kill(g_threads.mainThread, SIGTERM);
    }

    return reinterpret_cast<void*>(ret);
}

void start_qnx_slogger2(DltQnxSystemConfiguration *conf)
{
    static pthread_attr_t t_attr;
    static pthread_t pt;

    DLT_REGISTER_CONTEXT(dltQnxSlogger2Context, conf->qnxslogger2.contextId,
                         "SLOGGER2 Adapter");

    dlt_context_map_read(CONFIGURATION_FILES_DIR "/dlt-slog2ctxt.json");

    DLT_LOG_CXX(dltQnxSlogger2Context, DLT_LOG_DEBUG,
            "dlt-qnx-slogger2-adapter, start syslog");
    pthread_create(&pt, &t_attr, slogger2_thread, conf);
    g_threads.threads[g_threads.count++] = pt;
}
