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
 * \file dlt_offline_trace.c
 * For further information see http://www.genivi.org/.
 * @licence end@
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
   
#include <dlt_offline_trace.h>

int dlt_offline_trace_create_new_file(DltOfflineTrace *trace) {
    time_t t;
    struct tm *tmp;
    char outstr[200];

	/* set filename */
    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
    }
    if (strftime(outstr, sizeof(outstr),"%Y%m%d_%H%M%S", tmp) == 0) {
    }
	sprintf(trace->filename,"%s/dlt_offlinetrace_%s.dlt",trace->directory,outstr);

    /* open DLT output file */
	trace->ohandle = open(trace->filename,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */
	if (trace->ohandle == -1)
	{
		/* trace file cannot be opened */
		printf("Offline trace file %s cannot be created\n",trace->filename);
		return -1;
	} /* if */

	return 0; /* OK */	
}

unsigned long dlt_offline_trace_get_total_size(DltOfflineTrace *trace) {
	struct dirent *dp;
	char filename[256];
	unsigned long size = 0;
	struct stat status;

	/* go through all dlt files in directory */
	DIR *dir = opendir(trace->directory);
	while ((dp=readdir(dir)) != NULL) {
		if(strstr(dp->d_name,".dlt")) {
			sprintf(filename,"%s/%s",trace->directory,dp->d_name);
			stat(filename,&status);
			size += status.st_size;
		}
	}	
	closedir(dir);
	
	/* return size */
	return size; 
}

int dlt_offline_trace_delete_oldest_file(DltOfflineTrace *trace) {
	struct dirent *dp;
	char filename[256];
	char filename_oldest[256];
	unsigned long size_oldest = 0;
	struct stat status;
	time_t time_oldest = 0;

	filename[0] = 0;
	filename_oldest[0] = 0;

	/* go through all dlt files in directory */
	DIR *dir = opendir(trace->directory);
	while ((dp=readdir(dir)) != NULL) {
		if(strstr(dp->d_name,".dlt")) {
			sprintf(filename,"%s/%s",trace->directory,dp->d_name);
			stat(filename,&status);
			if(time_oldest == 0 || status.st_mtime < time_oldest) {
				time_oldest = status.st_mtime;
				size_oldest = status.st_size;
				strcpy(filename_oldest,filename);
			}
		}
	}	
	closedir(dir);
	
	/* delete file */
	if(filename_oldest[0]) {
		if(remove(filename_oldest)) {
			printf("Remove file %s failed!\n",filename_oldest);
			return -1; /* ERROR */
		}
	}
	else {
			printf("No file to be removed!\n");
			return -1; /* ERROR */
	}
	
	/* return size of deleted file*/
	return size_oldest; 
}

int dlt_offline_trace_check_size(DltOfflineTrace *trace) {
	
	/* check size of complete offline trace */
	while((int)dlt_offline_trace_get_total_size(trace) > (trace->maxSize-trace->fileSize))
	{
		/* remove oldest files as long as new file will not fit in completely into complete offline trace */
		if(dlt_offline_trace_delete_oldest_file(trace)<0) {
			return -1;
		}
	}
	
	return 0; /* OK */	
}
	
int dlt_offline_trace_init(DltOfflineTrace *trace,const char *directory,int fileSize,int maxSize) {

	/* init parameters */
	strcpy(trace->directory,directory);
	trace->fileSize = fileSize;
	trace->maxSize = maxSize;

	/* check complete offlien trace size, remove old logs if needed */
	dlt_offline_trace_check_size(trace);

	return dlt_offline_trace_create_new_file(trace);
}

int dlt_offline_trace_write(DltOfflineTrace *trace,unsigned char *data1,int size1,unsigned char *data2,int size2,unsigned char *data3,int size3) {

	if(trace->ohandle <= 0)
		return -1;

	/* check file size here */
	if((lseek(trace->ohandle,0,SEEK_CUR)+size1+size2+size3)>=trace->fileSize)
	{
		/* close old file */
		close(trace->ohandle);
		
		/* check complete offline trace size, remove old logs if needed */
		dlt_offline_trace_check_size(trace);
		
		/* create new file */
		dlt_offline_trace_create_new_file(trace);
	}
	
	/* write data into log file */
	if(data1) {
		if(write(trace->ohandle,data1,size1)!=size1) {
			printf("Offline trace write failed!\n");
			return -1;			
		}
	}
	if(data2) {
		if(write(trace->ohandle,data2,size2)!=size2) {
			printf("Offline trace write failed!\n");
			return -1;						
		}
	}
	if(data3) {
		if(write(trace->ohandle,data3,size3)!=size3) {
			printf("Offline trace write failed!\n");
			return -1;						
		}
	}

	return 0; /* OK */
}

int dlt_offline_trace_free(DltOfflineTrace *trace) {

	if(trace->ohandle <= 0)
		return -1;
	
	/* close last used log file */
	close(trace->ohandle);

	return 0; /* OK */
}
