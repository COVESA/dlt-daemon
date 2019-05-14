/*
 * @licence app begin@
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
 * @licence end@
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
#include <sys/ioctl.h> /* ioctl() */
#include <string.h> /* strerror() */
#include <syslog.h>

#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"

static DltReturnValue dlt_writev_reliable(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3);

DltReturnValue dlt_writev_reliable(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3)
{
    struct iovec iov[3];
    uint32_t bytes_written;
    int daemon_fifo_size;
    size_t non_read_bytes = 0;

    if (handle <= 0)
        /* Invalid handle */
        return DLT_RETURN_ERROR;

    iov[0].iov_base = ptr1;
    iov[0].iov_len = len1;
    iov[1].iov_base = ptr2;
    iov[1].iov_len = len2;
    iov[2].iov_base = ptr3;
    iov[2].iov_len = len3;

    if (dlt_is_file_check != 1)
    {
        /* Get Daemon FIFO size */
        if ((daemon_fifo_size = fcntl(handle, F_GETPIPE_SZ , 0)) == -1)
        {
            dlt_vlog(LOG_ERR, "get FIFO size error, could not perform write operation: %s\n",strerror(errno));
            return DLT_RETURN_ERROR;
        }
        if ((daemon_fifo_size > 0) && (ioctl(handle, FIONREAD, &non_read_bytes) != -1))
        {
            // Determine the FIFO size availability prior writing data to PIPE
            if ((daemon_fifo_size - non_read_bytes) > (len1+len2+len3))
            {
                bytes_written = writev(handle, iov, 3);
            }
            else
            {
                dlt_vlog(LOG_DEBUG, "Not enough space available to write the data to FIFO, hence storing it in ring buffer\n");
                return DLT_RETURN_ERROR; // Not enough size to write data to FIFO
            }
        }
        else
            return DLT_RETURN_ERROR;
    }
    else
    {
        // writing data to a File
        bytes_written = writev(handle, iov, 3);
    }
    if (bytes_written != (len1 + len2 + len3))
        return DLT_RETURN_ERROR;
    return DLT_RETURN_OK;
}

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
    return dlt_writev_reliable(handle, ptr1, len1, ptr2,  len2, 0, 0);
}

DltReturnValue dlt_user_log_out3(int handle, void *ptr1, size_t len1, void *ptr2, size_t len2, void *ptr3, size_t len3)
{
    DltReturnValue ret;
    ret = dlt_writev_reliable(handle, ptr1, len1, ptr2, len2, ptr3, len3);
    if (ret < DLT_RETURN_OK)
	{
        switch (errno) {
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

    return ret;
}
