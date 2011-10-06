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
**  SRC-MODULE: dlt-system.c                                                  **
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
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "dlt_common.h"
#include "dlt_user.h"

#include "dlt-system.h"
#include "dlt-system_cfg.h"
#include "dlt-system-log.h"

/* Port number, to which the syslogd-ng sends its log messages */
#define MAXSTRLEN             1024

DLT_DECLARE_CONTEXT(syslogContext);
DLT_DECLARE_CONTEXT(processesContext);
DLT_DECLARE_CONTEXT(filetransferContext);
DltContext logFileContext[DLT_SYSTEM_LOG_FILE_MAX];

void dlt_system_init_options(DltSystemOptions *options)
{
	int num;
	
	strncpy(options->ConfigurationFile,DEFAULT_CONFIGURATION_FILE,sizeof(options->ConfigurationFile));
	strncpy(options->ApplicationId,DEFAULT_APPLICATION_ID,sizeof(options->ApplicationId));
	strncpy(options->SyslogContextId,DEFAULT_SYSLOG_CONTEXT_ID,sizeof(options->SyslogContextId));
	options->SyslogPort = DEFAULT_SYSLOG_PORT;
	strncpy(options->FiletransferDirectory,DEFAULT_FILETRANSFER_DIRECTORY,sizeof(options->FiletransferDirectory));
	strncpy(options->FiletransferContextId,DEFAULT_FILETRANSFER_CONTEXT_ID,sizeof(options->FiletransferContextId));
	options->FiletransferTimeStartup = DEFAULT_FILETRANSFER_TIME_STARTUP;
	options->FiletransferTimeDelay = DEFAULT_FILETRANSFER_TIME_DELAY;
	options->LogProcessesMode = DEFAULT_LOG_PROCESSES_MODE;		
	strncpy(options->LogProcessesContextId,DEFAULT_LOG_PROCESSES_CONTEXT_ID,sizeof(options->LogProcessesContextId));
	options->LogFileNumber = 0;
	for(num=0;num<DLT_SYSTEM_LOG_FILE_MAX;num++) {
		options->LogFileFilename[num][0]=0;
		options->LogFileMode[num]=0;
		options->LogFileContextId[num][0]=0;
		options->LogFileTimeDelay[num]=0;
	}
}

int dlt_system_parse_options(DltSystemOptions *options,int argc, char* argv[])
{
	int opt;
    char version[255];
	
    while ((opt = getopt(argc, argv, "c:h")) != -1)
    {
        switch (opt)
        {
			case 'c':
			{
				strncpy(options->ConfigurationFile,optarg,sizeof(options->ConfigurationFile));
				break;
			}
			case 'h':
			{
				dlt_get_version(version);

				printf("Usage: dlt-system [options]\n");
				printf("System information manager and forwarder to DLT daemon.\n");
				printf("%s \n", version);
				printf("Options:\n");
				printf("-c filename  - Set configuration file (default: /etc/dlt-system.conf)\n");
				printf("-h           - This help\n");
				return -1;
				break;
			}
			default: /* '?' */
			{
				fprintf(stderr, "Unknown option '%c'\n", optopt);
				return -1;
			}
        }
    }	
    
    return 0;
}

