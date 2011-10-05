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

#include "dlt-system.h"
#include "dlt-system_cfg.h"
#include "dlt-system-log.h"

void dlt_system_log_kernel_version(DltSystemOptions *options,DltContext *context) {
		FILE * pFile;
		char buffer[1024];
		int bytes;
	
		pFile = fopen("/proc/version","r");
		
		if(pFile>0)
		{
			bytes = fread(buffer,1,sizeof(buffer)-1,pFile);
			
			fclose(pFile);
		
			if(bytes>0) { 
				buffer[bytes] = 0;
				DLT_LOG(*context, DLT_LOG_INFO, DLT_STRING(buffer));				   	
			}
		}
}

void dlt_system_log_processes(DltSystemOptions *options,DltContext *context) {
	FILE * pFile;
	struct dirent *dp;
	char filename[256];
	char buffer[1024];
	int bytes;
	int num;

	/* go through all dlt files in directory */
	DIR *dir = opendir("/proc");
	while ((dp=readdir(dir)) != NULL) {
		if(dp->d_name[0]>'0' && dp->d_name[0]<'9') {
			sprintf(filename,"/proc/%s/stat",dp->d_name);
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
			sprintf(filename,"/proc/%s/cmdline",dp->d_name);
			pFile = fopen(filename,"r");
			if(pFile>0)
			{
				bytes = fread(buffer,1,sizeof(buffer)-1,pFile);
				fclose(pFile);
			
				if(bytes>0) {
					for(num=0;num<bytes;num++) {
						if(buffer[num]==0) {							
							buffer[num] = ' ';
						}
					}
					buffer[bytes] = 0;
					DLT_LOG(*context, DLT_LOG_INFO, DLT_INT(atoi(dp->d_name)), DLT_STRING(buffer));				   	
				}
			}
		}
	}	
	closedir(dir);	
}
