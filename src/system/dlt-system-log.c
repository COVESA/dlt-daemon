#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#include "dlt_common.h"
#include "dlt_user.h"
#include "dlt_filetransfer.h"

#include "dlt-system.h"
#include "dlt-system_cfg.h"
#include "dlt-system-log.h"

void dlt_system_filetransfer_init(DltSystemOptions *options,DltSystemRuntime *runtime)
{
	runtime->filetransferFile[0] = 0;
	runtime->filetransferRunning = 0;
	runtime->filetransferCountPackages = 0;
}

void dlt_system_filetransfer_run(DltSystemOptions *options,DltSystemRuntime *runtime,DltContext *context)
{
	struct dirent *dp;
	char filename[256];
	unsigned long size_oldest = 0;
	struct stat status;
	time_t time_oldest = 0;
	int transferResult;
	int total_size, used_size;

	if(runtime->filetransferRunning == 0) {
		/* delete last transmitted file */
		if(runtime->filetransferFile[0]!=0) {
			printf("Remove File: %s\n",runtime->filetransferFile);
			if(remove(runtime->filetransferFile)) {
				printf("Remove file %s failed!\n",runtime->filetransferFile);
				return; 
			}
			runtime->filetransferFile[0]=0; 
		}

		/* filetransfer not running, check directory */
		filename[0] = 0;
		DIR *dir = opendir(options->FiletransferDirectory);
		while ((dp=readdir(dir)) != NULL) {
			if(strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) {
				sprintf(filename,"%s/%s",options->FiletransferDirectory,dp->d_name);
				stat(filename,&status);
				if(time_oldest == 0 || status.st_mtime < time_oldest) {
					time_oldest = status.st_mtime;
					size_oldest = status.st_size;
					strcpy(runtime->filetransferFile,filename);
				}
			}
		}	
		closedir(dir);

		/* start filetransfer if file exists */
		if(runtime->filetransferFile[0]) {
			printf("Start Filetransfer: %s\n",runtime->filetransferFile);
			runtime->filetransferCountPackages = dlt_user_log_file_packagesCount(context,runtime->filetransferFile);
			if(runtime->filetransferCountPackages  < 0 )
			{
					printf("Error: dlt_user_log_file_packagesCount\n");
					runtime->filetransferCountPackages = 0;
					runtime->timeFiletransferDelay = options->FiletransferTimeDelay;
					return;
			}			
			runtime->filetransferRunning = 1;
			transferResult = dlt_user_log_file_header(context,runtime->filetransferFile);
			if(transferResult < 0)
			{
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
			runtime->filetransferLastSentPackage++;
			transferResult = dlt_user_log_file_data(context,runtime->filetransferFile,runtime->filetransferLastSentPackage,0);
			if(transferResult < 0)
			{
				printf("Error: dlt_user_log_file_data\n");
				return;
			}			
			/* wait sending next package if more than 50% of buffer used */
			dlt_user_check_buffer(&total_size, &used_size);
			if((total_size - used_size) < (total_size/2))
				break;
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

	pFile = fopen(options->LogFileFilename[num],"r");
	
	if(pFile>0)
	{
		do {
			bytes = fread(buffer,1,sizeof(buffer)-1,pFile);
			
			if(bytes>0) { 
				buffer[bytes] = 0;
				DLT_LOG(*context, DLT_LOG_INFO, DLT_STRING(buffer));				   	
			}
		} while (bytes>0);
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
			if((strcmp(options->LogProcessFilename[num],"*")==0) || 
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
						DLT_LOG(*context, DLT_LOG_INFO, DLT_INT(atoi(dp->d_name)), DLT_STRING(buffer));				   	
					}
				}
				break;
			}
		}
	}	
	closedir(dir);	
	
	if(!found) {
			DLT_LOG(*context, DLT_LOG_INFO, DLT_STRING("Process"), DLT_STRING(options->LogProcessName[num]),DLT_STRING("not running!"));				   	
	}
}
