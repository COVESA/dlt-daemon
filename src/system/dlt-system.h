/*
 * Dlt system manager to Dlt
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
**  SRC-MODULE: dlt-system.h                                                  **
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

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

#ifndef DLT_SYSTEM_H
#define DLT_SYSTEM_H

#define DLT_SYSTEM_MODE_OFF 0
#define DLT_SYSTEM_MODE_STARTUP 1
#define DLT_SYSTEM_MODE_REGULAR 2

#define DLT_SYSTEM_LOG_FILE_MAX 32
#define DLT_SYSTEM_LOG_PROCESSES_MAX 32

typedef struct {
	char ConfigurationFile[256];
	char ApplicationId[256];
	int  daemonise;

	int  SyslogEnable;
	char SyslogContextId[256];
	int  SyslogPort;
	
	int  FiletransferEnable;
	char FiletransferDirectory1[256];
	char FiletransferDirectory2[256];
	char FiletransferContextId[256];
	int  FiletransferTimeStartup;
	int  FiletransferTimeDelay;

	int  LogFileEnable;
	int  LogFileNumber;
	char LogFileFilename[DLT_SYSTEM_LOG_FILE_MAX][256];
	int  LogFileMode[DLT_SYSTEM_LOG_FILE_MAX];
	int  LogFileTimeDelay[DLT_SYSTEM_LOG_FILE_MAX];
	char LogFileContextId[DLT_SYSTEM_LOG_FILE_MAX][256];

	int  LogProcessesEnable;
	char LogProcessesContextId[256];
	int  LogProcessNumber;
	char LogProcessName[DLT_SYSTEM_LOG_PROCESSES_MAX][256];
	char LogProcessFilename[DLT_SYSTEM_LOG_PROCESSES_MAX][256];
	int  LogProcessMode[DLT_SYSTEM_LOG_PROCESSES_MAX];
	int  LogProcessTimeDelay[DLT_SYSTEM_LOG_PROCESSES_MAX];
} DltSystemOptions;

typedef struct {
	int  timeStartup;	/* time in seconds since startup of dlt-system */
	int  timeFiletransferDelay;	/* time in seconds to start next filetransfer */
	char filetransferFile[256];
	int  timeLogFileDelay[DLT_SYSTEM_LOG_FILE_MAX];	/* time in seconds to start next file log */
	int  timeLogProcessDelay[DLT_SYSTEM_LOG_PROCESSES_MAX];	/* time in seconds to start next process log */
	int	 filetransferRunning; 	/* 0 = stooped, 1 = running */
	int  filetransferCountPackages; /* number of packets to be transfered */
	int  filetransferLastSentPackage; /* last packet sent starting from 1 */
} DltSystemRuntime;

#endif /* DLT_SYSTEM_H */
