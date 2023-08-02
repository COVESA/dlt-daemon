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
 * \file dlt_offline_trace.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_trace.c                                           **
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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <syslog.h>
#include <errno.h>

#include <dlt_offline_trace.h>
#include <dlt_multiple_files.h>

DltReturnValue dlt_offline_trace_write(MultipleFilesRingBuffer *trace,
                                       const unsigned char *data1,
                                       const int size1,
                                       const unsigned char *data2,
                                       const int size2,
                                       const unsigned char *data3,
                                       const int size3)
{

    if (trace->ohandle < 0) return DLT_RETURN_ERROR;

    multiple_files_buffer_rotate_file(trace, size1 + size2 + size3);

    /* write data into log file */
    if (multiple_files_buffer_write_chunk(trace, data1, size1) != DLT_RETURN_OK) return DLT_RETURN_ERROR;
    if (multiple_files_buffer_write_chunk(trace, data2, size2) != DLT_RETURN_OK) return DLT_RETURN_ERROR;
    if (multiple_files_buffer_write_chunk(trace, data3, size3) != DLT_RETURN_OK) return DLT_RETURN_ERROR;

    return DLT_RETURN_OK;
}
