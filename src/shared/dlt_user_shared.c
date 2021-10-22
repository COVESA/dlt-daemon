/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_user_shared.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_shared.c                                             **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/uio.h> /* writev() */

#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"

#ifdef __QNX__
#   include <stdio.h>
#   include <stdlib.h>
#   include <unistd.h>
#   include <string.h>
#   include <time.h>
#   include <stdint.h>
#   include <sys/neutrino.h>
#   include <sys/iofunc.h>
#   define RECV_BUFLEN 255
#   define MSGSEND_TIMEOUT 100  // Timeout for MsgSend() by default
    int timeoutMs = MSGSEND_TIMEOUT;
    static DltReturnValue SendMessage(int fd, iov_t *iov, int iovcnt);
#endif /* __QNX__ */

DltReturnValue dlt_user_set_userheader(DltUserHeader *userheader, uint32_t mtype)
{
    if (userheader == 0)
        return DLT_RETURN_ERROR;

    if (mtype <= 0)
        return DLT_RETURN_ERROR;

    userheader->pattern[0] = 'D';
    userheader->pattern[1] = 'U';
    userheader->pattern[2] = 'H';
    userheader->pattern[3] = 1;
    userheader->message = mtype;

    return DLT_RETURN_OK;
}

int dlt_user_check_userheader(DltUserHeader *userheader)
{
    if (userheader == 0)
        return -1;

    return (userheader->pattern[0] == 'D') &&
           (userheader->pattern[1] == 'U') &&
           (userheader->pattern[2] == 'H') &&
           (userheader->pattern[3] == 1);
}

DltReturnValue dlt_user_log_out2(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2)
{
    struct iovec iov[2];
    uint32_t bytes_written;

    if (handle < 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    iov[0].iov_base = ptr1;
    iov[0].iov_len = len1;
    iov[1].iov_base = ptr2;
    iov[1].iov_len = len2;

    bytes_written = (uint32_t) writev(handle, iov, 2);

    if (bytes_written != (len1 + len2))
        return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_out3(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3)
{
    struct iovec iov[3];
    uint32_t bytes_written;

    if (handle < 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    iov[0].iov_base = ptr1;
    iov[0].iov_len = len1;
    iov[1].iov_base = ptr2;
    iov[1].iov_len = len2;
    iov[2].iov_base = ptr3;
    iov[2].iov_len = len3;

    bytes_written = (uint32_t) writev(handle, iov, 3);

    if (bytes_written != (len1 + len2 + len3)) {
        switch (errno) {
        case ETIMEDOUT:
        {
            return DLT_RETURN_PIPE_ERROR;     /* ETIMEDOUT - connect timeout */
            break;
        }
        case EBADF:
        {
            return DLT_RETURN_PIPE_ERROR;     /* EBADF - handle not open */
            break;
        }
        case EPIPE:
        {
            return DLT_RETURN_PIPE_ERROR;     /* EPIPE - pipe error */
            break;
        }
        case EAGAIN:
        {
            return DLT_RETURN_PIPE_FULL;     /* EAGAIN - data could not be written */
            break;
        }
        default:
        {
            break;
        }
        }

        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

#ifdef DLT_DAEMON_USE_QNX_MESSAGE_IPC
DltReturnValue dlt_user_log_out2_qnx_msg(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2)
{
    if (handle < 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    io_write_t whdr = {.i.type = 2, .i.nbytes = (len1 + len2)};
    iov_t msg_iov[3];

    // set up the IOV to point to both parts:
    SETIOV (msg_iov + 0, &whdr, sizeof (whdr));
    SETIOV (msg_iov + 1, ptr1, len1);
    SETIOV (msg_iov + 2, ptr2, len2);

    return SendMessage(handle, msg_iov, 3);
}

DltReturnValue dlt_user_log_out3_qnx_msg(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3)
{
    if (handle < 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    io_write_t whdr = {.i.type = 3, .i.nbytes = (len1 + len2 + len3)};
    iov_t msg_iov[4];

    // set up the IOV to point to both parts:
    SETIOV (msg_iov + 0, &whdr, sizeof (whdr));
    SETIOV (msg_iov + 1, ptr1, len1);
    SETIOV (msg_iov + 2, ptr2, len2);
    SETIOV (msg_iov + 3, ptr3, len3);

    return SendMessage(handle, msg_iov, 4);
}

DltReturnValue dlt_attach_channel(name_attach_t **attach)
{
    DltReturnValue rc = DLT_RETURN_OK;
    char filename[DLT_PATH_MAX];
    char* env_test_unit = getenv("DLT_MESSAGING_SEND_TIMEOUT");

    snprintf(filename, DLT_PATH_MAX,  "dltapp%d", getpid());
    *attach = name_attach( NULL, filename, 0 );

    if (*attach == NULL || (*attach)->chid == -1) {
        fprintf(stderr, "server: ERROR: Couldn't create a channel errno %d (%s))\n", errno, strerror(errno));
        return DLT_RETURN_ERROR;
    }

    // Set timeout
    if (env_test_unit != NULL) {
        timeoutMs = (uint32_t)strtol(env_test_unit, NULL, 10);
    }

    return rc;
}

static DltReturnValue SendMessage(int fd, iov_t *iov, int iovcnt)
{
    DltReturnValue rc = DLT_RETURN_ERROR;
    iov_t recv_iov[1];
    char recv_msg[RECV_BUFLEN];
    int rc_MS = -1;

    SETIOV (recv_iov, recv_msg, RECV_BUFLEN);

    if (fd != -1) {
        if (-1 != timeoutMs) {
            struct sigevent event;
            uint64_t timeout;

            SIGEV_UNBLOCK_INIT(&event);
            timeout = (uint64_t)timeoutMs * (uint64_t)(1000 * 1000);
            (void)TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY, &event, &timeout, NULL);
        }

        /* This function will block until the message is received.  Ensure that it
        * is not a problem for the app to be blocked, or that the server always
        * replies soon enough to avoid problems, or both. */
        rc_MS = MsgSendv(fd, iov, iovcnt, recv_iov, 1);
    }

    if(rc_MS == -1) {
        fprintf(stdout, "SendMessage: MsgSendv returned an , errno=%d (%s)!\n", errno, strerror(errno));
        rc = DLT_RETURN_ERROR;
    } else {
        rc = DLT_RETURN_OK;
    }

    return rc;
}
#endif /* DLT_DAEMON_USE_QNX_MESSAGE_IPC */
