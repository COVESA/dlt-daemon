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
 * \file dlt_offline_trace.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_trace.h                                           **
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

#ifndef DLT_OFFLINE_TRACE_H
#define DLT_OFFLINE_TRACE_H

#include <limits.h>

#include "dlt_types.h"

#define DLT_OFFLINETRACE_FILENAME_BASE "dlt_offlinetrace"
#define DLT_OFFLINETRACE_FILENAME_DELI "_"
#define DLT_OFFLINETRACE_FILENAME_EXT  ".dlt"
#define DLT_OFFLINETRACE_INDEX_MAX_SIZE 10
#define DLT_OFFLINETRACE_FILENAME_TO_COMPARE "dlt_offlinetrace_"
/* "dlt_offlinetrace.4294967295.dlt" -> MAX 32byte include NULL terminate */
#define DLT_OFFLINETRACE_FILENAME_MAX_SIZE   (sizeof(DLT_OFFLINETRACE_FILENAME_BASE) + \
                                              sizeof(DLT_OFFLINETRACE_FILENAME_DELI) + \
                                              DLT_OFFLINETRACE_INDEX_MAX_SIZE + \
                                              sizeof(DLT_OFFLINETRACE_FILENAME_EXT) + 1)

typedef struct
{
    char directory[NAME_MAX + 1];/**< (String) Store DLT messages to local directory */
    char filename[NAME_MAX + 1]; /**< (String) Filename of currently used log file */
    int fileSize;                /**< (int) Maximum size in bytes of one trace file (Default: 1000000) */
    int maxSize;                 /**< (int) Maximum size of all trace files (Default: 4000000) */
    int filenameTimestampBased;  /**< (int) timestamp based or index based (Default: 1 Timestamp based) */
    int ohandle;
} DltOfflineTrace;

/**
 * Initialise the offline trace
 * This function call opens the currently used log file.
 * A check of the complete size of the offline trace is done during startup.
 * Old files are deleted, if there is not enough space left to create new file.
 * This function must be called before using further offline trace functions.
 * @param trace pointer to offline trace structure
 * @param directory directory where to store offline trace files
 * @param fileSize maximum size of one offline trace file.
 * @param maxSize maximum size of complete offline trace in bytes.
 *.@param filenameTimestampBased filename to be created on timestamp based or index based
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_offline_trace_init(DltOfflineTrace *trace,
                                             const char *directory,
                                             int fileSize,
                                             int maxSize,
                                             int filenameTimestampBased);

/**
 * Uninitialise the offline trace
 * This function call closes currently used log file.
 * This function must be called after usage of offline trace
 * @param buf pointer to offline trace structure
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_offline_trace_free(DltOfflineTrace *buf);

/**
 * Write data into offline trace
 * If the current used log file exceeds the max file size, new log file is created.
 * A check of the complete size of the offline trace is done before new file is created.
 * Old files are deleted, if there is not enough space left to create new file.
 * @param trace pointer to offline trace structure
 * @param data1 pointer to first data block to be written, null if not used
 * @param size1 size in bytes of first data block to be written, 0 if not used
 * @param data2 pointer to second data block to be written, null if not used
 * @param size2 size in bytes of second data block to be written, 0 if not used
 * @param data3 pointer to third data block to be written, null if not used
 * @param size3 size in bytes of third data block to be written, 0 if not used
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_offline_trace_write(DltOfflineTrace *trace,
                                              unsigned char *data1,
                                              int size1,
                                              unsigned char *data2,
                                              int size2,
                                              unsigned char *data3,
                                              int size3);

/**
 * Get size of currently used offline trace buffer
 * @return size in bytes
 */
extern unsigned long dlt_offline_trace_get_total_size(DltOfflineTrace *trace);

/**
 * Provides info about the offline logs storage directory
 * @param path path of the storage directory
 * @param file_name filename to search for
 * @param newest pointer to store newest filename
 * @param oldest pointer to store oldest filename
 * @return num of files in the directory
 */
unsigned int dlt_offline_trace_storage_dir_info(char *path, char *file_name, char *newest, char *oldest);

/**
 * creates filename with index
 * @param log_file_name file name created with index
 * @param name filename base
 * @param idx index to be used for file name creation
 */
void dlt_offline_trace_file_name(char *log_file_name, char *name, unsigned int idx);

/**
 * generates index for log file name
 * @param file filename supplied to create index
 * @return the index to be used for log file name
 */
unsigned int dlt_offline_trace_get_idx_of_log_file(char *file);


#endif /* DLT_OFFLINE_TRACE_H */
