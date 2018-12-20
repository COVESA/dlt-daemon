/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015  Intel Corporation
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
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

/**
 * @brief sample code for using pre-registered contexts
 */
int main()
{
    DltContext mainContext;
    DLT_REGISTER_CONTEXT(mainContext, "CTXP", "main context");

    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message before app registered"));
    usleep(200000);

    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Second message before app registered"));
    usleep(200000);

    DLT_REGISTER_APP("PRNT", "Sample pre-register application");

    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message after app registered"));
    usleep(200000);

    DLT_UNREGISTER_APP()
    ;

    return 0;
}
