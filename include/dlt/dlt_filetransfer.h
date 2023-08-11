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
 * \file dlt_filetransfer.h
 */

#ifndef DLT_FILETRANSFER_H
#define DLT_FILETRANSFER_H

#include <limits.h>    /* Needed for LONG_MAX */
#include <sys/stat.h>  /* Needed for struct stat st*/
#include "dlt.h"       /* Needed for DLT Logs */
#include <signal.h>    /* Signal handling */
#include "errno.h"


/* ! Error code for dlt_user_log_file_complete */
#define DLT_FILETRANSFER_ERROR_FILE_COMPLETE -300
/* ! Error code for dlt_user_log_file_complete */
#define DLT_FILETRANSFER_ERROR_FILE_COMPLETE1 -301
/* ! Error code for dlt_user_log_file_complete */
#define DLT_FILETRANSFER_ERROR_FILE_COMPLETE2 -302
/* ! Error code for dlt_user_log_file_complete */
#define DLT_FILETRANSFER_ERROR_FILE_COMPLETE3 -303
/* ! Error code for dlt_user_log_file_head */
#define DLT_FILETRANSFER_ERROR_FILE_HEAD -400
/* ! Error code for dlt_user_log_file_data */
#define DLT_FILETRANSFER_ERROR_FILE_DATA -500
/* ! Error code for dlt_user_log_file_data */
#define DLT_FILETRANSFER_ERROR_FILE_DATA_USER_BUFFER_FAILED -501
/* ! Error code for dlt_user_log_file_end */
#define DLT_FILETRANSFER_ERROR_FILE_END -600
/* ! Error code for dlt_user_log_file_infoAbout */
#define DLT_FILETRANSFER_ERROR_INFO_ABOUT -700
/* ! Error code for dlt_user_log_file_packagesCount */
#define DLT_FILETRANSFER_ERROR_PACKAGE_COUNT -800
/* ! Error code for failed get serial number */
#define DLT_FILETRANSFER_FILE_SERIAL_NUMBER -900


/* !Transfer the complete file as several dlt logs. */
/**This method transfer the complete file as several dlt logs. At first it will be checked that the file exist.
 * In the next step some generic informations about the file will be logged to dlt.
 * Now the header will be logged to dlt. See the method dlt_user_log_file_header for more informations.
 * Then the method dlt_user_log_data will be called with the parameter to log all packages in a loop with some timeout.
 * At last dlt_user_log_end is called to signal that the complete file transfer was okey. This is important for the plugin of the dlt viewer.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param deleteFlag Flag if the file will be deleted after transfer. 1->delete, 0->notDelete
 * @param timeout Timeout in ms to wait between some logs. Important that the FIFO of dlt will not be flooded with to many messages in a short period of time.
 * @return Returns 0 if everything was okey. If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_complete(DltContext *fileContext, const char *filename, int deleteFlag, int timeout);


/* !This method gives information about the number of packages the file have */
/**Every file will be divided into several packages. Every package will be logged as a single dlt log.
 * The number of packages depends on the BUFFER_SIZE.
 * At first it will be checked if the file exist. Then the file will be divided into
 * several packages depending on the buffer size.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @return Returns 0 if everything was okey. If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_packagesCount(DltContext *fileContext, const char *filename);


/* !Logs specific file inforamtions to dlt */
/**The filename, file size, file serial number and the number of packages will be logged to dlt.
 * @param fileContext Specific context
 * @param filename Absolute file path
 * @return Returns 0 if everything was okey.If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_infoAbout(DltContext *fileContext, const char *filename);


/* !Transfer the head of the file as a dlt logs. */
/**The head of the file must be logged to dlt because the head contains inforamtion about the file serial number,
 * the file name, the file size, package number the file have and the buffer size.
 * All these informations are needed from the plugin of the dlt viewer.
 * See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param alias Alias for the file. An alternative name to show in the receiving end
 * @return Returns 0 if everything was okey. If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_header_alias(DltContext *fileContext, const char *filename, const char *alias);

/* !Transfer the head of the file as a dlt logs. */
/**The head of the file must be logged to dlt because the head contains inforamtion about the file serial number,
 * the file name, the file size, package number the file have and the buffer size.
 * All these informations are needed from the plugin of the dlt viewer.
 * See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @return Returns 0 if everything was okey. If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_header(DltContext *fileContext, const char *filename);


/* !Transfer the content data of a file. */
/**See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param packageToTransfer Package number to transfer. If this param is LONG_MAX, the whole file will be transferred with a specific timeout
 * @param timeout Timeout to wait between dlt logs. Important because the dlt FIFO should not be flooded. Default is defined by MIN_TIMEOUT. The given timeout in ms can not be smaller than MIN_TIMEOUT.
 * @return Returns 0 if everything was okey. If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_data(DltContext *fileContext, const char *filename, int packageToTransfer, int timeout);



/* !Transfer the end of the file as a dlt logs. */
/**The end of the file must be logged to dlt because the end contains inforamtion about the file serial number.
 * This informations is needed from the plugin of the dlt viewer.
 * See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param deleteFlag Flag to delete the file after the whole file is transferred (logged to dlt).1->delete,0->NotDelete
 * @return Returns 0 if everything was okey. If there was a failure value < 0 will be returned.
 */
extern int dlt_user_log_file_end(DltContext *fileContext, const char *filename, int deleteFlag);

#endif /* DLT_FILETRANSFER_H */
