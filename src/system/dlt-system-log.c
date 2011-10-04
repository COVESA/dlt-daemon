#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dlt_common.h"
#include "dlt_user.h"

#include "dlt-system.h"
#include "dlt-system_cfg.h"
#include "dlt-system-log.h"

void dlt_system_log_kernel_version(DltContext *context) {
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
