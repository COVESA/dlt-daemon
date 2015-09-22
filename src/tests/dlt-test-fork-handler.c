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
 * \file dlt-test-fork-handler.c
*/

#include <unistd.h> /* for fork() */

#include "dlt.h"

/**
 * @brief sample code for using at_fork-handler
 */
int main()
{
    DltContext mainContext;

    DLT_REGISTER_APP("PRNT", "Parent application");
    DLT_REGISTER_CONTEXT(mainContext, "CTXP", "Parent context");
    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message before fork"));
    usleep(200000);

    pid_t pid = fork();
    if (pid == 0) /* child process */
    {
        /* this message should not be visible */
        /* DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Child's first message after fork, pid: "), DLT_INT32(getpid())); */
        /* unfortunately, this message does arrive, I assume because it still has (locally) valid data ... */

        DLT_REGISTER_APP("CHLD", "Child application");
        DLT_REGISTER_CONTEXT(mainContext, "CTXC", "Child context");
        DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Child's second message after fork, pid: "), DLT_INT32(getpid()));
        usleep(400000);
    }
    else if (pid == -1) /* error in fork */
    {
        return -1;
    }
    else /* parent */
    {
        DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Parent's first message after fork, pid: "), DLT_INT32(getpid()));
        usleep(500000);
    }

    DLT_UNREGISTER_APP()
    ;

    return 0;
}
