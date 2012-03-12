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
 * \file dlt-system-log.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-log.c                                              **
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <zlib.h>
#include <sys/inotify.h>

#include "dlt_common.h"
#include "dlt_user.h"
#include "dlt_filetransfer.h"

#include "dlt-system.h"
#include "dlt-system_cfg.h"
#include "dlt-system-log.h"

/* Size of the compression buffer */
#define Z_CHUNK_SZ 1024*128

#define INOTIFY_SZ (sizeof(struct inotify_event))
#define INOTIFY_LEN (INOTIFY_SZ + 1024)

/* Check if the file name ends in .z */
int dlt_system_is_gz_file(char *file_name)
{
	  size_t f_len = strlen(file_name);
	  size_t z_len = strlen(".gz");

	  if(z_len > f_len)
		  return 0;

	  if(strncmp( file_name + f_len - z_len, ".gz", z_len ) == 0)
	  {
		  return 1;
	  }
	  return 0;
}
/* File compression routine.
 * Side effects:
 * - modifies src to point to the compressed file.
 * - removes the original file
 */
int dlt_system_compress_file(char *src_name, int level)
{
	char dst_name[256];
	char dst_mode[8];
	char *buf;

	gzFile dst_file;
	FILE *src_file;

	/* Initialize output */
	sprintf(dst_name, "%s.gz", src_name);
	sprintf(dst_mode, "wb%d", level);
	dst_file = gzopen(dst_name, dst_mode);

	/* Initialize input */
	src_file = fopen(src_name, "r");

	if(!src_file || !dst_file)
	{
		gzclose(dst_file);
		fclose(src_file);
		return -1;
	}

	/* Initialize buffer */
	buf = malloc(Z_CHUNK_SZ);

	/* Read from the src and write to dst */
	while(!feof(src_file))
	{
		int read = fread(buf, 1, Z_CHUNK_SZ, src_file);
		if(ferror(src_file))
		{
			free(buf);
			gzclose(dst_file);
			fclose(src_file);
			return -1;
		}
		gzwrite(dst_file, buf, read);
	}

	/* Clean up */
	free(buf);
	gzclose(dst_file);
	fclose(src_file);

	/* Remove source file */
	remove(src_name);

	/* Rename in name buffer */
	strcpy(src_name, dst_name);

	return 0;
}

int dlt_system_filetransfer_init(DltSystemOptions *options,DltSystemRuntime *runtime)
{
	runtime->filetransferFile[0] = 0;
	runtime->filetransferRunning = 0;
	runtime->filetransferCountPackages = 0;

	// Initialize watch for filetransfer directories.
	dlt_system_inotify_handle = inotify_init1(IN_NONBLOCK);
	if(dlt_system_inotify_handle < 0) {
		return -1;
	}

	// We can discard the descriptors.
	// They will be closed automatically on program exit.
	if(inotify_add_watch(dlt_system_inotify_handle, options->FiletransferDirectory1, IN_CLOSE_WRITE|IN_MOVED_TO) < 0)
		return -1;
	if(inotify_add_watch(dlt_system_inotify_handle, options->FiletransferDirectory2, IN_CLOSE_WRITE|IN_MOVED_TO) < 0)
		return -1;
	return 0;
}

