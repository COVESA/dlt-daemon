/**
 * Copyright (C) 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * DLT offline log storage functionality header file.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file: dlt_offline_logstorage_behavior.h
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_logstorage_behavior.h                             **
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


#ifndef DLT_OFFLINELOGSTORAGE_DLT_OFFLINE_LOGSTORAGE_BEHAVIOR_H_
#define DLT_OFFLINELOGSTORAGE_DLT_OFFLINE_LOGSTORAGE_BEHAVIOR_H_

/* ON_MSG behavior */
int dlt_logstorage_prepare_on_msg(DltLogStorageFilterConfig *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int log_msg_size,
                                  DltNewestFileName *newest_file_info);
int dlt_logstorage_write_on_msg(DltLogStorageFilterConfig *config,
                                DltLogStorageUserConfig *file_config,
                                char *dev_path,
                                unsigned char *data1,
                                int size1,
                                unsigned char *data2,
                                int size2,
                                unsigned char *data3,
                                int size3);

/* status is strategy, e.g. DLT_LOGSTORAGE_SYNC_ON_MSG is used when callback
 * is called on message received */
int dlt_logstorage_sync_on_msg(DltLogStorageFilterConfig *config,
                               DltLogStorageUserConfig *file_config,
                               char *dev_path,
                               int status);

/* Logstorage cache functionality */
int dlt_logstorage_prepare_msg_cache(DltLogStorageFilterConfig *config,
                                     DltLogStorageUserConfig *file_config,
                                     char *dev_path,
                                     int log_msg_size,
                                     DltNewestFileName *newest_file_info);

int dlt_logstorage_write_msg_cache(DltLogStorageFilterConfig *config,
                                   DltLogStorageUserConfig *file_config,
                                   char *dev_path,
                                   unsigned char *data1,
                                   int size1,
                                   unsigned char *data2,
                                   int size2,
                                   unsigned char *data3,
                                   int size3);

int dlt_logstorage_sync_msg_cache(DltLogStorageFilterConfig *config,
                                  DltLogStorageUserConfig *file_config,
                                  char *dev_path,
                                  int status);

#endif /* DLT_OFFLINELOGSTORAGE_DLT_OFFLINE_LOGSTORAGE_BEHAVIOR_H_ */
