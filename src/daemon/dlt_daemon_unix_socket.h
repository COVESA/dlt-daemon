/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015, Advanced Driver Information Technology
 * Copyright of Advanced Driver Information Technology, Bosch and Denso.
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
 * \author Christoph Lipka <clipka@jp.adit-jv.com>
 *
 * \copyright Copyright © 2015 ADIT. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_unix_socket.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_unix_socket.h                                      **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
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
**  cl          Christoph Lipka            ADIT                               **
*******************************************************************************/

#ifndef DLT_DAEMON_UNIX_SOCKET_H
#define DLT_DAEMON_UNIX_SOCKET_H

#ifdef ANDROID
DltReturnValue dlt_daemon_unix_android_get_socket(int *sock, const char *sock_path);
#endif
int dlt_daemon_unix_socket_open(int *sock, char *socket_path, int type, int mask);
int dlt_daemon_unix_socket_close(int sock);

#endif /* DLT_DAEMON_UNIX_SOCKET_H */
