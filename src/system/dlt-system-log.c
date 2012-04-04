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

	if(!dst_file)
	{
		return -1;
	}
	/* Initialize input */
	src_file = fopen(src_name, "r");

	if(!src_file)
	{
		gzclose(dst_file);
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

int dlt_system_inotify_handle;
int dlt_system_watch_descriptor_dir1;
int dlt_system_watch_descriptor_dir2;
#define INOTIFY_SZ (sizeof(struct inotify_event))
#define INOTIFY_LEN (INOTIFY_SZ + 256)
#define MAX_FILE_QUEUE 256

int dlt_system_filetransfer_init(DltSystemOptions *options,DltSystemRuntime *runtime)
{
	runtime->filetransferFile[0] = 0;
	runtime->filetransferRunning = 0;
	runtime->filetransferCountPackages = 0;

	// Initialize watch for filetransfer directories.
	dlt_system_inotify_handle = inotify_init1(IN_NONBLOCK);
	if(dlt_system_inotify_handle < 0)
	{
		return -1;
	}

	dlt_system_watch_descriptor_dir1 =  inotify_add_watch(dlt_system_inotify_handle, options->FiletransferDirectory1, IN_CLOSE_WRITE|IN_MOVED_TO);
	dlt_system_watch_descriptor_dir2 =  inotify_add_watch(dlt_system_inotify_handle, options->FiletransferDirectory2, IN_CLOSE_WRITE|IN_MOVED_TO);
	if(dlt_system_watch_descriptor_dir1 < 0 || dlt_system_watch_descriptor_dir2 < 0)
	{
		return -1;
	}
	return 0;
}

void dlt_system_filetransfer_run(DltSystemOptions *options,DltSystemRuntime *runtime,DltContext *context)
{
	struct stat status;
	int transferResult;
	int total_size, used_size;
	static char inotify_buf[INOTIFY_LEN];
	static char *file_stack1[MAX_FILE_QUEUE];
	static char *file_stack2[MAX_FILE_QUEUE];
	static int file_stack_ptr1 = -1;
	static int file_stack_ptr2 = -1;
	static int first_run = 1;

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

		/* Check inotify watch, add new files to stack */
		if(file_stack_ptr1 < (MAX_FILE_QUEUE/2) - 4 && file_stack_ptr2 < (MAX_FILE_QUEUE/2) - 4)
		{
			int len = read(dlt_system_inotify_handle, inotify_buf, INOTIFY_LEN);
			if(len < 0)
			{
				if(errno != EWOULDBLOCK)
				{
					fprintf(stderr, "dlt_system_filetransfer_run error reading inotify handle. %d\n", errno);
					return;
				}
			}
			int i = 0;
			while(i < len)
			{
				struct inotify_event *ievent = (struct inotify_event *)&inotify_buf[i];
				if(ievent->len > 0)
				{
					if((ievent->mask & IN_CLOSE_WRITE || ievent->mask & IN_MOVED_TO) &&
							!dlt_system_is_gz_file(ievent->name))
					{
						if(ievent->wd == dlt_system_watch_descriptor_dir1)
						{
							int plen = strlen(options->FiletransferDirectory1);
							plen += ievent->len + 1;
							file_stack_ptr1++;
							file_stack1[file_stack_ptr1] = malloc(plen);
							sprintf(file_stack1[file_stack_ptr1], "%s/%s",
									options->FiletransferDirectory1,
									ievent->name);
						}
						else if(ievent->wd == dlt_system_watch_descriptor_dir2)
						{
							int plen = strlen(options->FiletransferDirectory2);
							plen += ievent->len + 1;
							file_stack_ptr2++;
							file_stack2[file_stack_ptr2] = malloc(plen);
							sprintf(file_stack2[file_stack_ptr2], "%s/%s",
									options->FiletransferDirectory2,
									ievent->name);
						}
						else
						{
							fprintf(stderr, "Unknown inotify descriptor in dlt_system_filetransfer_run.\n");
							return;
						}
					}
				}
				i += INOTIFY_SZ + ievent->len;
			}
		}

		// Check if there were files before inotify was registered.
		// TODO: This is awfully clumsy, review when modularizing
		if(first_run && (file_stack_ptr1 < (MAX_FILE_QUEUE/2) - 4 && file_stack_ptr2 < (MAX_FILE_QUEUE/2) - 4))
		{
			char filename[256];
			struct dirent *dp;
			DIR *dir;
			filename[0] = 0;
			dir = opendir(options->FiletransferDirectory1);
			if(dir > 0) {
				while ((dp=readdir(dir)) != NULL) {
					if(strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) {
						sprintf(filename,"%s/%s",options->FiletransferDirectory1,dp->d_name);
						if(stat(filename,&status)==0)
						{
							int plen = strlen(filename)+1;
							file_stack_ptr1++;
							file_stack1[file_stack_ptr1] = malloc(plen);
							strcpy(file_stack1[file_stack_ptr1], filename);
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
							int plen = strlen(filename)+1;
							file_stack_ptr2++;
							file_stack2[file_stack_ptr2] = malloc(plen);
							strcpy(file_stack2[file_stack_ptr2], filename);
						}
					}
				}
				closedir(dir);
			}
			first_run = 0;
		}

		/* Select file to transfer */
		if(file_stack_ptr1 > -1)
		{
			memset(runtime->filetransferFile, 0, 256);
			/*
			 * FIXME: filetransferfile is constant 256 len buffer
			 * file_stack items are dynamically allocated. This
			 * might overflow, (and will, eventually...)
			 */
			memcpy(runtime->filetransferFile, file_stack1[file_stack_ptr1], strlen(file_stack1[file_stack_ptr1]));
			free(file_stack1[file_stack_ptr1--]);
			if(options->FiletransferCompression1)
			{
				dlt_system_compress_file(runtime->filetransferFile, options->FiletransferCompressionLevel);
			}
			struct stat st;
			stat(runtime->filetransferFile, &st);
			runtime->filetransferFilesize = st.st_size;
		}
		else if(file_stack_ptr2 > -1)
		{
			memset(runtime->filetransferFile, 0, 256);
			memcpy(runtime->filetransferFile, file_stack2[file_stack_ptr2], strlen(file_stack2[file_stack_ptr2]));
			free(file_stack2[file_stack_ptr2--]);
			if(options->FiletransferCompression2)
			{
				dlt_system_compress_file(runtime->filetransferFile, options->FiletransferCompressionLevel);
			}
			struct stat st;
			stat(runtime->filetransferFile, &st);
			runtime->filetransferFilesize = st.st_size;
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
		int iteration_package_count = 0;
		/* filetransfer is running, send next data */
		while(runtime->filetransferLastSentPackage<runtime->filetransferCountPackages) {
			/* wait sending next package if more than 50% of buffer used */
			dlt_user_check_buffer(&total_size, &used_size);
			if((total_size - used_size) < (total_size/2))
			{
				break;
			}

			/* Give a chance for other packets to go through periodically */
			if(iteration_package_count > 100)
			{
				iteration_package_count = 0;
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
			iteration_package_count++;
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
