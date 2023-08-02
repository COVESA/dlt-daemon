/**
 * Copyright (C) 2018 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT offline log storage functionality internal header file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Aditya Paluri <venkataaditya.paluri@in.bosch.com> ADIT 2018
 *
 * \file: dlt_daemon_offline_logstorage_internal.h
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
 *                                                                            **
 *  SRC-MODULE: dlt_daemon_offline_logstorage_internal.h                      **
 *                                                                            **
 *  TARGET    : linux                                                         **
 *                                                                            **
 *  PROJECT   : DLT                                                           **
 *                                                                            **
 *  AUTHOR    : Aditya Paluri venkataaditya.paluri@in.bosch.com               **
 *  PURPOSE   :                                                               **
 *                                                                            **
 *  REMARKS   :                                                               **
 *                                                                            **
 *  PLATFORM DEPENDANT [yes/no]: yes                                          **
 *                                                                            **
 *  TO BE CHANGED BY USER [yes/no]: no                                        **
 *                                                                            **
 ******************************************************************************/

/*******************************************************************************
*                      Author Identity                                       **
*******************************************************************************
*                                                                            **
* Initials     Name                       Company                            **
* --------     -------------------------  ---------------------------------- **
*  ap          Aditya Paluri              ADIT                               **
*******************************************************************************/

#ifndef DLT_DAEMON_OFFLINE_LOGSTORAGE_INTERNAL_H
#define DLT_DAEMON_OFFLINE_LOGSTORAGE_INTERNAL_H

DLT_STATIC DltReturnValue dlt_logstorage_split_key(char *key,
                                                   char *apid,
                                                   char *ctid,
                                                   char *ecuid);

DltReturnValue dlt_logstorage_update_all_contexts(DltDaemon *daemon,
                                                  DltDaemonLocal *daemon_local,
                                                  char *id,
                                                  int curr_log_level,
                                                  int cmp_flag,
                                                  char *ecuid,
                                                  int verbose);

DltReturnValue dlt_logstorage_update_context(DltDaemon *daemon,
                                             DltDaemonLocal *daemon_local,
                                             char *apid,
                                             char *ctid,
                                             char *ecuid,
                                             int curr_log_level,
                                             int verbose);

DltReturnValue dlt_logstorage_update_context_loglevel(DltDaemon *daemon,
                                                      DltDaemonLocal *daemon_local,
                                                      char *key,
                                                      int curr_log_level,
                                                      int verbose);

#endif
