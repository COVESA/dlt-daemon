/*
 * Dlt- Diagnostic Log and Trace daemon
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
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

