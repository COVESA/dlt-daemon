/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2022, Amazon.com, Inc. or its affiliates.
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 */

/*!
 * \author
 * Haris Okanovic <harisokn@amazon.com>
 *
 * \copyright Copyright (C) 2022 Amazon.com, Inc. or its affiliates. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include "dlt_common.h"
#include "dlt-daemon.h"
#include "dlt_daemon_client.h"
#include "dlt_daemon_connection.h"
#include "dlt_types.h"

#include "dlt_sdjournal.h"
#include <dlt_sdjournal_chmap.h>
#include "dlt_sdjournal_types.h"

#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)

#include <systemd/sd-journal.h>

// assumed dlt_vlog() calls below with "%.4s"
static_assert(DLT_ID_SIZE == 4);

int dlt_sdjournal_init(DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (!daemon_local) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    DltSdJournal *sdj = &daemon_local->pSdJournal;
    memset(sdj, 0, sizeof(DltSdJournal));

    sd_id128_t currentBootId = {0};
    int result = sd_id128_get_boot(&currentBootId);
    if (result != 0) {
        dlt_vlog(LOG_ERR, "%s: sd_id128_get_boot() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    /* open a handle to all system logs */
    result = sd_journal_open(&sdj->handle, SD_JOURNAL_LOCAL_ONLY);
    if (result != 0 || !sdj->handle) {
        dlt_vlog(LOG_ERR, "%s: sd_journal_open() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    result = sd_journal_seek_head(sdj->handle);
    if (result != 0) {
        dlt_vlog(LOG_ERR, "%s: sd_journal_seek_head() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    const int sdjFd = sd_journal_get_fd(sdj->handle);
    if (sdjFd < 0) {
        dlt_vlog(LOG_ERR, "%s: sd_journal_get_fd() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    const int sdjEvents = sd_journal_get_events(sdj->handle);
    if (sdjEvents < 0) {
        dlt_vlog(LOG_ERR, "%s: sd_journal_get_events() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    result = dlt_receiver_init(&sdj->receiver, sdjFd, DLT_RECEIVE_FD, 0);
    if (result < 0) {
        dlt_vlog(LOG_ERR, "%s: dlt_receiver_init() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    /* skip entries from previous boots */
    unsigned int skipCount = 0;
    uint64_t skipEntryTime = 0;
    sd_id128_t skipEntryBootId = {0};
    do {
        result = sd_journal_next(sdj->handle);
        if (result < 0) {
            dlt_vlog(LOG_ERR, "%s: sd_journal_next() failed: result=%d, errno=%d\n",
                __func__, result, errno);
            dlt_sdjournal_deinit(daemon_local, verbose);
            return DLT_RETURN_ERROR;
        }
        if (result == 0) {
            /* no more entries, next entry will be from current boot */
            break;
        }

        result = sd_journal_get_monotonic_usec(sdj->handle, &skipEntryTime, &skipEntryBootId);
        if (result < 0) {
            dlt_vlog(LOG_ERR, "%s: sd_journal_get_monotonic_usec() failed: result=%d, errno=%d\n",
                __func__, result, errno);
            dlt_sdjournal_deinit(daemon_local, verbose);
            return DLT_RETURN_ERROR;
        }

        ++skipCount;
    } while (!sd_id128_equal(skipEntryBootId, currentBootId));
    dlt_vlog(LOG_DEBUG, "%s: skipped %u entries from prior boots\n", __func__, skipCount);

    result = dlt_connection_create(
        daemon_local,
        &daemon_local->pEvent,
        sdjFd,
        sdjEvents,
        DLT_CONNECTION_JOURNAL_GATEWAY
    );
    if (result != 0) {
        dlt_vlog(LOG_ERR, "%s: dlt_connection_create() failed for journal reader: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    sdj->defaultLogLevel = DLT_LOG_VERBOSE;

    const size_t kChmapTableSize = 16 * 1024;
    sdj->chmap = dlt_sdjournal_chmap_create(kChmapTableSize);
    if (!sdj->chmap) {
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    dlt_vlog(LOG_INFO, "%s: Systemd Journal reader initialized\n", __func__);
    return DLT_RETURN_OK;
}

void dlt_sdjournal_deinit(DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (!daemon_local) {
        return;
    }

    DltSdJournal *sdj = &daemon_local->pSdJournal;
    if (!sdj->handle) {
        return;
    }

    dlt_sdjournal_chmap_delete(sdj->chmap);

    dlt_receiver_free(&sdj->receiver);

    sd_journal_close(sdj->handle);
    sdj->handle = NULL;
}

static long unsigned int syslog_severity_to_dlt_level(long unsigned int syslogSeverity)
{
    switch (syslogSeverity) {
        case LOG_EMERG:
        case LOG_ALERT:
        case LOG_CRIT:
            return DLT_LOG_FATAL;
        case LOG_ERR:
            return DLT_LOG_ERROR;
        case LOG_WARNING:
            return DLT_LOG_WARN;
        case LOG_NOTICE:
        case LOG_INFO:
            return DLT_LOG_INFO;
        case LOG_DEBUG:
            return DLT_LOG_DEBUG;
        default:
            return DLT_LOG_INFO;
    };
}

static const char* syslog_facility_to_dlt_context(long unsigned int syslogFacility)
{
    switch (syslogFacility) {
        case LOG_FAC(LOG_KERN):
            return "KERN";
        case LOG_FAC(LOG_USER):
            return "USER";
        case LOG_FAC(LOG_MAIL):
            return "MAIL";
        case LOG_FAC(LOG_DAEMON):
            return "DAEM";
        case LOG_FAC(LOG_AUTH):
            return "AUTH";
        case LOG_FAC(LOG_SYSLOG):
            return "SYSL";
        case LOG_FAC(LOG_LPR):
            return "LPR";
        case LOG_FAC(LOG_NEWS):
            return "NEWS";
        case LOG_FAC(LOG_UUCP):
            return "UUCP";
        case LOG_FAC(LOG_CRON):
            return "CRON";
        case LOG_FAC(LOG_AUTHPRIV):
            return "AUTP";
        case LOG_FAC(LOG_FTP):
            return "FTP";
        case LOG_FAC(LOG_LOCAL0):
            return "LOC0";
        case LOG_FAC(LOG_LOCAL1):
            return "LOC1";
        case LOG_FAC(LOG_LOCAL2):
            return "LOC2";
        case LOG_FAC(LOG_LOCAL3):
            return "LOC3";
        case LOG_FAC(LOG_LOCAL4):
            return "LOC4";
        case LOG_FAC(LOG_LOCAL5):
            return "LOC5";
        case LOG_FAC(LOG_LOCAL6):
            return "LOC6";
        case LOG_FAC(LOG_LOCAL7):
            return "LOC7";
        default:
            return "";
    }
}

static void syslog_id_to_dlt_app_id(char* appId, const char* syslogId)
{
    if (!appId || !syslogId)
        return;

    memset(appId, 0, DLT_ID_SIZE);

    int appIdIdx = 0;
    int syslogIdIdx = 0;

    while (appIdIdx < DLT_ID_SIZE && syslogId[syslogIdIdx] != '\0') {
        const char c = syslogId[syslogIdIdx++];

        if (isascii(c) && isalnum(c)) {
            appId[appIdIdx++] = toupper(c);
        }
    }
}

static int dlt_sdjournal_send_message(DltDaemon *daemon, DltDaemonLocal *daemon_local,
                                      long unsigned int timestamp, long unsigned int pid,
                                      const char* appId, const char* ctxId, long unsigned int level,
                                      uint8_t msgCount, const char* str,
                                      int verbose)
{
    DltMessage msg = { 0 };
    DltStandardHeaderExtra *pStandardExtra = NULL;
    uint32_t uiType;
    uint16_t uiSize;
    uint32_t uiExtraSize;

    PRINT_FUNCTION_VERBOSE(verbose);

    /* Set storageheader */
    msg.storageheader = (DltStorageHeader *)(msg.headerbuffer);
    dlt_set_storageheader(msg.storageheader, daemon->ecuid);

    /* Set standardheader */
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_UEH | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS |
        DLT_HTYP_PROTOCOL_VERSION1;
    msg.standardheader->mcnt = msgCount;

    uiExtraSize = (uint32_t) (DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) +
        (DLT_IS_HTYP_UEH(msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0));
    msg.headersize = (uint32_t) sizeof(DltStorageHeader) + (uint32_t) sizeof(DltStandardHeader) + uiExtraSize;

    /* Set extraheader */
    pStandardExtra =
        (DltStandardHeaderExtra *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader));
    dlt_set_id(pStandardExtra->ecu, daemon->ecuid);
    pStandardExtra->tmsp = DLT_HTOBE_32(timestamp);
    pStandardExtra->seid = DLT_HTOBE_32(pid);

    /* Set extendedheader */
    msg.extendedheader =
        (DltExtendedHeader *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                              DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_VERB | (DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) |
        ((level << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN);
    msg.extendedheader->noar = 1;
    dlt_set_id(msg.extendedheader->apid, appId);
    dlt_set_id(msg.extendedheader->ctid, ctxId);

    /* Set payload data... */
    uiType = DLT_TYPE_INFO_STRG;
    uiSize = (str != NULL) ? (uint16_t)(strlen(str) + 1) : (uint16_t)0;
    msg.datasize = (uint32_t) (sizeof(uint32_t) + sizeof(uint16_t) + uiSize);

    msg.databuffer = (uint8_t *)malloc((size_t) msg.datasize);
    msg.databuffersize = msg.datasize;

    if (msg.databuffer == 0) {
        dlt_log(LOG_WARNING, "Can't allocate buffer for get log info message\n");
        return DLT_RETURN_ERROR;
    }

    msg.datasize = 0;
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiType), sizeof(uint32_t));
    msg.datasize += (uint32_t) sizeof(uint32_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiSize), sizeof(uint16_t));
    msg.datasize += (uint32_t) sizeof(uint16_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), str, uiSize);
    msg.datasize += uiSize;

    /* Calc length */
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL, daemon,daemon_local,
                           msg.headerbuffer, sizeof(DltStorageHeader),
                           msg.headerbuffer + sizeof(DltStorageHeader),
                           (int) (msg.headersize - sizeof(DltStorageHeader)),
                           msg.databuffer, (int) msg.datasize, verbose);

    free(msg.databuffer);

    return DLT_RETURN_OK;
}

/* Returns DLT style timestamp of journal entry, 0.1 ms resolution */
static long unsigned int dlt_sdjournal_get_timestamp_field(DltSdJournal *sdj) {
    uint64_t timestampUsec64 = 0;
    int result = sd_journal_get_monotonic_usec(sdj->handle, &timestampUsec64, NULL);
    if (result < 0) {
        dlt_vlog(LOG_ERR, "%s: sd_journal_get_monotonic_usec() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        return 0;
    }

    return (long unsigned int)(timestampUsec64 / 100);
}

static const char* dlt_sdjournal_get_string_field_raw(DltSdJournal *sdj, const char* const fieldName, size_t* resultLen, int verbose) {
    const size_t fieldNameLen = strlen(fieldName);
    const char* value = NULL;
    size_t length = 0;
    int result = sd_journal_get_data(sdj->handle, fieldName, (void*)&value, &length);
    if (result < 0 || !value || length < (fieldNameLen + 1)) {
        if (verbose) {
            dlt_vlog(LOG_ERR, "%s: sd_journal_get_data() for '%s' failed: result=%d, errno=%d, value=%p, length=%llu\n",
                __func__, fieldName, result, errno, (const void*)value, (long long unsigned int)length);
        }

        *resultLen = 0;
        return "";
    }

    *resultLen = (length - fieldNameLen - 1);
    return (value + fieldNameLen + 1);
}

static char* dlt_sdjournal_get_string_field(DltSdJournal *sdj, const char* const fieldName, int verbose) {
    size_t length = 0;
    const char* const str = dlt_sdjournal_get_string_field_raw(sdj, fieldName, &length, verbose);
    return strndup(str, length);
}

static long unsigned int dlt_sdjournal_get_luint_field(DltSdJournal *sdj, const char* const fieldName, int verbose) {
    long unsigned int result = 0;

    size_t length = 0;
    const char* const str = dlt_sdjournal_get_string_field_raw(sdj, fieldName, &length, verbose);
    if (length > 0 && str[length] == '\0') {
        result = atol(str);
    }

    return result;
}


static char* string_join(const size_t argCount, ...) {
    va_list arglist;

    size_t resultSize = 0;
    va_start(arglist, argCount);
    for (size_t argIdx = 0; argIdx < argCount; ++argIdx) {
        const char* const argStr = va_arg(arglist, const char*);
        const size_t argStrLen = (argStr != NULL) ? strlen(argStr) : 0;

        resultSize += argStrLen;

        ++resultSize; // for delimiter
    }
    va_end(arglist);

    if (resultSize == 0) {
        return strdup("");
    }

    char* const result = malloc(resultSize);
    if (!result) {
        return NULL;
    }

    char* resultItr = result;
    va_start(arglist, argCount);
    for (size_t argIdx = 0; argIdx < argCount; ++argIdx) {
        const char* const argStr = va_arg(arglist, const char*);
        const size_t argStrLen = (argStr != NULL) ? strlen(argStr) : 0;

        memcpy(resultItr, argStr, argStrLen);
        resultItr += argStrLen;

        *resultItr = ' ';
        ++resultItr;
    }
    va_end(arglist);

    --resultItr;
    *resultItr = '\0';

    return result;
}

static int dlt_sdjournal_process_message(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltSdJournal *sdj, int verbose) {
    const long unsigned int timestamp = dlt_sdjournal_get_timestamp_field(sdj);
    const long unsigned int pid = dlt_sdjournal_get_luint_field(sdj, "_PID", verbose);

    const long unsigned int severity = dlt_sdjournal_get_luint_field(sdj, "PRIORITY", verbose);
    const long unsigned int dltLevel = syslog_severity_to_dlt_level(severity);

    const long unsigned int facility = dlt_sdjournal_get_luint_field(sdj, "SYSLOG_FACILITY", verbose);
    const char* const dltContext = syslog_facility_to_dlt_context(facility);

    const char* const transport = dlt_sdjournal_get_string_field(sdj, "_TRANSPORT", verbose);
    const char* const id = dlt_sdjournal_get_string_field(sdj, "SYSLOG_IDENTIFIER", verbose);
    const char* const message = dlt_sdjournal_get_string_field(sdj, "MESSAGE", verbose);

    const char* formattedMessage = NULL;
    char dltId[DLT_ID_SIZE] = {0};

    if (strcmp(transport, "kernel") == 0) {
        memcpy(dltId, "KERN", DLT_ID_SIZE);

        if (facility == LOG_KERN) {
            // SYSLOG_IDENTIFIER field is "kernel" in this case
            formattedMessage = string_join(2, "kernel", message);
        } else {
            // Journal attempts to parse a colon delimited id from kernel messages
            // emitted from user space. Id is stored in the SYSLOG_IDENTIFIER field
            // and rest of each line is stored in the MESSAGE field. Glue both
            // fields together to see entire output.
            formattedMessage = string_join(3, "kernel", id, message);
        }
    } else {
        syslog_id_to_dlt_app_id(dltId, id);

        formattedMessage = string_join(2, id, message);
    }

    int result = DLT_RETURN_OK;

    int maxAllowedLevel = dlt_sdjournal_chmap_get_max_level(sdj->chmap, dltId);
    if (maxAllowedLevel == DLT_LOG_DEFAULT) {
        maxAllowedLevel = sdj->defaultLogLevel;
    }

    if (dltLevel <= maxAllowedLevel) {
        result = dlt_sdjournal_send_message(
            daemon, daemon_local,
            timestamp, pid,
            dltId, dltContext, dltLevel,
            sdj->logEntryCount, formattedMessage,
            verbose
        );
    }

    free(transport);
    free(id);
    free(message);
    free(formattedMessage);

    return result;
}

static int dlt_sdjournal_read(DltDaemon *daemon, DltDaemonLocal *daemon_local, DltSdJournal *sdj, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    unsigned int errorCount = 0;
    for(;;) {
        int result = sd_journal_next(sdj->handle);
        if (result < 0) {
            ++errorCount;
            dlt_vlog(LOG_ERR, "%s: sd_journal_next() failed: result=%d, errno=%d\n",
                __func__, result, errno);
            break;
        } else if (result == 0) {
            break;
        }

        ++sdj->logEntryCount;

        result = dlt_sdjournal_process_message(daemon, daemon_local, sdj, verbose);
        if (result != DLT_RETURN_OK) {
            ++errorCount;
            dlt_vlog(LOG_DEBUG, "%s: failed to read message %u: result=%d\n",
                __func__, sdj->logEntryCount, result);
        }
    }

    dlt_vlog(LOG_DEBUG, "%s: logEntryCount=%u, errorCount=%u\n",
        __func__, sdj->logEntryCount, errorCount);

    if (errorCount) {
        return DLT_RETURN_ERROR;
    } else {
        return DLT_RETURN_OK;
    }
}

int dlt_sdjournal_process(DltDaemon *daemon,
                          DltDaemonLocal *daemon_local,
                          DltReceiver *receiver,
                          int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (!daemon || !daemon_local || !receiver) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    DltSdJournal *sdj = &daemon_local->pSdJournal;
    if (!sdj->handle) {
        dlt_vlog(LOG_ERR, "%s: Systemd Journal handle not initialized\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    int result = sd_journal_process(sdj->handle);
    if (result < 0) {
        dlt_vlog(LOG_ERR, "%s: sd_journal_process() failed: result=%d, errno=%d\n",
            __func__, result, errno);
        dlt_sdjournal_deinit(daemon_local, verbose);
        return DLT_RETURN_ERROR;
    }

    // https://www.freedesktop.org/software/systemd/man/sd_journal_process.html
    // Programs only interested in a strictly sequential stream of log data may treat
    // SD_JOURNAL_INVALIDATE the same way as SD_JOURNAL_APPEND, thus ignoring any changes
    // to the log view earlier than the old end of the log stream.
    if (result == SD_JOURNAL_APPEND || result == SD_JOURNAL_INVALIDATE) {
        return dlt_sdjournal_read(daemon, daemon_local, sdj, verbose);
    } else {
        return DLT_RETURN_OK;
    }
}

DltReceiver* dlt_sdjournal_get_receiver(DltDaemonLocal *daemon_local)
{
    if (!daemon_local) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters\n", __func__);
        return NULL;
    }

    DltSdJournal *sdj = &daemon_local->pSdJournal;
    if (!sdj->handle) {
        dlt_vlog(LOG_ERR, "%s: Systemd Journal handle not initialized\n", __func__);
        return NULL;
    }

    return &sdj->receiver;
}

static int dlt_level_to_syslog_severity(uint8_t dltLevel)
{
    switch (dltLevel) {
        case DLT_LOG_OFF:
            // no "off" config in syslog conventions
            // crit is close enough
            return LOG_CRIT;
        case DLT_LOG_FATAL:
            return LOG_CRIT;
        case DLT_LOG_ERROR:
            return LOG_ERR;
        case DLT_LOG_WARN:
            return LOG_WARNING;
        case DLT_LOG_INFO:
            return LOG_INFO;
        case DLT_LOG_DEBUG:
            return LOG_DEBUG;
        case DLT_LOG_VERBOSE:
            return LOG_DEBUG;
        default:
            return LOG_INFO;
    };
}

int dlt_sdjournal_change_app_level(DltDaemonLocal* daemon_local, const char* apid, const char* ctid, uint8_t level)
{
    dlt_vlog(LOG_INFO, "%s: apid='%.4s', ctid='%.4s', level=%hhu\n",
        __func__, apid, ctid, level);

    if (*ctid != '\0' && strncmp(ctid, "*", DLT_ID_SIZE) != 0) {
        dlt_vlog(LOG_ERR, "%s: only ctid='*' is presently supported to change log level of entire app\n",
            __func__);
        return DLT_RETURN_ERROR;
    }

    if (!daemon_local) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltSdJournal *sdj = &daemon_local->pSdJournal;
    if (!sdj->handle) {
        dlt_vlog(LOG_ERR, "%s: Systemd Journal handle not initialized\n", __func__);
        return DLT_RETURN_ERROR;
    }

    if (level == (uint8_t)DLT_LOG_DEFAULT) {
        return dlt_sdjournal_chmap_unset_max_level(sdj->chmap, apid);
    } else if (level >= (uint8_t)DLT_LOG_OFF && level < (uint8_t)DLT_LOG_MAX) {
        return dlt_sdjournal_chmap_set_max_level(sdj->chmap, apid, (int)level);
    } else {
        dlt_vlog(LOG_ERR, "%s: invalid level\n", __func__);
        return DLT_RETURN_ERROR;
    }
}

int dlt_sdjournal_change_default_level(DltDaemonLocal* daemon_local, uint8_t level)
{
    dlt_vlog(LOG_INFO, "%s: level=%hhu\n",
        __func__, level);

    if (!daemon_local) {
        dlt_vlog(LOG_ERR, "%s: invalid parameters\n", __func__);
        return DLT_RETURN_ERROR;
    }

    DltSdJournal *sdj = &daemon_local->pSdJournal;
    if (!sdj->handle) {
        dlt_vlog(LOG_ERR, "%s: Systemd Journal handle not initialized\n", __func__);
        return DLT_RETURN_ERROR;
    }

    if (level == (uint8_t)DLT_LOG_DEFAULT) {
        sdj->defaultLogLevel = DLT_LOG_VERBOSE;
    } else if (level >= (uint8_t)DLT_LOG_OFF && level < (uint8_t)DLT_LOG_MAX) {
        sdj->defaultLogLevel = (int)level;
    } else {
        dlt_vlog(LOG_ERR, "%s: invalid level\n", __func__);
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

#else /* NOT defined(DLT_SYSTEMD_JOURNAL_ENABLE) */

int dlt_sdjournal_init(DltDaemonLocal *daemon_local, int verbose)
{
    dlt_vlog(LOG_ERR, "%s: Systemd Journal support not enabled\n", __func__);
    return DLT_RETURN_ERROR;
}

void dlt_sdjournal_deinit(DltDaemonLocal *daemon_local, int verbose)
{
}

int dlt_sdjournal_process(DltDaemon *daemon,
                          DltDaemonLocal *daemon_local,
                          DltReceiver *receiver,
                          int verbose)
{
    return DLT_RETURN_ERROR;
}

DltReceiver* dlt_sdjournal_get_receiver(DltDaemonLocal *daemon_local)
{
    return NULL;
}

#endif /* defined(DLT_SYSTEMD_JOURNAL_ENABLE) */
