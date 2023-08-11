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
 * \file dlt-test-fork-handler.c
 */

#include <unistd.h> /* for fork() */
#include <time.h>
#include <errno.h>

#include "dlt.h"

void dlt_log_message(DltContext *context, DltLogLevelType ll, char *text, int32_t num)
{
    DltContextData contextData;

    if (text == NULL)
        return;

    if (dlt_user_log_write_start(context, &contextData, ll) > 0) {
        dlt_user_log_write_string(&contextData, text);
        if (num > 0) {
            dlt_user_log_write_int32(&contextData, num);
        }
        dlt_user_log_write_finish(&contextData);
    }

    return;
}

/**
 * @brief sample code for using at_fork-handler
 */
int main()
{
    DltContext mainContext;
    struct timespec timeout, r;

    timeout.tv_sec  = 0;
    timeout.tv_nsec = 200000000L;

    dlt_register_app("PRNT", "Parent application");
    dlt_register_context(&mainContext, "CTXP", "Parent context");
    dlt_log_message(&mainContext, DLT_LOG_WARN, "First message before fork", 0);
    nanosleep(&timeout, &r);

    pid_t pid = fork();
    if (pid == 0) { /* child process */
        /* this message should not be visible */
        dlt_log_message(&mainContext, DLT_LOG_WARN, "Child's first message after fork, pid: ", getpid());

        /* this will not register CHLD application */
        dlt_register_app("CHLD", "Child application");
        /* this will not register CTXC context */
        dlt_register_context(&mainContext, "CTXC", "Child context");
        /* this will not log a message */
        dlt_log_message(&mainContext, DLT_LOG_WARN, "Child's second message after fork, pid: ", getpid());
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
        dlt_log_message(&mainContext, DLT_LOG_WARN, "Parent's first message after fork, pid: ", getpid());
        nanosleep(&timeout, &r);
    }

    dlt_unregister_app();

    return 0;
}
