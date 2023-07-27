/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
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
#include <sys/time.h> /* timeval */

#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"

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

DltReturnValue dlt_user_log_out2_with_timeout(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2)
{
    if (handle < 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(handle, &fds);

    struct timeval tv = { DLT_WRITEV_TIMEOUT_SEC, DLT_WRITEV_TIMEOUT_USEC };
    if (select(handle+1, NULL, &fds, NULL, &tv) < 0) {
        return DLT_RETURN_ERROR;
    }

    if (FD_ISSET(handle, &fds)) {
        return dlt_user_log_out2(handle, ptr1, len1, ptr2, len2);
    } else {
        return DLT_RETURN_ERROR;
    }
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

DltReturnValue dlt_user_log_out3_with_timeout(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3)
{
    if (handle < 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(handle, &fds);

    struct timeval tv = { DLT_WRITEV_TIMEOUT_SEC, DLT_WRITEV_TIMEOUT_USEC };
    if (select(handle+1, NULL, &fds, NULL, &tv) < 0) {
        return DLT_RETURN_ERROR;
    }

    if (FD_ISSET(handle, &fds)) {
        return dlt_user_log_out3(handle, ptr1, len1, ptr2, len2, ptr3, len3);
    } else {
        return DLT_RETURN_ERROR;
    }
}
