/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt-system.h
 * For further information see http://www.genivi.org/.
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

	int  SyslogEnable;					/*# Enable the Syslog Adapter (Default: 0)*/
	char SyslogContextId[256];			/*# The Context Id of the syslog adapter (Default: SYSL)*/
	int  SyslogPort;					/*# The UDP port opened by DLT system mamager to receive system logs (Default: 47111)*/
	
	int  FiletransferEnable;			/*# Enable the Filetransfer (Default: 0)*/
	char FiletransferDirectory1[256];	/*# Directory which contains files to be transfered over DLT (Default: /tmp/filetransfer)# Files are deleted after Filetransfer is finished and after TimeDelay expired*/
	char FiletransferDirectory2[256];
	char FiletransferContextId[256];	/*# The Context Id of the filetransfer (Default: FILE)*/
	int  FiletransferTimeStartup;		/*# Time after startup of dlt-system when first file is transfered (Default: 30)# Time in seconds*/
	int  FiletransferTimeDelay;			/*# Time to wait when transfered file is deleted and next file transfer starts (Default: 10)# Time in seconds*/
	int  FiletransferTimeoutBetweenLogs;/*# Waits a period of time between two file transfer logs of a single file to DLT to ensure that the FIFO of DLT is not flooded.*/

	/*# Log different files
	# Mode: 0 = off, 1 = startup only, 2 = regular
	# TimeDelay: If mode regular is set, time delay is the number of seconds for next sent
	*/
	int  LogFileEnable;					/*# Enable the logging of files (Default: 0)*/
	int  LogFileNumber;
	char LogFileFilename[DLT_SYSTEM_LOG_FILE_MAX][256];
	int  LogFileMode[DLT_SYSTEM_LOG_FILE_MAX];
	int  LogFileTimeDelay[DLT_SYSTEM_LOG_FILE_MAX];
	char LogFileContextId[DLT_SYSTEM_LOG_FILE_MAX][256];

	int  LogProcessesEnable;			/*# Enable the logging of processes (Default: 0)*/
	char LogProcessesContextId[256];	/*# The Context Id of the kernel version (Default: PROC)*/

	/*# Log different processes
	# Name: * = all process, X=alternative name (must correspind to /proc/X/cmdline
	# Filename: the filename in the subdirectory /proc/processid/
	# Mode: 0 = off, 1 = startup only, 2 = regular
	# TimeDelay: If mode regular is set, time delay is the number of seconds for next sent
	*/
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
	long int filetransferFilesize;
	int  timeLogFileDelay[DLT_SYSTEM_LOG_FILE_MAX];	/* time in seconds to start next file log */
	int  timeLogProcessDelay[DLT_SYSTEM_LOG_PROCESSES_MAX];	/* time in seconds to start next process log */
	int	 filetransferRunning; 	/* 0 = stooped, 1 = running */
	int  filetransferCountPackages; /* number of packets to be transfered */
	int  filetransferLastSentPackage; /* last packet sent starting from 1 */
} DltSystemRuntime;

#endif /* DLT_SYSTEM_H */