int dlt_system_parse_configuration(DltSystemOptions *options)
{
	FILE * pFile;
	char line[1024];
	char token[1024];
	char value[1024];
    char *pch;

	/* open configuration file */
	pFile = fopen (options->ConfigurationFile,"r");

	if (pFile!=NULL)
	{
		while(1)
		{
			/* fetch line from configuration file */
			if ( fgets (line , 1024 , pFile) != NULL )
			{
				  //printf("Line: %s\n",line);
				  pch = strtok (line," =\r\n");
				  token[0]=0;
				  value[0]=0;
				  
				  while (pch != NULL)
				  {
					if(strcmp(pch,"#")==0)
						break;

					if(token[0]==0)
					{
						strncpy(token,pch,sizeof(token));
					}
					else
					{
						strncpy(value,pch,sizeof(value));
						break;
					}

					pch = strtok (NULL, " =\r\n");
				  }
				  
				  if(token[0] && value[0])
				  {
						/* parse arguments here */
						if(strcmp(token,"ApplicationId")==0)
						{
							strncpy(options->ApplicationId,value,sizeof(options->ApplicationId));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SyslogContextId")==0)
						{
							strncpy(options->SyslogContextId,value,sizeof(options->SyslogContextId));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SyslogPort")==0)
						{
							options->SyslogPort = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"FiletransferDirectory")==0)
						{
							strncpy(options->FiletransferDirectory,value,sizeof(options->FiletransferDirectory));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"FiletransferContextId")==0)
						{
							strncpy(options->FiletransferContextId,value,sizeof(options->FiletransferContextId));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"FiletransferTimeStartup")==0)
						{
							options->FiletransferTimeStartup = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"FiletransferTimeDelay")==0)
						{
							options->FiletransferTimeDelay = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LogFileFilename")==0)
						{
							strncpy(options->LogFileFilename[options->LogFileNumber],value,sizeof(options->LogFileFilename[options->LogFileNumber]));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LogFileMode")==0)
						{
							options->LogFileMode[options->LogFileNumber] = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LogFileTimeDelay")==0)
						{
							options->LogFileTimeDelay[options->LogFileNumber] = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LogFileContextId")==0)
						{
							strncpy(options->LogFileContextId[options->LogFileNumber],value,sizeof(options->LogFileContextId[options->LogFileNumber]));
							printf("Option: %s=%s\n",token,value);
							if(options->LogFileNumber <  (DLT_SYSTEM_LOG_FILE_MAX-1) )
								options->LogFileNumber++;
						}
						else if(strcmp(token,"LogProcessesMode")==0)
						{
							options->LogProcessesMode = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LogProcessesContextId")==0)
						{
							strncpy(options->LogProcessesContextId,value,sizeof(options->LogProcessesContextId));
							printf("Option: %s=%s\n",token,value);
						}
						else
						{
							fprintf(stderr, "Unknown option: %s=%s\n",token,value);
						}
					}

			}
			else
			{
				break;
			}
		}
		fclose (pFile);
	}
	else
	{
		fprintf(stderr, "Cannot open configuration file: %s\n",options->ConfigurationFile);
		return -1;
	}	

	return 0;
}

int main(int argc, char* argv[])
{
    int sock;
    int bytes_read;
    socklen_t addr_len;
    char recv_data[MAXSTRLEN];
    struct sockaddr_in client_addr, server_addr;
    fd_set rfds;
    struct timeval tv;
    int retval;
    uint32_t lasttime;
    int firsttime = 1;
    int num;

    DltSystemOptions options;
    DltSystemRuntime runtime;

	/* init options */
	dlt_system_init_options(&options);

	/* parse command line options */
	if(dlt_system_parse_options(&options,argc,argv)) {
        return -1;
	}

	/* parse configuration file */
	if(dlt_system_parse_configuration(&options)) {
        return -1;
	}	
	
	/* register application  and contexts */
    DLT_REGISTER_APP(options.ApplicationId,"DLT System Manager");
 	
	DLT_REGISTER_CONTEXT(syslogContext,options.SyslogContextId,"SYSLOG Adapter");
    
 	if(options.LogProcessesMode != DLT_SYSTEM_MODE_OFF)
		DLT_REGISTER_CONTEXT(processesContext,options.LogProcessesContextId,"Log Processes");

	if(options.FiletransferDirectory[0]!=0)
		DLT_REGISTER_CONTEXT(filetransferContext,options.FiletransferContextId,"Filetransfer");

	for(num=0;num<options.LogFileNumber;num++) {
		if(options.LogFileFilename[num][0]!=0)
			DLT_REGISTER_CONTEXT(logFileContext[num],options.LogFileContextId[num],options.LogFileFilename[num]);
		runtime.timeLogFileDelay[num]=0;

	}

	/* create systemd socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }
        server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(options.SyslogPort);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);
    if (bind(sock, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        perror("Bind");
        return -1;
    }
    addr_len = sizeof(struct sockaddr);

   /* Watch sockets to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

	/* init timers */
	lasttime = dlt_uptime();
	runtime.timeStartup = 0;
	runtime.timeFiletransferDelay = 0;

	/* initialise filetransfer manager */
	dlt_system_filetransfer_init(&options,&runtime);

    while (1)
    {
	    /* Wait up to one second. */
		tv.tv_sec = 0;
		if(runtime.filetransferRunning)
			tv.tv_usec = 10000;
		else
			tv.tv_usec = (dlt_uptime()-lasttime+10000)*100;

		/* wait data to be received, or wait min time */
		retval = select(1, &rfds, NULL, NULL, &tv);

		if (retval == -1)
			perror("select()");
		else if (retval)
			;//printf("Data is available now.\n");
		else
			;//printf("No data within one seconds.\n");

		/* call filtransfer even in shorter time schedule */
		if(runtime.filetransferRunning)
			dlt_system_filetransfer_run(&options,&runtime,&filetransferContext);

		if((dlt_uptime()-lasttime) >= 10000)
		{
			/* one second elapsed */
			lasttime = dlt_uptime();
			runtime.timeStartup++;
			
			/* filetransfer manager */
			if(options.FiletransferDirectory[0]!=0) {
				if(runtime.timeStartup > options.FiletransferTimeStartup) {
					if(runtime.timeFiletransferDelay>0) {
						runtime.timeFiletransferDelay--;
					}
					else {
						dlt_system_filetransfer_run(&options,&runtime,&filetransferContext);
					}
				}
			}
			
			/* log kernel version */
			for(num=0;num<options.LogFileNumber;num++) {
				if(((options.LogFileMode[num] == DLT_SYSTEM_MODE_STARTUP) && firsttime) ||
				   (options.LogFileMode[num] == DLT_SYSTEM_MODE_REGULAR) ) {
						if(runtime.timeLogFileDelay[num]<=0) {
							dlt_system_log_file(&options,&logFileContext[num],num);
							runtime.timeLogFileDelay[num]=options.LogFileTimeDelay[num]-1;
					    }
					    else {
							runtime.timeLogFileDelay[num]--;
						}
				}
			}
			
			/* log processes information */
			if(((options.LogProcessesMode == DLT_SYSTEM_MODE_STARTUP) && firsttime) ||
			   (options.LogProcessesMode == DLT_SYSTEM_MODE_REGULAR) ) {
				   dlt_system_log_processes(&options,&processesContext);
			}			
			
			firsttime = 0;
		}

		/* check syslog adapter socket */
		if(FD_ISSET(sock, &rfds))
		{
			bytes_read = 0;

			bytes_read = recvfrom(sock, recv_data, MAXSTRLEN, 0,
								  (struct sockaddr *)&client_addr, &addr_len);
								  
			if (bytes_read == -1)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					exit(1);
				}
			}

			recv_data[bytes_read] = '\0';

			if (bytes_read != 0)
			{
				DLT_LOG(syslogContext, DLT_LOG_INFO, DLT_STRING(recv_data));
			}
		}
    }

	if(options.LogProcessesMode != DLT_SYSTEM_MODE_OFF)
		DLT_UNREGISTER_CONTEXT(processesContext);
		
	if(options.FiletransferDirectory[0]!=0)
		DLT_UNREGISTER_CONTEXT(filetransferContext);

	for(num=0;num<options.LogFileNumber;num++) {
		if(options.LogFileFilename[num][0]!=0)
			DLT_UNREGISTER_CONTEXT(logFileContext[num]);
	}
		
    DLT_UNREGISTER_CONTEXT(syslogContext);	
    DLT_UNREGISTER_APP();

    return 0;
}
