/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2016 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author
 * Christoph Lipka <clipka@jp.adit-jv.com>
 *
 * \copyright Copyright © 2016 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_protocol.c
 */

#include "dlt_protocol.h"

const char *const dlt_service_names[] = {
    "DLT_SERVICE_ID",
    "DLT_SERVICE_ID_SET_LOG_LEVEL",
    "DLT_SERVICE_ID_SET_TRACE_STATUS",
    "DLT_SERVICE_ID_GET_LOG_INFO",
    "DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL",
    "DLT_SERVICE_ID_STORE_CONFIG",
    "DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT",
    "DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS",
    "DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH",
    "DLT_SERVICE_ID_SET_VERBOSE_MODE",
    "DLT_SERVICE_ID_SET_MESSAGE_FILTERING",
    "DLT_SERVICE_ID_SET_TIMING_PACKETS",
    "DLT_SERVICE_ID_GET_LOCAL_TIME",
    "DLT_SERVICE_ID_USE_ECU_ID",
    "DLT_SERVICE_ID_USE_SESSION_ID",
    "DLT_SERVICE_ID_USE_TIMESTAMP",
    "DLT_SERVICE_ID_USE_EXTENDED_HEADER",
    "DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL",
    "DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS",
    "DLT_SERVICE_ID_GET_SOFTWARE_VERSION",
    "DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW"
};
const char *const dlt_user_service_names[] = {
    "DLT_USER_SERVICE_ID",
    "DLT_SERVICE_ID_UNREGISTER_CONTEXT",
    "DLT_SERVICE_ID_CONNECTION_INFO",
    "DLT_SERVICE_ID_TIMEZONE",
    "DLT_SERVICE_ID_MARKER",
    "DLT_SERVICE_ID_OFFLINE_LOGSTORAGE",
    "DLT_SERVICE_ID_PASSIVE_NODE_CONNECT",
    "DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS",
    "DLT_SERVICE_ID_SET_ALL_LOG_LEVEL",
    "DLT_SERVICE_ID_SET_ALL_TRACE_STATUS",
    "DLT_SERVICE_ID_UNDEFINED", /* 0xF0A is not defined */
    "DLT_SERVICE_ID_RESERVED",
    "DLT_SERVICE_ID_RESERVED",
    "DLT_SERVICE_ID_RESERVED",
    "DLT_SERVICE_ID_RESERVED"
};

const char *dlt_get_service_name(unsigned int id)
{
    if (id == DLT_SERVICE_ID_CALLSW_CINJECTION)
        return "DLT_SERVICE_ID_CALLSW_CINJECTION";
    else if ((id == DLT_SERVICE_ID) || (id >= DLT_USER_SERVICE_ID_LAST_ENTRY) ||
             ((id >= DLT_SERVICE_ID_LAST_ENTRY) && (id <= DLT_USER_SERVICE_ID)))
        return "UNDEFINED";
    else if ((id > DLT_SERVICE_ID) && (id < DLT_SERVICE_ID_LAST_ENTRY))
        return dlt_service_names[id];
    else /* user services */
        return dlt_user_service_names[id & 0xFF];
}
