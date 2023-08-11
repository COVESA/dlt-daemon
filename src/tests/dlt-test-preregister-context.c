/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015  Intel Corporation
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
 * \author Stefan Vacek <stefan.vacek@intel.com> Intel Corporation
 *
 * \copyright Copyright © 2015 Intel Corporation. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-preregister-context.c
 */

#include <unistd.h> /* for fork() */

#include "dlt.h"
#include "dlt_user_macros.h"

/**
 * @brief sample code for using pre-registered contexts
 */
int main()
{
    DltContext mainContext;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 200000 * 1000;

    DLT_REGISTER_CONTEXT(mainContext, "CTXP", "main context");

    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message before app registered"));
    nanosleep(&ts, NULL);

    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Second message before app registered"));
    nanosleep(&ts, NULL);

    DLT_REGISTER_APP("PRNT", "Sample pre-register application");

    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message after app registered"));
    nanosleep(&ts, NULL);

    DLT_UNREGISTER_APP()
    ;

    return 0;
}
