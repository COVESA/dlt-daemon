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
 * \file dlt_daemon_socket.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_socket.h                                           **
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

#ifndef DLT_DAEMON_SOCKET_H
#define DLT_DAEMON_SOCKET_H

#include <limits.h>
#include <semaphore.h>
#include "dlt_common.h"
#include "dlt_user.h"

int dlt_daemon_socket_open(int *sock, unsigned int servPort, char *ip);
int dlt_daemon_socket_close(int sock);

int dlt_daemon_socket_get_send_qeue_max_size(int sock);

int dlt_daemon_socket_send(int sock,
                           void *data1,
                           int size1,
                           void *data2,
                           int size2,
                           char serialheader);

/**
 * @brief dlt_daemon_socket_sendreliable - sends data to socket with additional checks and resending functionality - trying to be reliable
 * @param sock
 * @param data_buffer
 * @param message_size
 * @return on sucess: DLT_DAEMON_ERROR_OK, on error: DLT_DAEMON_ERROR_SEND_FAILED
 */
int dlt_daemon_socket_sendreliable(int sock, void *data_buffer, int message_size);

#endif /* DLT_DAEMON_SOCKET_H */
