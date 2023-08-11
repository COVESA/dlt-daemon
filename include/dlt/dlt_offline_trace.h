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

#include "dlt_multiple_files.h"
#include "dlt_types.h"

#define DLT_OFFLINETRACE_FILENAME_BASE "dlt_offlinetrace"
#define DLT_OFFLINETRACE_FILENAME_EXT  ".dlt"

/**
 * Write data into offline traces.
 * If the current used log file exceeds the max file size, new log file is created.
 * A check of the complete size of the offline traces is done before new file is created.
 * Old files are deleted, if there is not enough space left to create new file.
 * @param trace pointer to MultipleFilesRingBuffer struct.
 * @param data1 pointer to first data block to be written, null if not used.
 * @param size1 size in bytes of first data block to be written, 0 if not used.
 * @param data2 pointer to second data block to be written, null if not used.
 * @param size2 size in bytes of second data block to be written, 0 if not used.
 * @param data3 pointer to third data block to be written, null if not used.
 * @param size3 size in bytes of third data block to be written, 0 if not used.
 * @return negative value if there was an error.
 */
extern DltReturnValue dlt_offline_trace_write(MultipleFilesRingBuffer *trace,
                                              const unsigned char *data1,
                                              int size1,
                                              const unsigned char *data2,
                                              int size2,
                                              const unsigned char *data3,
                                              int size3);

#endif /* DLT_OFFLINE_TRACE_H */
