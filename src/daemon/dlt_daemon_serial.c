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
 * \file dlt_daemon_serial.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_serial.c                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h> /* send() */

#include "dlt-daemon.h"

#include "dlt_types.h"

#include "dlt_daemon_serial.h"

int dlt_daemon_serial_send(int sock,
                           void *data1,
                           int size1,
                           void *data2,
                           int size2,
                           char serialheader)
{
    /* Optional: Send serial header, if requested */
    if (serialheader) {
        if (0 > write(sock, dltSerialHeader, sizeof(dltSerialHeader))) {
            return DLT_DAEMON_ERROR_SEND_FAILED;
        }
    }

    /* Send data */

    if (data1 && (size1 > 0)) {
        if (0 > write(sock, data1, size1)) {
            return DLT_DAEMON_ERROR_SEND_FAILED;
        }
    }

    if (data2 && (size2 > 0)) {
        if (0 > write(sock, data2, size2)) {
            return DLT_DAEMON_ERROR_SEND_FAILED;
        }
    }

    return DLT_DAEMON_ERROR_OK;
}
