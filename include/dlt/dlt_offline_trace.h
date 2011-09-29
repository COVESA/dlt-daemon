/*
* Dlt- Diagnostic Log and Trace user library
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

typedef struct
{
    char directory[256]; /**< (String) Store DLT messages to local directory */
    char filename[256]; /**< (String) Filename of currently used log file */
    int  fileSize;	/**< (int) Maximum size in bytes of one trace file (Default: 1000000) */
    int  maxSize;	/**< (int) Maximum size of all trace files (Default: 4000000) */
    
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
 * @return negative value if there was an error
 */
extern int dlt_offline_trace_init(DltOfflineTrace *trace,const char *directory,int fileSize,int maxSize);

/**
 * Uninitialise the offline trace
 * This function call closes currently used log file.
 * This function must be called after usage of offline trace
 * @param trace pointer to offline trace structure
 * @return negative value if there was an error
 */
extern int dlt_offline_trace_free(DltOfflineTrace *buf);

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
extern int dlt_offline_trace_write(DltOfflineTrace *trace,unsigned char *data1,int size1,unsigned char *data2,int size2,unsigned char *data3,int size3);

/**
 * Get size of currently used offline trace buffer
 * @return size in bytes
 */
extern unsigned long dlt_offline_trace_get_total_size(DltOfflineTrace *trace);

#endif /* DLT_OFFLINE_TRACE_H */
