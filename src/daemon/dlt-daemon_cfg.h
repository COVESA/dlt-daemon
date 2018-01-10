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
 * \file dlt-daemon_cfg.h
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-daemon-cfg.h                                              **
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
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#ifndef DLT_DAEMON_CFG_H
#define DLT_DAEMON_CFG_H

/*************/
/* Changable */
/*************/

/* Stack size of timing packet thread */
#define DLT_DAEMON_TIMINGPACKET_THREAD_STACKSIZE 100000

/* Stack size of ecu version thread */
#define DLT_DAEMON_ECU_VERSION_THREAD_STACKSIZE 100000

/* Size of receive buffer for fifo connection  (from user application) */
#define DLT_DAEMON_RCVBUFSIZE       10024
/* Size of receive buffer for socket connection (from dlt client) */
#define DLT_DAEMON_RCVBUFSIZESOCK   10024
/* Size of receive buffer for serial connection (from dlt client) */
#define DLT_DAEMON_RCVBUFSIZESERIAL 10024

/* Size of buffer for text output */
#define DLT_DAEMON_TEXTSIZE         10024

/* Size of buffer */
#define DLT_DAEMON_TEXTBUFSIZE        512

/* Maximum length of a description */
#define DLT_DAEMON_DESCSIZE           256

/* Name of daemon lock file, contain process id of dlt daemon instance */
#define DLT_DAEMON_LOCK_FILE  "dltd.lock"

/* Umask of daemon, creates files with permission 750 */
#define DLT_DAEMON_UMASK              027
/* Permissions of daemon lock file */
#define DLT_DAEMON_LOCK_FILE_PERM    0640

/* Default ECU ID, used in storage header and transmitted to client*/
#define DLT_DAEMON_ECU_ID "ECU1"

/* Default baudrate for serial interface */
#define DLT_DAEMON_SERIAL_DEFAULT_BAUDRATE 115200

/************************/
/* Don't change please! */
/************************/

#endif /* DLT_DAEMON_CFG_H */

