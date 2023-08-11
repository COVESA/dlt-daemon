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
 * \author
 * Oleg Tropmann <oleg.tropmann@daimler.com>
 * Daniel Weber <daniel.w.weber@daimler.com>
 *
 * \copyright Copyright © 2022 Mercedes-Benz AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_multiple_files.h
 */


#ifndef DLT_MULTIPLE_FILES_H
#define DLT_MULTIPLE_FILES_H

#include <limits.h>

#include "dlt_common.h"
#include "dlt_types.h"

#define MULTIPLE_FILES_FILENAME_INDEX_DELIM "."
#define MULTIPLE_FILES_FILENAME_TIMESTAMP_DELIM "_"

/**
 * Represents a ring buffer of multiple files of identical file size.
 * File names differ in timestamp or index (depending on chosen mode).
 * This buffer is used, e.g. for dlt offline traces and the internal dlt logging (dlt.log)
 */
typedef struct
{
    char directory[NAME_MAX + 1];/**< (String) Store DLT messages to local directory */
    char filename[NAME_MAX + 1]; /**< (String) Filename of currently used log file */
    int fileSize;                /**< (int) Maximum size in bytes of one file, e.g. for offline trace 1000000 as default */
    int maxSize;                 /**< (int) Maximum size of all files, e.g. for offline trace 4000000 as default */
    bool filenameTimestampBased;  /**< (bool) is filename timestamp based? false = index based (Default: true) */
    char filenameBase[NAME_MAX + 1];/**< (String) Prefix of file name */
    char filenameExt[NAME_MAX + 1];/**< (String) Extension of file name */
    int ohandle;                 /**< (int) file handle to current output file */
} MultipleFilesRingBuffer;

/**
 * Initialise the multiple files buffer.
 * This function call opens the currently used log file.
 * A check of the complete size of the files is done during startup.
 * Old files are deleted, if there is not enough space left to create new file.
 * This function must be called before using further multiple files functions.
 * @param files_buffer pointer to MultipleFilesRingBuffer struct.
 * @param directory directory where to store multiple files.
 * @param file_size maximum size of one files.
 * @param max_size maximum size of complete multiple files in bytes.
 * @param filename_timestamp_based filename to be created on timestamp-based or index-based.
 * @param append Indicates whether the current log files is used or a new file should be be created
 * @param filename_base Base name.
 * @param filename_ext File extension.
 * @return negative value if there was an error.
 */
extern DltReturnValue multiple_files_buffer_init(MultipleFilesRingBuffer *files_buffer,
                                                 const char *directory,
                                                 int file_size,
                                                 int max_size,
                                                 bool filename_timestamp_based,
                                                 bool append,
                                                 const char *filename_base,
                                                 const char *filename_ext);

/**
 * Uninitialise the multiple files buffer.
 * This function call closes currently used log file.
 * This function must be called after usage of multiple files.
 * @param files_buffer pointer to MultipleFilesRingBuffer struct.
 * @return negative value if there was an error.
*/
extern DltReturnValue multiple_files_buffer_free(const MultipleFilesRingBuffer *files_buffer);

/**
 * Write data into multiple files.
 * If the current used log file exceeds the max file size, new log file is created.
 * A check of the complete size of the multiple files is done before new file is created.
 * Old files are deleted, if there is not enough space left to create new file.
 * @param files_buffer pointer to MultipleFilesRingBuffer struct.
 * @param data pointer to first data block to be written, null if not used.
 * @param size size in bytes of first data block to be written, 0 if not used.
 * @return negative value if there was an error.
 */
extern DltReturnValue multiple_files_buffer_write(MultipleFilesRingBuffer *files_buffer,
                                                  const unsigned char *data,
                                                  int size);

/**
 * First the limits are verified. Then the oldest file is deleted and a new file is created on demand.
 * @param files_buffer pointer to MultipleFilesRingBuffer struct.
 * @param size size in bytes of data that will be written.
 */
void multiple_files_buffer_rotate_file(MultipleFilesRingBuffer *files_buffer,
                                       int size);

/**
 * Writes the given data to current file specified by corresponding file handle.
 * @param files_buffer pointer to MultipleFilesRingBuffer struct.
 * @param data pointer to data block to be written, null if not used.
 * @param size size in bytes of given data block to be written, 0 if not used.
 */
DltReturnValue multiple_files_buffer_write_chunk(const MultipleFilesRingBuffer *files_buffer,
                                                 const unsigned char *data,
                                                 int size);

/**
 * Get size of currently used multiple files buffer.
 * @return size in bytes.
 */
extern ssize_t multiple_files_buffer_get_total_size(const MultipleFilesRingBuffer *files_buffer);

/**
 * Provides info about the multiple files storage directory.
 * @param path path of the storage directory
 * @param file_name filename to search for
 * @param newest pointer to store newest filename
 * @param oldest pointer to store oldest filename
 * @return num of files in the directory.
 */
unsigned int multiple_files_buffer_storage_dir_info(const char *path, const char *file_name,
                                                    char *newest, char *oldest);

/**
 * Creates filename with index.
 * @param files_buffer pointer to MultipleFilesRingBuffer struct.
 * @param length the maximum length of the log_file_name.
 * @param idx index to be used for file name creation.
 */
void multiple_files_buffer_file_name(MultipleFilesRingBuffer *files_buffer, size_t length, unsigned int idx);

/**
 * Generates index for log file name.
 * @param file filename supplied to create index.
 * @return the index to be used for log file name.
 */
unsigned int multiple_files_buffer_get_idx_of_log_file(char *file);

#endif // DLT_MULTIPLE_FILES_H
