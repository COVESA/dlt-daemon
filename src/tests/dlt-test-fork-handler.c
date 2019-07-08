/*
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
#include <time.h>
#include <errno.h>

#include "dlt.h"

/**
 * @brief sample code for using at_fork-handler
 */
int main()
{
    DltContext mainContext;
    struct timespec timeout, r;

    timeout.tv_sec  = 0;
    timeout.tv_nsec = 200000000L;

    DLT_REGISTER_APP("PRNT", "Parent application");
    DLT_REGISTER_CONTEXT(mainContext, "CTXP", "Parent context");
    DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message before fork"));
    nanosleep(&timeout, &r);

    pid_t pid = fork();
    if (pid == 0) { /* child process */
        /* this message should not be visible */
        DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Child's first message after fork, pid: "), DLT_INT32(getpid()));

        /* this will not register CHLD application */
        DLT_REGISTER_APP("CHLD", "Child application");
        /* this will not register CTXC context */
        DLT_REGISTER_CONTEXT(mainContext, "CTXC", "Child context");
        /* this will not log a message */
        DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Child's second message after fork, pid: "), DLT_INT32(getpid()));
        nanosleep(&timeout, &r);
        if (execlp("dlt-example-user", "dlt-example-user", "-n 1",
                   "you should see this message", NULL))
            return errno;
    }
    else if (pid == -1) /* error in fork */
    {
        return -1;
    }
    else { /* parent */
        DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Parent's first message after fork, pid: "), DLT_INT32(getpid()));
        nanosleep(&timeout, &r);
    }

    DLT_UNREGISTER_APP()
    ;

    return 0;
}