void dlt_system_filetransfer_run(DltSystemOptions *options,DltSystemRuntime *runtime,DltContext *context)
{
	char filename[256];
	struct stat status;
	int transferResult;
	int total_size, used_size;
	static char inotify_buf[INOTIFY_LEN];
	static char file_stack[256][256];
	static int file_stack_ptr = -1;

	if(runtime->filetransferRunning == 0) {
		/* delete last transmitted file */
		if(runtime->filetransferFile[0]!=0) {
			if(stat(runtime->filetransferFile,&status)==0) 
			{
				if(runtime->filetransferFilesize == status.st_size)
				{
					/* delete file only if size is not changed since starting transfer */
					printf("Remove File: %s\n",runtime->filetransferFile);
					if(remove(runtime->filetransferFile)) {
						printf("Remove file %s failed!\n",runtime->filetransferFile);
					}					
				}
			}
			runtime->filetransferFile[0]=0; 
		}

		/* Check inotify watches */
		int len = read(dlt_system_inotify_handle, inotify_buf, INOTIFY_LEN);
		if(len < 0)
		{
			if(errno != EWOULDBLOCK) {
				fprintf(stderr, "dlt_system_filetransfer_run:\n%s\n", strerror(errno));
				return;
			}
		}

		int i = 0;
		while(i < len)
		{
			struct inotify_event *event = (struct inotify_event *) &inotify_buf[i];
			printf("inotify: %s \n", event->name);
			if(event->mask | IN_CLOSE_WRITE && event->mask | IN_MOVED_TO)
			{
				file_stack_ptr++;
				strcpy(file_stack[file_stack_ptr], event->name);
			}
			i += INOTIFY_SZ + event->len;
		}

		/* filetransfer not running, check directory */
		filename[0] = 0;
		dir = opendir(options->FiletransferDirectory1);
		if(dir > 0) {
			while ((dp=readdir(dir)) != NULL) {
				if(strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) {
					sprintf(filename,"%s/%s",options->FiletransferDirectory1,dp->d_name);
					if(stat(filename,&status)==0)
					{
						if((time_oldest == 0 || status.st_mtime < time_oldest) && (status.st_size != 0) && !dlt_system_is_gz_file(filename)) {
							time_oldest = status.st_mtime;
							strcpy(runtime->filetransferFile,filename);
							runtime->filetransferFilesize = status.st_size;

							/* Compress the file if required */
							if(options->FiletransferCompression1 > 0)
							{
								printf("Start compression: %s\n",runtime->filetransferFile);
								if(dlt_system_compress_file(runtime->filetransferFile, options->FiletransferCompressionLevel) < 0)
									return;
								stat(runtime->filetransferFile,&status);
								runtime->filetransferFilesize = status.st_size;
							}
						}
					}
				}
			}	
			closedir(dir);
		}
		dir = opendir(options->FiletransferDirectory2);
		if(dir > 0) {
			while ((dp=readdir(dir)) != NULL) {
				if(strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) {
					sprintf(filename,"%s/%s",options->FiletransferDirectory2,dp->d_name);
					if(stat(filename,&status)==0)
					{
						if((time_oldest == 0 || status.st_mtime < time_oldest) && (status.st_size != 0) && !dlt_system_is_gz_file(filename)) {
							time_oldest = status.st_mtime;
							strcpy(runtime->filetransferFile,filename);
							runtime->filetransferFilesize = status.st_size;

							/* Compress the file if required */
							if(options->FiletransferCompression2 > 0)
							{
								printf("Start compression: %s\n",runtime->filetransferFile);
								if(dlt_system_compress_file(runtime->filetransferFile, options->FiletransferCompressionLevel) < 0)
									return;
								stat(runtime->filetransferFile,&status);
								runtime->filetransferFilesize = status.st_size;
							}
						}
					}
=======
		if(file_stack_ptr > -1)
		{
			sprintf(filename, "%s/%s", options->FiletransferDirectory1, file_stack[file_stack_ptr]);
			if(stat(filename,&status)==0)
			{
				strcpy(runtime->filetransferFile,filename);
				runtime->filetransferFilesize = status.st_size;
			}
			else
			{
				sprintf(filename, "%s/%s", options->FiletransferDirectory2, file_stack[file_stack_ptr]);
				if(stat(filename,&status)==0)
				{
					strcpy(runtime->filetransferFile,filename);
					runtime->filetransferFilesize = status.st_size;
>>>>>>> 917575d... First working version of inotify for file transfer.
				}
			}
			memset(file_stack[file_stack_ptr--], 0, 256);
		}

		/* start filetransfer if file exists */
		if(runtime->filetransferFile[0]) {
			printf("Start Filetransfer: %s\n",runtime->filetransferFile);
			runtime->filetransferCountPackages = dlt_user_log_file_packagesCount(context,runtime->filetransferFile);
			if(runtime->filetransferCountPackages  < 0 )
			{
					/* a problem occured; stop filetransfer and continue with next file after timeout */
					printf("Error: dlt_user_log_file_packagesCount\n");
					runtime->filetransferCountPackages = 0;
					runtime->filetransferRunning = 0;
					runtime->timeFiletransferDelay = options->FiletransferTimeDelay;
					return;
			}			
			runtime->filetransferRunning = 1;
			transferResult = dlt_user_log_file_header(context,runtime->filetransferFile);
			if(transferResult < 0)
			{
				/* a problem occured; stop filetransfer and continue with next file after timeout */
				printf("Error: dlt_user_log_file_header\n");
				runtime->filetransferCountPackages = 0;
				runtime->filetransferRunning = 0;
				runtime->timeFiletransferDelay = options->FiletransferTimeDelay;

				return;
			}
			runtime->filetransferLastSentPackage = 0;		
		}

	}
	
	if (runtime->filetransferRunning == 1) {
		/* filetransfer is running, send next data */
		while(runtime->filetransferLastSentPackage<runtime->filetransferCountPackages) {
			/* wait sending next package if more than 50% of buffer used */
			dlt_user_check_buffer(&total_size, &used_size);
			if((total_size - used_size) < (total_size/2))
			{
				break;
			}

			runtime->filetransferLastSentPackage++;
			transferResult = dlt_user_log_file_data(context,runtime->filetransferFile,runtime->filetransferLastSentPackage,options->FiletransferTimeoutBetweenLogs);
			if(transferResult < 0)
			{
				/* a problem occured; stop filetransfer and continue with next file after timeout */
				printf("Error: dlt_user_log_file_data\n");
				runtime->filetransferCountPackages = 0;
				runtime->filetransferRunning = 0;
				runtime->timeFiletransferDelay = options->FiletransferTimeDelay;
				return;
			}			

		}
		if(runtime->filetransferLastSentPackage==runtime->filetransferCountPackages) {
			transferResult = dlt_user_log_file_end(context,runtime->filetransferFile,0);
			if(transferResult < 0)
			{
				printf("Error: dlt_user_log_file_end\n");
				runtime->filetransferCountPackages = 0;
				runtime->filetransferRunning = 0;
				runtime->timeFiletransferDelay = options->FiletransferTimeDelay;
				return;
			}
			runtime->timeFiletransferDelay = options->FiletransferTimeDelay;
			runtime->filetransferRunning = 0;
		}					
	}	
}

void dlt_system_log_file(DltSystemOptions *options,DltContext *context,int num) {
	FILE * pFile;
	char buffer[1024];
	int bytes;
	int seq = 1;

	pFile = fopen(options->LogFileFilename[num],"r");
	
	if(pFile>0)
	{
		while (!feof(pFile)) {
			bytes = fread(buffer,1,sizeof(buffer)-1,pFile);
			if(bytes>=0)
				buffer[bytes] = 0;
			else
				buffer[0] = 0;
			
			if(feof(pFile)) {
				DLT_LOG(*context, DLT_LOG_INFO, DLT_INT(seq*-1), DLT_STRING(buffer));				   	
			}
			else {
				DLT_LOG(*context, DLT_LOG_INFO, DLT_INT(seq++), DLT_STRING(buffer));				   	
			}
		} 
		fclose(pFile);
	}
}

void dlt_system_log_process(DltSystemOptions *options,DltContext *context,int num) {
	FILE * pFile;
	struct dirent *dp;
	char filename[256];
	char buffer[1024];
	int bytes;
	int found = 0;

	/* go through all dlt files in directory */
	DIR *dir = opendir("/proc");
	if(dir>0) {
		while ((dp=readdir(dir)) != NULL) {
			if(dp->d_name[0]>'0' && dp->d_name[0]<'9') {		
				buffer[0] = 0;	
				sprintf(filename,"/proc/%s/cmdline",dp->d_name);
				pFile = fopen(filename,"r");
				if(pFile>0)
				{
					bytes = fread(buffer,1,sizeof(buffer)-1,pFile);
					fclose(pFile);
				}
				if((strcmp(options->LogProcessName[num],"*")==0) || 
				  (strcmp(buffer,options->LogProcessName[num])==0) ) {
					found = 1;
					sprintf(filename,"/proc/%s/%s",dp->d_name,options->LogProcessFilename[num]);
					pFile = fopen(filename,"r");
					if(pFile>0)
					{
						bytes = fread(buffer,1,sizeof(buffer)-1,pFile);
						fclose(pFile);
					
						if(bytes>0) { 
							buffer[bytes] = 0;
							DLT_LOG(*context, DLT_LOG_INFO, DLT_INT(atoi(dp->d_name)),DLT_STRING(options->LogProcessFilename[num]), DLT_STRING(buffer));				   	
						}
					}
					if(strcmp(options->LogProcessName[num],"*")!=0)
						break;
				}
			}
		}	
		closedir(dir);	
	}
	
	if(!found) {
			DLT_LOG(*context, DLT_LOG_INFO, DLT_STRING("Process"), DLT_STRING(options->LogProcessName[num]),DLT_STRING("not running!"));				   	
	}
}
