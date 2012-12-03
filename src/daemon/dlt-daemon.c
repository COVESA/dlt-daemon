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
 * \file dlt-daemon.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-daemon.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
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
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#include <netdb.h>
#include <ctype.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>

#include <sys/timerfd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/stat.h>

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
#include "sd-daemon.h"
#endif

/**
  \defgroup daemon DLT Daemon
  \addtogroup daemon
  \{
*/

/** Global text output buffer, mainly used for creation of error/warning strings */
static char str[DLT_DAEMON_TEXTBUFSIZE];

static DltDaemonTimingPacketThreadData dlt_daemon_timingpacket_thread_data;

static pthread_t      dlt_daemon_timingpacket_thread_handle;
static pthread_attr_t dlt_daemon_timingpacket_thread_attr;

static DltDaemonECUVersionThreadData dlt_daemon_ecu_version_thread_data;
static pthread_t      dlt_daemon_ecu_version_thread_handle;

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
static DltDaemonTimingPacketThreadData dlt_daemon_systemd_watchdog_thread_data;
static pthread_t      dlt_daemon_systemd_watchdog_thread_handle;
#endif

/**
 * Print usage information of tool.
 */
void usage()
{
	char version[DLT_DAEMON_TEXTBUFSIZE];
	dlt_get_version(version);

    //printf("DLT logging daemon %s %s\n", _DLT_PACKAGE_VERSION, _DLT_PACKAGE_VERSION_STATE);
    //printf("Compile options: %s %s %s %s",_DLT_SYSTEMD_ENABLE, _DLT_SYSTEMD_WATCHDOG_ENABLE, _DLT_TEST_ENABLE, _DLT_SHM_ENABLE);
    printf("%s", version);
    printf("Usage: dlt-daemon [options]\n");
    printf("Options:\n");
    printf("  -d            Daemonize\n");
    printf("  -h            Usage\n");
    printf("  -c filename   DLT daemon configuration file (Default: /etc/dlt.conf)\n");
} /* usage() */

/**
 * Option handling
 */
int option_handling(DltDaemonLocal *daemon_local,int argc, char* argv[])
 {
	int c;

	if (daemon_local==0)
	{
		fprintf (stderr, "Invalid parameter passed to option_handling()\n");
		return -1;
	}

    /* Initialize flags */
    memset(daemon_local,0,sizeof(DltDaemonLocal));

    opterr = 0;

    while ((c = getopt (argc, argv, "hdc:")) != -1)
    {
        switch (c)
        {
        case 'd':
        {
            daemon_local->flags.dflag = 1;
            break;
        }
        case 'c':
        {
            strncpy(daemon_local->flags.cvalue,optarg,sizeof(daemon_local->flags.cvalue));
            break;
        }
        case 'h':
        {
            usage();
            return -2; /* return no error */
        }
        case '?':
        {
            if (optopt == 'c')
            {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
            }
            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
        	fprintf (stderr, "Invalid option, this should never occur!\n");
            return -1;
        }
        } /* switch() */
    }

    return 0;

 } /* option_handling() */

/**
 * Option file parser
 */
int option_file_parser(DltDaemonLocal *daemon_local)
{
	FILE * pFile;
	char line[1024];
	char token[1024];
	char value[1024];
    char *pch;
    const char *filename;

	/* set default values for configuration */
	daemon_local->flags.sharedMemorySize = DLT_SHM_SIZE;
	daemon_local->flags.sendMessageTime = 0;
	daemon_local->flags.offlineTraceDirectory[0] = 0;
	daemon_local->flags.offlineTraceFileSize = 1000000;
	daemon_local->flags.offlineTraceMaxSize = 0;
	daemon_local->flags.loggingMode = 0;
	daemon_local->flags.loggingLevel = 6;
	strncpy(daemon_local->flags.loggingFilename, DLT_USER_DIR "/dlt.log",sizeof(daemon_local->flags.loggingFilename));
	daemon_local->flags.sendECUSoftwareVersion = 0;
	memset(daemon_local->flags.pathToECUSoftwareVersion, 0, sizeof(daemon_local->flags.pathToECUSoftwareVersion));

	/* open configuration file */
	if(daemon_local->flags.cvalue[0])
		filename = daemon_local->flags.cvalue;
	else
		filename = "/etc/dlt.conf";
    //printf("Load configuration from file: %s\n",filename);
	pFile = fopen (filename,"r");

	if (pFile!=NULL)
	{
		while(1)
		{
			/* fetch line from configuration file */
			if ( fgets (line , 1024 , pFile) != NULL )
			{
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
						if(strcmp(token,"Verbose")==0)
						{
							daemon_local->flags.vflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PrintASCII")==0)
						{
							daemon_local->flags.aflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PrintHex")==0)
						{
							daemon_local->flags.xflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PrintHeadersOnly")==0)
						{
							daemon_local->flags.sflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendSerialHeader")==0)
						{
							daemon_local->flags.lflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendContextRegistration")==0)
						{
							daemon_local->flags.rflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendMessageTime")==0)
						{
							daemon_local->flags.sendMessageTime = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232SyncSerialHeader")==0)
						{
							daemon_local->flags.mflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"TCPSyncSerialHeader")==0)
						{
							daemon_local->flags.nflag = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232DeviceName")==0)
						{
							strncpy(daemon_local->flags.yvalue,value,sizeof(daemon_local->flags.yvalue));
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232Baudrate")==0)
						{
							strncpy(daemon_local->flags.bvalue,value,sizeof(daemon_local->flags.bvalue));
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"ECUId")==0)
						{
							strncpy(daemon_local->flags.evalue,value,sizeof(daemon_local->flags.evalue));
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PersistanceStoragePath")==0)
						{
							strncpy(daemon_local->flags.ivalue,value,sizeof(daemon_local->flags.ivalue));
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LoggingMode")==0)
						{
							daemon_local->flags.loggingMode = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LoggingLevel")==0)
						{
							daemon_local->flags.loggingLevel = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"LoggingFilename")==0)
						{
							strncpy(daemon_local->flags.loggingFilename,value,sizeof(daemon_local->flags.loggingFilename));
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SharedMemorySize")==0)
						{
							daemon_local->flags.sharedMemorySize = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceDirectory")==0)
						{
							strncpy(daemon_local->flags.offlineTraceDirectory,value,sizeof(daemon_local->flags.offlineTraceDirectory));
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceFileSize")==0)
						{
							daemon_local->flags.offlineTraceFileSize = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceMaxSize")==0)
						{
							daemon_local->flags.offlineTraceMaxSize = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendECUSoftwareVersion")==0)
						{
							daemon_local->flags.sendECUSoftwareVersion = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PathToECUSoftwareVersion")==0)
						{
							strncpy(daemon_local->flags.pathToECUSoftwareVersion,value,sizeof(daemon_local->flags.pathToECUSoftwareVersion));
							//printf("Option: %s=%s\n",token,value);
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
		fprintf(stderr, "Cannot open configuration file: %s\n",filename);
	}	

	return 0;
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
	char version[DLT_DAEMON_TEXTBUFSIZE];
    DltDaemonLocal daemon_local;
    DltDaemon daemon;

    int i,back;

    /* Command line option handling */
	if ((back = option_handling(&daemon_local,argc,argv))<0)
	{
		if(back!=-2) {
			fprintf (stderr, "option_handling() failed!\n");
		}
		return -1;
	}

    /* Configuration file option handling */
	if ((back = option_file_parser(&daemon_local))<0)
	{
		if(back!=-2) {
			fprintf (stderr, "option_file_parser() failed!\n");
		}
		return -1;
	}
	
    /* Initialize internal logging facility */
    dlt_log_set_filename(daemon_local.flags.loggingFilename);
    dlt_log_set_level(daemon_local.flags.loggingLevel);
    dlt_log_init(daemon_local.flags.loggingMode);

    /* Print version information */
    dlt_get_version(version);

    sprintf(str,"Starting DLT Daemon; %s\n", version );
    dlt_log(LOG_NOTICE, str);

	PRINT_FUNCTION_VERBOSE(daemon_local.flags.vflag);

    /* --- Daemon init phase 1 begin --- */
    if (dlt_daemon_local_init_p1(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
    {
    	dlt_log(LOG_CRIT,"Initialization of phase 1 failed!\n");
        return -1;
    }
    /* --- Daemon init phase 1 end --- */

    /* --- Daemon connection init begin */
    if (dlt_daemon_local_connection_init(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
    {
    	dlt_log(LOG_CRIT,"Initialization of local connections failed!\n");
        return -1;
    }
    /* --- Daemon connection init end */

    /* --- Daemon init phase 2 begin --- */
    if (dlt_daemon_local_init_p2(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
    {
    	dlt_log(LOG_CRIT,"Initialization of phase 2 failed!\n");
        return -1;
    }
    /* --- Daemon init phase 2 end --- */

    while (1)
    {
        /* wait for events form all FIFO and sockets */
        daemon_local.read_fds = daemon_local.master;
        if (select(daemon_local.fdmax+1, &(daemon_local.read_fds), NULL, NULL, NULL) == -1)
        {
            dlt_log(LOG_CRIT, "select() failed!\n");
            return -1 ;
        } /* if */

        /* run through the existing FIFO and sockets to check for events */
        for (i = 0; i <= daemon_local.fdmax; i++)
        {
            if (FD_ISSET(i, &(daemon_local.read_fds)))
            {
                if (i == daemon_local.sock && ((daemon.mode == DLT_USER_MODE_EXTERNAL) || (daemon.mode == DLT_USER_MODE_BOTH)))
                {
                    /* event from TCP server socket, new connection */
                    if (dlt_daemon_process_client_connect(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_CRIT,"Connect to dlt client failed!\n");
                        return -1;
                    }
                }
                else if (i == daemon_local.fp)
                {
                    /* event from the FIFO happened */
                    if (dlt_daemon_process_user_messages(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_CRIT,"Processing of messages from user connection failed!\n");
                        return -1;
                    }
                }
                else if ((i == daemon_local.fdserial) && (daemon_local.flags.yvalue[0]))
                {
                    /* event from serial connection to client received */
                    if (dlt_daemon_process_client_messages_serial(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_CRIT,"Processing of messages from serial connection failed!\n");
                        return -1;
                    }
                }
                else
                {
                    /* event from tcp connection to client received */
                    daemon_local.receiverSock.fd = i;
                    if (dlt_daemon_process_client_messages(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_CRIT,"Processing of messages from client connection failed!\n");
						return -1;
                    }
                } /* else */
            } /* if */
        } /* for */
    } /* while */

    dlt_daemon_local_cleanup(&daemon, &daemon_local, daemon_local.flags.vflag);

    dlt_log(LOG_NOTICE, "Leaving DLT daemon\n");

    return 0;

} /* main() */

int dlt_daemon_local_init_p1(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
	int ret;
#endif


    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_init_p1()\n");
        return -1;
    }

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
    ret = sd_booted();

    if(ret == 0){
    	dlt_log(LOG_CRIT, "system not booted with systemd!\n");
//    	return -1;
    }
    else if(ret < 0)
    {
    	dlt_log(LOG_CRIT, "sd_booted failed!\n");
    	return -1;
    }
    else
    {
    	dlt_log(LOG_INFO, "system booted with systemd\n");
    }
#endif



    /* Check for daemon mode */
    if (daemon_local->flags.dflag)
    {
        dlt_daemon_daemonize(daemon_local->flags.vflag);
    }

    /* initialise structure to use DLT file */
    if (dlt_file_init(&(daemon_local->file),daemon_local->flags.vflag)==-1)
    {
		dlt_log(LOG_ERR,"Could not initialize file structure\n");
		/* Return value ignored, dlt daemon will exit */
		dlt_file_free(&(daemon_local->file),daemon_local->flags.vflag);
		return -1;
    }

    signal(SIGPIPE,SIG_IGN);

    signal(SIGTERM, dlt_daemon_signal_handler); /* software termination signal from kill */
    signal(SIGHUP,  dlt_daemon_signal_handler); /* hangup signal */
    signal(SIGQUIT, dlt_daemon_signal_handler);
    signal(SIGINT,  dlt_daemon_signal_handler);
	
    return 0;
}

int dlt_daemon_local_init_p2(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_init_p2()\n");
        return -1;
    }

    /* Daemon data */
    if (dlt_daemon_init(daemon,daemon_local->flags.ivalue,daemon_local->flags.vflag)==-1)
    {
    	dlt_log(LOG_ERR,"Could not initialize daemon data\n");
		return -1;
    }

	/* init offline trace */
	if(((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) && daemon_local->flags.offlineTraceDirectory[0])
	{
		if (dlt_offline_trace_init(&(daemon_local->offlineTrace),daemon_local->flags.offlineTraceDirectory,daemon_local->flags.offlineTraceFileSize,daemon_local->flags.offlineTraceMaxSize)==-1)
		{
			dlt_log(LOG_ERR,"Could not initialize offline trace\n");
			return -1;
		}
	}

    /* Set ECU id of daemon */
    if (daemon_local->flags.evalue[0])
    {
        dlt_set_id(daemon->ecuid,daemon_local->flags.evalue);
    }
    else
    {
        dlt_set_id(daemon->ecuid,DLT_DAEMON_ECU_ID);
    }

    /* Set flag for optional sending of serial header */
    daemon->sendserialheader = daemon_local->flags.lflag;

#ifdef DLT_SHM_ENABLE
	/* init shared memory */
    if (dlt_shm_init_server(&(daemon_local->dlt_shm),DLT_SHM_KEY,daemon_local->flags.sharedMemorySize)==-1)
    {
    	dlt_log(LOG_ERR,"Could not initialize shared memory\n");
		return -1;
    }
#endif
	
    /* prepare main loop */
    if (dlt_message_init(&(daemon_local->msg),daemon_local->flags.vflag)==-1)
    {
    	dlt_log(LOG_ERR,"Could not initialize message\n");
		return -1;
    }

    if (dlt_receiver_init(&(daemon_local->receiver),daemon_local->fp,DLT_DAEMON_RCVBUFSIZE)==-1)
    {
    	dlt_log(LOG_ERR,"Could not initialize receiver\n");
		return -1;
    }
    if (dlt_receiver_init(&(daemon_local->receiverSock),daemon_local->sock,DLT_DAEMON_RCVBUFSIZESOCK)==-1)
	{
    	dlt_log(LOG_ERR,"Could not initialize receiver for socket\n");
		return -1;
    }
    if (daemon_local->flags.yvalue[0])
    {
        if (dlt_receiver_init(&(daemon_local->receiverSerial),daemon_local->fdserial,DLT_DAEMON_RCVBUFSIZESERIAL)==-1)
        {
			dlt_log(LOG_ERR,"Could not initialize receiver for serial connection\n");
			return -1;
        }
    }

    /* setup period thread for timing packets */
    if (pthread_attr_init(&dlt_daemon_timingpacket_thread_attr)<0)
    {
        dlt_log(LOG_WARNING, "Initialization of default thread stack size failed!\n");
    }
    else
    {
        if (pthread_attr_setstacksize(&dlt_daemon_timingpacket_thread_attr,DLT_DAEMON_TIMINGPACKET_THREAD_STACKSIZE)<0)
        {
            dlt_log(LOG_WARNING, "Setting of default thread stack size failed!\n");
        }
    }
    
    /* configure sending timing packets */
    if (daemon_local->flags.sendMessageTime)    
    {
		daemon->timingpackets = 1;
	}
	
    /* Binary semaphore for thread */
    if (sem_init(&dlt_daemon_mutex, 0, 1)==-1)
    {
    	dlt_log(LOG_ERR,"Could not initialize binary semaphore\n");
        return -1;
    }

    /* start thread */
    dlt_daemon_timingpacket_thread_data.daemon = daemon;
    dlt_daemon_timingpacket_thread_data.daemon_local = daemon_local;

    if (pthread_create(&(dlt_daemon_timingpacket_thread_handle),
                       &dlt_daemon_timingpacket_thread_attr,
                       (void *) &dlt_daemon_timingpacket_thread,
                       (void *)&dlt_daemon_timingpacket_thread_data)!=0)
	{
		dlt_log(LOG_ERR,"Could not initialize timing packet thread\n");
		pthread_attr_destroy(&dlt_daemon_timingpacket_thread_attr);
        return -1;
	}

    pthread_attr_destroy(&dlt_daemon_timingpacket_thread_attr);

    /* start thread for ecu version, if enabled */
    if(daemon_local->flags.sendECUSoftwareVersion > 0)
    {
		dlt_daemon_ecu_version_thread_data.daemon = daemon;
		dlt_daemon_ecu_version_thread_data.daemon_local = daemon_local;

		if (pthread_create(&(dlt_daemon_ecu_version_thread_handle),
						   NULL,
						   (void *) &dlt_daemon_ecu_version_thread,
						   (void *)&dlt_daemon_ecu_version_thread_data)!=0)
		{
			dlt_log(LOG_ERR,"Could not initialize ecu version thread\n");
			return -1;
		}
    }

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)

    dlt_daemon_systemd_watchdog_thread_data.daemon = daemon;
    dlt_daemon_systemd_watchdog_thread_data.daemon_local = daemon_local;

	if (pthread_create(&(dlt_daemon_systemd_watchdog_thread_handle),
					   NULL,
					   (void *) &dlt_daemon_systemd_watchdog_thread,
					   (void *) &dlt_daemon_systemd_watchdog_thread_data)!=0)
	{
		dlt_log(LOG_ERR,"Could not initialize systemd watchdog thread\n");
		return -1;
	}
#endif


    return 0;
}

int dlt_daemon_local_connection_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int ret;
    int yes = 1;

    struct sockaddr_in servAddr;
    unsigned int servPort = DLT_DAEMON_TCP_PORT;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_connection_init()\n");
        return -1;
    }

    /* open named pipe(FIFO) to receive DLT messages from users */
    umask(0);

    /* Try to delete existing pipe, ignore result of unlink */
    unlink(DLT_USER_FIFO);

    ret=mkfifo(DLT_USER_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH  | S_IWOTH );
    if (ret==-1)
    {
        sprintf(str,"FIFO user %s cannot be created!\n",DLT_USER_FIFO);
        dlt_log(LOG_ERR, str);
        return -1;
    } /* if */

    daemon_local->fp = open(DLT_USER_FIFO, O_RDWR);
    if (daemon_local->fp==-1)
    {
        sprintf(str,"FIFO user %s cannot be opened!\n",DLT_USER_FIFO);
        dlt_log(LOG_ERR, str);
        return -1;
    } /* if */

    /* create and open socket to receive incoming connections from client */
    if ((daemon_local->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        dlt_log(LOG_ERR, "socket() failed!\n");
        return -1;
    } /* if */

    setsockopt(daemon_local->sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port        = htons(servPort);

    if (bind(daemon_local->sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        dlt_log(LOG_ERR, "bind() failed!\n");
        return -1;
    } /* if */

    if (daemon_local->flags.vflag)
    {
        dlt_log(LOG_INFO, "Bind succesfull\n");
    }

    if (listen(daemon_local->sock, 3) < 0)
    {
        dlt_log(LOG_ERR, "listen() failed!\n");
        return -1;
    } /* if */

    if (daemon_local->flags.vflag)
    {
        dlt_log(LOG_INFO, "Listen succesfull\n");
    }

    /* prepare usage of select(), add FIFO and receiving socket */
    FD_ZERO(&(daemon_local->master));
    FD_ZERO(&(daemon_local->read_fds));
    FD_SET(daemon_local->sock, &(daemon_local->master));

    daemon_local->fdmax = daemon_local->sock;

    FD_SET(daemon_local->fp, &(daemon_local->master));

    if (daemon_local->fp > daemon_local->fdmax)
    {
        daemon_local->fdmax = daemon_local->fp;
    }

    if (daemon_local->flags.yvalue[0])
    {
        /* create and open serial connection from/to client */
        /* open serial connection */
        daemon_local->fdserial=open(daemon_local->flags.yvalue,O_RDWR);
        if (daemon_local->fdserial<0)
        {
            sprintf(str,"Failed to open serial device %s\n", daemon_local->flags.yvalue);
            daemon_local->flags.yvalue[0] = 0;
            dlt_log(LOG_ERR, str);
            return -1;
        }

        if (isatty(daemon_local->fdserial))
        {
            if (daemon_local->flags.bvalue[0])
            {
                daemon_local->baudrate = dlt_convert_serial_speed(atoi(daemon_local->flags.bvalue));
            }
            else
            {
                daemon_local->baudrate = dlt_convert_serial_speed(DLT_DAEMON_SERIAL_DEFAULT_BAUDRATE);
            }

            if (dlt_setup_serial(daemon_local->fdserial,daemon_local->baudrate)<0)
            {
                close(daemon_local->fdserial);
                sprintf(str,"Failed to configure serial device %s (%s) \n", daemon_local->flags.yvalue, strerror(errno));
                daemon_local->flags.yvalue[0] = 0;
                dlt_log(LOG_ERR, str);
                return -1;
            }

            FD_SET(daemon_local->fdserial, &(daemon_local->master));

            if (daemon_local->fdserial > daemon_local->fdmax)
            {
                daemon_local->fdmax = daemon_local->fdserial;
            }

            if (daemon_local->flags.vflag)
            {
                dlt_log(LOG_INFO, "Serial init done\n");
            }
        }
        else
        {
            close(daemon_local->fdserial);
            fprintf(stderr,"Device is not a serial device, device = %s (%s) \n", daemon_local->flags.yvalue, strerror(errno));
            daemon_local->flags.yvalue[0] = 0;
            return -1;
        }
    }

    return 0;
}


void dlt_daemon_local_cleanup(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
		dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_cleanup()\n");
        return;
    }

	/* Ignore result */
    dlt_receiver_free(&(daemon_local->receiver));
    /* Ignore result */
    dlt_receiver_free(&(daemon_local->receiverSock));

	/* Ignore result */
    dlt_message_free(&(daemon_local->msg),daemon_local->flags.vflag);
    close(daemon_local->fp);

	/* free shared memory */
	if(daemon_local->flags.offlineTraceDirectory[0])
		dlt_offline_trace_free(&(daemon_local->offlineTrace));
#if 0
    if (daemon_local->flags.ovalue[0])
    {
        close(daemon_local->ohandle);
    } /* if */
#endif

	/* Ignore result */
    dlt_file_free(&(daemon_local->file),daemon_local->flags.vflag);

    /* Try to delete existing pipe, ignore result of unlink() */
    unlink(DLT_USER_FIFO);

#ifdef DLT_SHM_ENABLE
	/* free shared memory */
	dlt_shm_free_server(&(daemon_local->dlt_shm));
#endif

    /* Try to delete lock file, ignore result of unlink() */
    unlink(DLT_DAEMON_LOCK_FILE);
}

void dlt_daemon_signal_handler(int sig)
{
    switch (sig)
    {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
    case SIGQUIT:
    {
        /* finalize the server */
        //dlt_log("terminate signal catched");
        dlt_log(LOG_NOTICE, "Exiting DLT daemon\n");

        /* Try to delete existing pipe, ignore result of unlink() */
        unlink(DLT_USER_FIFO);

        /* Try to delete lock file, ignore result of unlink() */
        unlink(DLT_DAEMON_LOCK_FILE);

		/* Terminate program */
        exit(0);
        break;
    }
    default:
    {
	/* This case should never occur */
	break;
    }
    } /* switch */
} /* dlt_daemon_signal_handler() */

void dlt_daemon_daemonize(int verbose)
{
    int i,lfp,bytes_written,ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    dlt_log(LOG_NOTICE, "Daemon mode\n");

    /* Daemonize */
    i=fork();
    if (i<0)
    {
    	dlt_log(LOG_CRIT, "Unable to fork(), exiting DLT daemon\n");
        exit(-1); /* fork error */
    }

    if (i>0)
    {
        exit(0); /* parent exits */
    }
    /* child (daemon) continues */

    /* Process independency */

     /* obtain a new process group */
    if (setsid()==-1)
    {
    	dlt_log(LOG_CRIT, "setsid() failed, exiting DLT daemon\n");
        exit(-1); /* fork error */
    }

    /* Close descriptors */
    for (i=getdtablesize();i>=0;--i)
    {
        close(i); /* close all descriptors */
    }

    /* Open standard descriptors stdin, stdout, stderr */
    i=open("/dev/null",O_RDWR); /* open stdin */
    ret=dup(i); /* stdout */
    if (0 > ret){
            dlt_log(LOG_CRIT, "can't open standard descriptor stdout\n");
            exit(-1); /* can not open */
    }
    ret=dup(i); /* stderr */
    if (0 > ret){
            dlt_log(LOG_CRIT, "can't open standard descriptor stderr");
            exit(-1); /* can not open */
    }

    /* Set umask */
    umask(DLT_DAEMON_UMASK);

    /* Change to known directory */
    ret=chdir(DLT_USER_DIR);
    if (0 > ret){
            dlt_log(LOG_CRIT, "Can't change to known directory");
            exit(-1); /* Can't change to known directory */
    }

    /* Ensure single copy of daemon;
       run only one instance at a time */
    lfp=open(DLT_DAEMON_LOCK_FILE,O_RDWR|O_CREAT,DLT_DAEMON_LOCK_FILE_PERM);
    if (lfp<0)
    {
    	dlt_log(LOG_CRIT, "can't open lock file, exiting DLT daemon\n");
        exit(-1); /* can not open */
    }
    if (lockf(lfp,F_TLOCK,0)<0)
    {
    	dlt_log(LOG_CRIT, "can't lock lock file, exiting DLT daemon\n");
        exit(-1); /* can not lock */
    }
    /* only first instance continues */

    sprintf(str,"%d\n",getpid());
    bytes_written=write(lfp,str,strlen(str)); /* record pid to lockfile */
    if (0 > bytes_written){
            dlt_log(LOG_CRIT, "write pid to lockfile failed:");
    }

    /* Catch signals */
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);

} /* dlt_daemon_daemonize() */

int dlt_daemon_process_client_connect(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    socklen_t cli_size;
    struct sockaddr cli;

    int in_sock = -1;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_client_connect()\n");
        return -1;
    }

    /* event from TCP server socket, new connection */
    cli_size = sizeof(cli);
    if ((in_sock  = accept(daemon_local->sock,&cli, &cli_size)) < 0)
    {
        dlt_log(LOG_ERR, "accept() failed!\n");
        return -1 ;
    }

    /* check if file file descriptor was already used, and make it invalid if it is reused */
    /* This prevents sending messages to wrong file descriptor */
    dlt_daemon_applications_invalidate_fd(daemon,in_sock,verbose);
    dlt_daemon_contexts_invalidate_fd(daemon,in_sock,verbose);

    //sprintf("str,"Client Connection from %s\n", inet_ntoa(cli.sin_addr));
    //dlt_log(str);
    FD_SET(in_sock, &(daemon_local->master)); /* add to master set */
    if (in_sock > daemon_local->fdmax)
    {
        /* keep track of the maximum */
        daemon_local->fdmax = in_sock;
    } /* if */

    daemon_local->client_connections++;
    if (daemon_local->flags.vflag)
    {
        sprintf(str, "New connection to client established, #connections: %d\n",daemon_local->client_connections);
        dlt_log(LOG_INFO, str);
    }

    if (daemon_local->client_connections==1)
    {
        if (daemon_local->flags.vflag)
        {
            dlt_log(LOG_INFO, "Send ring-buffer to client\n");
        }
        if (dlt_daemon_send_ringbuffer_to_client(daemon, daemon_local, verbose)==-1)
        {
        	dlt_log(LOG_ERR,"Can't send contents of ringbuffer to clients\n");
			return -1;
        }
		
		/* send new log state to all applications */
		daemon->state = 1;		
		dlt_daemon_user_send_all_log_state(daemon,verbose);
    }

    return 0;
}

int dlt_daemon_process_client_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int bytes_to_be_removed=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_client_messages()\n");
        return -1;
    }

    if (dlt_receiver_receive_socket(&(daemon_local->receiverSock))<=0)
    {
        close(daemon_local->receiverSock.fd);
        FD_CLR(daemon_local->receiverSock.fd, &(daemon_local->master));

        if (daemon_local->client_connections)
        {
            daemon_local->client_connections--;
        }

		if(daemon_local->client_connections==0)
		{
			/* send new log state to all applications */
			daemon->state = 0;		
			dlt_daemon_user_send_all_log_state(daemon,verbose);
		}
				
        if (daemon_local->flags.vflag)
        {
            sprintf(str, "Connection to client lost, #connections: %d\n",daemon_local->client_connections);
            dlt_log(LOG_INFO, str);
        }

        /* check: return 0; */
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),(uint8_t*)daemon_local->receiverSock.buf,daemon_local->receiverSock.bytesRcvd,daemon_local->flags.nflag,daemon_local->flags.vflag)==0)
    {
        /* Check for control message */
        if (DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)))
        {
            dlt_daemon_control_process_control(daemon_local->receiverSock.fd, daemon, &(daemon_local->msg), daemon_local->flags.vflag);
        }

        bytes_to_be_removed = daemon_local->msg.headersize+daemon_local->msg.datasize-sizeof(DltStorageHeader);
        if (daemon_local->msg.found_serialheader)
        {
            bytes_to_be_removed += sizeof(dltSerialHeader);
        }
        if (daemon_local->msg.resync_offset)
        {
            bytes_to_be_removed += daemon_local->msg.resync_offset;
        }

        if (dlt_receiver_remove(&(daemon_local->receiverSock),bytes_to_be_removed)==-1)
        {
        	dlt_log(LOG_ERR,"Can't remove bytes from receiver for sockets\n");
			return -1;
        }

    } /* while */


    if (dlt_receiver_move_to_begin(&(daemon_local->receiverSock))==-1)
    {
    	dlt_log(LOG_ERR,"Can't move bytes to beginning of receiver buffer for sockets\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_client_messages_serial(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int bytes_to_be_removed=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
		dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_client_messages_serial()\n");
        return -1;
    }

    if (dlt_receiver_receive_fd(&(daemon_local->receiverSerial))<=0)
    {
		dlt_log(LOG_ERR, "dlt_receiver_receive_fd() for messages from serial interface failed!\n");
        return -1;
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),(uint8_t*)daemon_local->receiverSerial.buf,daemon_local->receiverSerial.bytesRcvd,daemon_local->flags.mflag,daemon_local->flags.vflag)==0)
    {
        /* Check for control message */
        if (DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)))
        {
            if (dlt_daemon_control_process_control(daemon_local->receiverSerial.fd, daemon, &(daemon_local->msg), daemon_local->flags.vflag)==-1)
            {
				dlt_log(LOG_ERR,"Can't process control messages\n");
				return -1;
            }
        }

        bytes_to_be_removed = daemon_local->msg.headersize+daemon_local->msg.datasize-sizeof(DltStorageHeader);
        if (daemon_local->msg.found_serialheader)
        {
            bytes_to_be_removed += sizeof(dltSerialHeader);
        }
        if (daemon_local->msg.resync_offset)
        {
            bytes_to_be_removed += daemon_local->msg.resync_offset;
        }

        if (dlt_receiver_remove(&(daemon_local->receiverSerial),bytes_to_be_removed)==-1)
		{
        	dlt_log(LOG_ERR,"Can't remove bytes from receiver for serial connection\n");
			return -1;
        }

    } /* while */


    if (dlt_receiver_move_to_begin(&(daemon_local->receiverSerial))==-1)
	{
    	dlt_log(LOG_ERR,"Can't move bytes to beginning of receiver buffer for serial connection\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_messages(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int offset=0;
    int run_loop=1;
    DltUserHeader *userheader;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_messages()\n");
        return -1;
    }

    /* read data from FIFO */
    if (dlt_receiver_receive_fd(&(daemon_local->receiver))<0)
    {
        dlt_log(LOG_ERR, "dlt_receiver_receive_fd() for user messages failed!\n");
        return -1;
    }

    /* look through buffer as long as data is in there */
    do
    {
        if (daemon_local->receiver.bytesRcvd < (int32_t)sizeof(DltUserHeader))
        {
            break;
        }

        /* resync if necessary */
        offset=0;
        do
        {
            userheader = (DltUserHeader*) (daemon_local->receiver.buf+offset);

            /* Check for user header pattern */
            if (dlt_user_check_userheader(userheader))
            {
                break;
            }

            offset++;

        }
        while ((int32_t)(sizeof(DltUserHeader)+offset)<=daemon_local->receiver.bytesRcvd);

        /* Check for user header pattern */
        if (dlt_user_check_userheader(userheader)==0)
        {
            break;
        }

        /* Set new start offset */
        if (offset>0)
        {
            daemon_local->receiver.buf+=offset;
            daemon_local->receiver.bytesRcvd-=offset;
        }

        switch (userheader->message)
        {
        case DLT_USER_MESSAGE_OVERFLOW:
        {
            if (dlt_daemon_process_user_message_overflow(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
            	run_loop=0;
            }
            break;
        }
        case DLT_USER_MESSAGE_REGISTER_CONTEXT:
        {
            if (dlt_daemon_process_user_message_register_context(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        case DLT_USER_MESSAGE_UNREGISTER_CONTEXT:
        {
            if (dlt_daemon_process_user_message_unregister_context(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        case DLT_USER_MESSAGE_LOG:
        {
            if (dlt_daemon_process_user_message_log(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
#ifdef DLT_SHM_ENABLE
        case DLT_USER_MESSAGE_LOG_SHM:
        {
            if (dlt_daemon_process_user_message_log_shm(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
#endif
        case DLT_USER_MESSAGE_REGISTER_APPLICATION:
        {
            if (dlt_daemon_process_user_message_register_application(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        case DLT_USER_MESSAGE_UNREGISTER_APPLICATION:
        {
            if (dlt_daemon_process_user_message_unregister_application(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        case DLT_USER_MESSAGE_APP_LL_TS:
        {
            if (dlt_daemon_process_user_message_set_app_ll_ts(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        case DLT_USER_MESSAGE_LOG_MODE:
        {
            if (dlt_daemon_process_user_message_log_mode(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        default:
        {
            dlt_log(LOG_ERR,"(Internal) Invalid user message type received!\n");

            /* remove user header */
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader))==-1)
            {
				dlt_log(LOG_ERR,"Can't remove bytes from receiver for user messages\n");
				return -1;
			}

            /* In next invocation of do-while loop, a resync will be triggered if additional data was received */
            run_loop=0;

            break;
        }
        }

    }
    while (run_loop);

    /* keep not read data in buffer */
    if (dlt_receiver_move_to_begin(&(daemon_local->receiver))==-1)
	{
    	dlt_log(LOG_ERR,"Can't move bytes to beginning of receiver buffer for user messages\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int j, sent;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_overflow()\n");
        return -1;
    }

    /* Store in daemon, that a message buffer overflow has occured */
    daemon->message_buffer_overflow = DLT_MESSAGE_BUFFER_OVERFLOW;

    /* look if TCP connection to client is available */
    sent = 0;

    for (j = 0; j <= daemon_local->fdmax; j++)
    {
        /* send to everyone! */
        if (FD_ISSET(j, &(daemon_local->master)))
        {
            /* except the listener and ourselves */
            if ((j != daemon_local->fp) && (j != daemon_local->sock))
            {
                dlt_daemon_control_message_buffer_overflow(j, daemon, verbose);
                sent=1;
                /* Reset overflow state */
                daemon->message_buffer_overflow = DLT_MESSAGE_BUFFER_NO_OVERFLOW;
            } /* if */
        } /* if */
    } /* for */

    /* message was not sent, so store it in ringbuffer */
    if (sent==0)
    {
        dlt_daemon_control_message_buffer_overflow(DLT_DAEMON_STORE_TO_BUFFER, daemon, verbose);
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader))==-1)
    {
		dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message overflow\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_register_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    uint32_t len=0;
    DltDaemonApplication *application;
    char description[DLT_DAEMON_DESCSIZE];
    DltUserControlMsgRegisterApplication *usercontext;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_register_application()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterApplication)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    usercontext = (DltUserControlMsgRegisterApplication*) (daemon_local->receiver.buf+sizeof(DltUserHeader));

    memset(description,0,sizeof(description));

    len=usercontext->description_length;
    if ((len>0) && (len<=DLT_DAEMON_DESCSIZE))
    {
        /* Read and store application description */
        strncpy(description, (daemon_local->receiver.buf+sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterApplication)), len);
    }

    application=dlt_daemon_application_add(daemon,usercontext->apid,usercontext->pid,description,verbose);

	/* send log state to new application */
	dlt_daemon_user_send_log_state(daemon,application,verbose);

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterApplication)+len)==-1)
	{
		dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message register application\n");
		return -1;
    }

    if (application==0)
    {
    	dlt_log(LOG_CRIT,"Can't add application");
        return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_register_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    uint32_t len=0;
    int8_t loglevel, tracestatus;
    DltUserControlMsgRegisterContext *usercontext;
    char description[DLT_DAEMON_DESCSIZE];
	DltDaemonApplication *application;
	DltDaemonContext *context;
	DltServiceGetLogInfoRequest *req;

	DltMessage msg;
	int j;
	int sent;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_register_context()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    usercontext = (DltUserControlMsgRegisterContext*) (daemon_local->receiver.buf+sizeof(DltUserHeader));

    memset(description,0,sizeof(description));

    len=usercontext->description_length;
    if ((len>0) && (len<=DLT_DAEMON_DESCSIZE))
    {
        /* Read and store context description */
        strncpy(description, (daemon_local->receiver.buf+sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)), len);
    }

    application = dlt_daemon_application_find(daemon,usercontext->apid,verbose);

    if (application==0)
    {
        dlt_log(LOG_ERR, "Application not found in dlt_daemon_process_user_message_register_context()\n");
        if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
		{
			dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message register context\n");
			return -1;
		}
        return 0;
    }

    /* Pre-set loglevel */
    if (usercontext->log_level == DLT_USER_LOG_LEVEL_NOT_SET)
    {
        loglevel=DLT_LOG_DEFAULT;
    }
    else
    {
        loglevel=usercontext->log_level;
        /* Plausibility check */
        if ((loglevel<DLT_LOG_DEFAULT) || (loglevel>DLT_LOG_VERBOSE))
        {
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
            {
				dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message register context\n");
			}
            return -1;
        }
    }

    /* Pre-set tracestatus */
    if (usercontext->trace_status == DLT_USER_TRACE_STATUS_NOT_SET)
    {
        tracestatus=DLT_TRACE_STATUS_DEFAULT;
    }
    else
    {
        tracestatus=usercontext->trace_status;

        /* Plausibility check */
        if ((tracestatus<DLT_TRACE_STATUS_DEFAULT) || (tracestatus>DLT_TRACE_STATUS_ON))
        {
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
			{
				dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message register context\n");
			}
            return -1;
        }
    }

    context = dlt_daemon_context_add(daemon,usercontext->apid,usercontext->ctid, loglevel, tracestatus, usercontext->log_level_pos,application->user_handle,description,verbose);

    if (context==0)
    {
        if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
		{
			dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message register context\n");
		}

		dlt_log(LOG_CRIT,"Can't add context");
        return -1;
    }
    /* Create automatic get log info response for registered context */
    if (daemon_local->flags.rflag)
    {
        /* Prepare request for get log info with one application and one context */
        if (dlt_message_init(&msg, verbose)==-1)
        {
        	dlt_log(LOG_ERR,"Can't initialize message");
        	return -1;
        }

        msg.datasize = sizeof(DltServiceGetLogInfoRequest);
        if (msg.databuffer && (msg.databuffersize < msg.datasize))
        {
            free(msg.databuffer);
            msg.databuffer=0;
        }
        if (msg.databuffer == 0){
        	msg.databuffer = (uint8_t *) malloc(msg.datasize);
        	msg.databuffersize = msg.datasize;
        }
        if (msg.databuffer==0)
        {
        	dlt_log(LOG_ERR,"Can't allocate buffer for get log info message\n");
			return -1;
        }

        req = (DltServiceGetLogInfoRequest*) msg.databuffer;

        req->service_id = DLT_SERVICE_ID_GET_LOG_INFO;
        req->options = 7;
        dlt_set_id(req->apid, usercontext->apid);
        dlt_set_id(req->ctid, usercontext->ctid);
        dlt_set_id(req->com,"remo");

        sent=0;

        /* Send response to get log info request to DLT clients */
        for (j = 0; j <= daemon_local->fdmax; j++)
        {
            /* send to everyone! */
            if (FD_ISSET(j, &(daemon_local->master)))
            {
                /* except the listener and ourselves */
                if ((j != daemon_local->fp) && (j != daemon_local->sock))
                {
                    dlt_daemon_control_get_log_info(j , daemon, &msg, verbose);
                    sent=1;
                }
            }
        }

        if (sent==0)
        {
            /* Store to buffer */
            dlt_daemon_control_get_log_info(DLT_DAEMON_STORE_TO_BUFFER , daemon, &msg, verbose);
        }

        dlt_message_free(&msg, verbose);
    }

    if (context->user_handle >= DLT_FD_MINIMUM)
    {
        /* This call also replaces the default values with the values defined for default */
        if (dlt_daemon_user_send_log_level(daemon, context, verbose)==-1)
        {
			dlt_log(LOG_ERR,"Can't send current log level as response to user message register context\n");
			return -1;
        }
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
	{
		dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message register context\n");
		return -1;
	}

    return 0;
}

int dlt_daemon_process_user_message_unregister_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltUserControlMsgUnregisterApplication *usercontext;
    DltDaemonApplication *application;
    DltDaemonContext *context;
    int i, offset_base;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_unregister_application()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterApplication)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    if (daemon->num_applications>0)
    {
        usercontext = (DltUserControlMsgUnregisterApplication*) (daemon_local->receiver.buf+sizeof(DltUserHeader));

        /* Delete this application and all corresponding contexts for this application from internal table */
        application = dlt_daemon_application_find(daemon,usercontext->apid, verbose);

        if (application)
        {
            /* Calculate start offset within contexts[] */
            offset_base=0;
            for (i=0; i<(application-(daemon->applications)); i++)
            {
                offset_base+=daemon->applications[i].num_contexts;
            }

            for (i=application->num_contexts-1; i>=0; i--)
            {
                context = &(daemon->contexts[offset_base+i]);
                if (context)
                {
                    /* Delete context */
                    if (dlt_daemon_context_del(daemon, context, verbose)==-1)
                    {
                    	dlt_log(LOG_ERR,"Can't delete context for user message unregister application\n");
                    	return -1;
                    }
                }
            }

            /* Delete this application entry from internal table*/
            if (dlt_daemon_application_del(daemon, application, verbose)==-1)
            {
            	dlt_log(LOG_ERR,"Can't delete application for user message unregister application\n");
				return -1;
            }
        }
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterApplication))==-1)
    {
    	dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message unregister application\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_unregister_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	DltUserControlMsgUnregisterContext *usercontext;
	DltDaemonContext *context;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_unregister_context()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    usercontext = (DltUserControlMsgUnregisterContext*) (daemon_local->receiver.buf+sizeof(DltUserHeader));
    context = dlt_daemon_context_find(daemon,usercontext->apid, usercontext->ctid, verbose);

    if (context)
    {
        /* Delete this connection entry from internal table*/
        if (dlt_daemon_context_del(daemon, context, verbose)==-1)
        {
			dlt_log(LOG_ERR,"Can't delete context for user message unregister context\n");
			return -1;
		}
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext))==-1)
    {
    	dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message unregister context\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_log(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int bytes_to_be_removed;
    int j,sent,third_value;
    ssize_t ret;

    static char text[DLT_DAEMON_TEXTSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_log()\n");
        return -1;
    }

    if (dlt_message_read(&(daemon_local->msg),(unsigned char*)daemon_local->receiver.buf+sizeof(DltUserHeader),daemon_local->receiver.bytesRcvd-sizeof(DltUserHeader),0,verbose)==0)
    {
        /* set overwrite ecu id */
        if (daemon_local->flags.evalue!=0)
        {
            /* Set header extra parameters */
            dlt_set_id(daemon_local->msg.headerextra.ecu, daemon->ecuid );
            //msg.headerextra.seid = 0;
            if (dlt_message_set_extraparameters(&(daemon_local->msg),0)==-1)
            {
            	dlt_log(LOG_ERR,"Can't set message extra parameters in process user message log\n");
				return -1;
            }

            /* Correct value of timestamp, this was changed by dlt_message_set_extraparameters() */
            daemon_local->msg.headerextra.tmsp = DLT_BETOH_32(daemon_local->msg.headerextra.tmsp);
        }

        /* prepare storage header */
        if (DLT_IS_HTYP_WEID(daemon_local->msg.standardheader->htyp))
        {
            if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon_local->msg.headerextra.ecu)==-1)
            {
				dlt_log(LOG_ERR,"Can't set storage header in process user message log\n");
				return -1;
            }
        }
        else
        {
            if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon->ecuid)==-1)
            {
				dlt_log(LOG_ERR,"Can't set storage header in process user message log\n");
				return -1;
            }
        }

        {
            /* if no filter set or filter is matching display message */
            if (daemon_local->flags.xflag)
            {
                if (dlt_message_print_hex(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
				{
					dlt_log(LOG_ERR,"dlt_message_print_hex() failed!\n");
				}
            } /*  if */
            else if (daemon_local->flags.aflag)
            {
                if (dlt_message_print_ascii(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
                {
					dlt_log(LOG_ERR,"dlt_message_print_ascii() failed!\n");
				}
            } /* if */
            else if (daemon_local->flags.sflag)
            {
                if (dlt_message_print_header(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
				{
					dlt_log(LOG_ERR,"dlt_message_print_header() failed!\n");
				}
                /* print message header only */
            } /* if */

            sent=0;

			/* write message to offline trace */
			if(((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) && daemon_local->flags.offlineTraceDirectory[0])
			{
				dlt_offline_trace_write(&(daemon_local->offlineTrace),daemon_local->msg.headerbuffer,daemon_local->msg.headersize,
										daemon_local->msg.databuffer,daemon_local->msg.datasize,0,0);
				sent = 1;
			}

            /* look if TCP connection to client is available */
            for (j = 0;((daemon->mode == DLT_USER_MODE_EXTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) &&  (j <= daemon_local->fdmax); j++)
            {
                /* send to everyone! */
                if (FD_ISSET(j, &(daemon_local->master)))
                {
                    /* except the listener and ourselves */
                    if (daemon_local->flags.yvalue[0])
                    {
                        third_value = daemon_local->fdserial;
                    }
                    else
                    {
                        third_value = daemon_local->sock;
                    }

                    if ((j != daemon_local->fp) && (j != daemon_local->sock) && (j != third_value))
                    {
                        DLT_DAEMON_SEM_LOCK();

                        if (daemon_local->flags.lflag)
                        {
                            send(j,dltSerialHeader,sizeof(dltSerialHeader),0);
                        }

                        send(j,daemon_local->msg.headerbuffer+sizeof(DltStorageHeader),daemon_local->msg.headersize-sizeof(DltStorageHeader),0);
                        send(j,daemon_local->msg.databuffer,daemon_local->msg.datasize,0);

                        DLT_DAEMON_SEM_FREE();

                        sent=1;
                    } /* if */
                    else if ((j == daemon_local->fdserial) && (daemon_local->flags.yvalue!=0))
                    {
                        DLT_DAEMON_SEM_LOCK();

                        if (daemon_local->flags.lflag)
                        {
                            ret=write(j,dltSerialHeader,sizeof(dltSerialHeader));
                            if (0 > ret)
                            {
                              dlt_log(LOG_ERR,"write(j,daemon_local->msg.headerbuffer failed\n");
                            }
                        }

                        ret=write(j,daemon_local->msg.headerbuffer+sizeof(DltStorageHeader),daemon_local->msg.headersize-sizeof(DltStorageHeader));
                        if (0 > ret)
                        {
                          dlt_log(LOG_ERR,"write(j,dltSerialHeader failed\n");
                        }
                        ret=write(j,daemon_local->msg.databuffer,daemon_local->msg.datasize);
                        if (0 > ret)
                        {
                          dlt_log(LOG_ERR,"write(j,daemon_local->msg.databuffer failed\n");
                        }

                        DLT_DAEMON_SEM_FREE();

                        sent=1;
                    }
                } /* if */
            } /* for */

            /* Message was not sent to client, so store it in client ringbuffer */
            if (sent==0)
            {
            	DLT_DAEMON_SEM_LOCK();
                if (dlt_buffer_push3(&(daemon->client_ringbuffer),
                                    daemon_local->msg.headerbuffer+sizeof(DltStorageHeader),daemon_local->msg.headersize-sizeof(DltStorageHeader),
                                    daemon_local->msg.databuffer,daemon_local->msg.datasize,
                                    0, 0
                                   )<0)
				{
					dlt_log(LOG_ERR,"Storage of message in history buffer failed! Message discarded.\n");
				}
                DLT_DAEMON_SEM_FREE();
            }

        }
        /* keep not read data in buffer */
        bytes_to_be_removed = daemon_local->msg.headersize+daemon_local->msg.datasize-sizeof(DltStorageHeader)+sizeof(DltUserHeader);
        if (daemon_local->msg.found_serialheader)
        {
            bytes_to_be_removed += sizeof(dltSerialHeader);
        }

        if (dlt_receiver_remove(&(daemon_local->receiver),bytes_to_be_removed)==-1)
        {
        	dlt_log(LOG_ERR,"Can't remove bytes from receiver\n");
        	return -1;
        }
    }
    else
    {
    	dlt_log(LOG_ERR,"Can't read messages from receiver\n");
        return -1;
    }

    return 0;
}

#ifdef DLT_SHM_ENABLE
int dlt_daemon_process_user_message_log_shm(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int bytes_to_be_removed=0;
    int j,sent,third_value;
    ssize_t ret;
    uint8_t rcv_buffer[10000];
    int size;
    DltUserHeader *userheader;

    static char text[DLT_DAEMON_TEXTSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_log()\n");
        return -1;
    }

    userheader = (DltUserHeader*) (daemon_local->receiver.buf);
  
	//dlt_shm_status(&(daemon_local->dlt_shm));
	while (1)
    {		
		/* log message in SHM */
		if((size = dlt_shm_copy(&(daemon_local->dlt_shm),rcv_buffer,10000)) <= 0)
			break;
		if (dlt_message_read(&(daemon_local->msg),rcv_buffer,size,0,verbose)!=0) {
			break;
			dlt_log(LOG_ERR,"Can't read messages from shm\n");
			return -1;
		}				
		bytes_to_be_removed = daemon_local->msg.headersize+daemon_local->msg.datasize-sizeof(DltStorageHeader)+sizeof(DltUserHeader);
		if (daemon_local->msg.found_serialheader)
		{
			bytes_to_be_removed += sizeof(dltSerialHeader);
		}
		
		/* set overwrite ecu id */
		if (daemon_local->flags.evalue[0])
		{
			/* Set header extra parameters */
			dlt_set_id(daemon_local->msg.headerextra.ecu, daemon->ecuid );
			//msg.headerextra.seid = 0;
			if (dlt_message_set_extraparameters(&(daemon_local->msg),0)==-1)
			{
				dlt_log(LOG_ERR,"Can't set message extra parameters in process user message log\n");
				dlt_shm_remove(&(daemon_local->dlt_shm));
				return -1;
			}

			/* Correct value of timestamp, this was changed by dlt_message_set_extraparameters() */
			daemon_local->msg.headerextra.tmsp = DLT_BETOH_32(daemon_local->msg.headerextra.tmsp);
		}

		/* prepare storage header */
		if (DLT_IS_HTYP_WEID(daemon_local->msg.standardheader->htyp))
		{
			if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon_local->msg.headerextra.ecu)==-1)
			{
				dlt_log(LOG_ERR,"Can't set storage header in process user message log\n");
				dlt_shm_remove(&(daemon_local->dlt_shm));
				return -1;
			}
		}
		else
		{
			if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon->ecuid)==-1)
			{
				dlt_log(LOG_ERR,"Can't set storage header in process user message log\n");
				dlt_shm_remove(&(daemon_local->dlt_shm));
				return -1;
			}
		}

		/* display message */
		if (daemon_local->flags.xflag)
		{
			if (dlt_message_print_hex(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_ERR,"dlt_message_print_hex() failed!\n");
			}
		} /*  if */
		else if (daemon_local->flags.aflag)
		{
			if (dlt_message_print_ascii(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_ERR,"dlt_message_print_ascii() failed!\n");
			}
		} /* if */
		else if (daemon_local->flags.sflag)
		{
			if (dlt_message_print_header(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_ERR,"dlt_message_print_header() failed!\n");
			}
			/* print message header only */
		} /* if */

		sent=0;

		/* write message to offline trace */
		if(((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) && daemon_local->flags.offlineTraceDirectory[0])
		{
			dlt_offline_trace_write(&(daemon_local->offlineTrace),daemon_local->msg.headerbuffer,daemon_local->msg.headersize,
									daemon_local->msg.databuffer,daemon_local->msg.datasize,0,0);
			sent = 1;
		}

		/* look if TCP connection to client is available */
		for (j = 0;((daemon->mode == DLT_USER_MODE_EXTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) &&  (j <= daemon_local->fdmax); j++)
		{
			/* send to everyone! */
			if (FD_ISSET(j, &(daemon_local->master)))
			{
				/* except the listener and ourselves */
				if (daemon_local->flags.yvalue[0])
				{
					third_value = daemon_local->fdserial;
				}
				else
				{
					third_value = daemon_local->sock;
				}

				if ((j != daemon_local->fp) && (j != daemon_local->sock) && (j != third_value))
				{
					DLT_DAEMON_SEM_LOCK();

					if (daemon_local->flags.lflag)
					{
						send(j,dltSerialHeader,sizeof(dltSerialHeader),0);
					}

					send(j,daemon_local->msg.headerbuffer+sizeof(DltStorageHeader),daemon_local->msg.headersize-sizeof(DltStorageHeader),0);
					send(j,daemon_local->msg.databuffer,daemon_local->msg.datasize,0);

					DLT_DAEMON_SEM_FREE();

					sent=1;
				} /* if */
				else if ((j == daemon_local->fdserial) && (daemon_local->flags.yvalue[0]))
				{
					DLT_DAEMON_SEM_LOCK();

					if (daemon_local->flags.lflag)
					{
						ret=write(j,dltSerialHeader,sizeof(dltSerialHeader));
					}

					ret=write(j,daemon_local->msg.headerbuffer+sizeof(DltStorageHeader),daemon_local->msg.headersize-sizeof(DltStorageHeader));
					ret=write(j,daemon_local->msg.databuffer,daemon_local->msg.datasize);

					DLT_DAEMON_SEM_FREE();

					sent=1;
				}
			} /* if */
		} /* for */

		/* Message was not sent to client, so store it in client ringbuffer */
		if (sent==1 || (daemon->mode == DLT_USER_MODE_OFF))
		{
			if(userheader->message == DLT_USER_MESSAGE_LOG_SHM) {
				/* dlt message was sent, remove from buffer if log message from shm */
				dlt_shm_remove(&(daemon_local->dlt_shm));
			}			
		}
		else
		{
			/* dlt message was not sent, keep in buffer */
			break;
		}
		
	}

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader))==-1)
    {
		dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message overflow\n");
		return -1;
    }
    
    return 0;
}
#endif

int dlt_daemon_process_user_message_set_app_ll_ts(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    DltUserControlMsgAppLogLevelTraceStatus *usercontext;
    DltDaemonApplication *application;
    DltDaemonContext *context;
    int i, offset_base;
    int8_t old_log_level, old_trace_status;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_set_app_ll_ts()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgAppLogLevelTraceStatus )))
    {
    	/* Not enough bytes receeived */
        return -1;
    }

    if (daemon->num_applications>0)
    {
        usercontext = (DltUserControlMsgAppLogLevelTraceStatus*) (daemon_local->receiver.buf+sizeof(DltUserHeader));

        /* Get all contexts with application id matching the received application id */
        application = dlt_daemon_application_find(daemon, usercontext->apid, verbose);
        if (application)
        {
            /* Calculate start offset within contexts[] */
            offset_base=0;
            for (i=0; i<(application-(daemon->applications)); i++)
            {
                offset_base+=daemon->applications[i].num_contexts;
            }

            for (i=0; i < application->num_contexts; i++)
            {
                context = &(daemon->contexts[offset_base+i]);
                if (context)
                {
                    old_log_level = context->log_level;
                    context->log_level = usercontext->log_level; /* No endianess conversion necessary*/

                    old_trace_status = context->trace_status;
                    context->trace_status = usercontext->trace_status;   /* No endianess conversion necessary */

                    /* The folowing function sends also the trace status */
                    if (context->user_handle >= DLT_FD_MINIMUM && dlt_daemon_user_send_log_level(daemon, context, verbose)!=0)
                    {
                        context->log_level = old_log_level;
                        context->trace_status = old_trace_status;
                    }
                }
            }
        }
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgAppLogLevelTraceStatus))==-1)
	{
		dlt_log(LOG_ERR,"Can't remove bytes from receiver\n");
		return -1;
	}

    return 0;
}

int dlt_daemon_process_user_message_log_mode(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	DltUserControlMsgLogMode *logmode;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_log_mode()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    logmode = (DltUserControlMsgLogMode*) (daemon_local->receiver.buf+sizeof(DltUserHeader));

	/* set the new log mode */
	daemon->mode = logmode->log_mode;

	/* write configuration persistantly */
	dlt_daemon_configuration_save(daemon, daemon->runtime_configuration, verbose);

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgLogMode))==-1)
    {
    	dlt_log(LOG_ERR,"Can't remove bytes from receiver for user message log mode\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_send_ringbuffer_to_client(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    static uint8_t data[DLT_DAEMON_RCVBUFSIZE];
    int length;
    int j, third_value;
    ssize_t ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_send_ringbuffer_to_client()\n");
        return -1;
    }

	/* Attention: If the message can't be send at this time, it will be silently discarded. */
    while ((length = dlt_buffer_pull(&(daemon->client_ringbuffer), data, sizeof(data) )) > 0)
    {
        /* look if TCP connection to client is available */
        for (j = 0; j <= daemon_local->fdmax; j++)
        {
            /* send to everyone! */
            if (FD_ISSET(j, &(daemon_local->master)))
            {
                /* except the listener and ourselves */
                if (daemon_local->flags.yvalue[0])
                {
                    third_value = daemon_local->fdserial;
                }
                else
                {
                    third_value = daemon_local->sock;
                }

                if ((j != daemon_local->fp) && (j != daemon_local->sock) && (j != third_value))
                {
                    DLT_DAEMON_SEM_LOCK();

                    if (daemon_local->flags.lflag)
                    {
                        send(j,dltSerialHeader,sizeof(dltSerialHeader),0);
                    }
                    send(j,data,length,0);

                    DLT_DAEMON_SEM_FREE();

                } /* if */
                else if ((j == daemon_local->fdserial) && (daemon_local->flags.yvalue[0]))
                {
                    DLT_DAEMON_SEM_LOCK();

                    if (daemon_local->flags.lflag)
                    {
                        ret=write(j,dltSerialHeader,sizeof(dltSerialHeader));
                        if (0 > ret)
                        {
                                dlt_log(LOG_ERR, "dlt_daemon_send_ringbuffer_to_client: write(j,dltSerialHeader,sizeof(dltSerialHeader)) failed!\n");
                                DLT_DAEMON_SEM_FREE();
                                return -1;
                        }
                    }
                    ret=write(j,data,length);
                    if (0 > ret)
                    {
                            dlt_log(LOG_ERR, "dlt_daemon_send_ringbuffer_to_client: write(j,data,length) failed!\n");
                            DLT_DAEMON_SEM_FREE();
                            return -1;
                    }
                    DLT_DAEMON_SEM_FREE();

                }
            } /* if */
        } /* for */
        length = sizeof(data);
    }

    return 0;
}

void dlt_daemon_timingpacket_thread(void *ptr)
{
    DltDaemonPeriodicData info;
    int j;

    DltDaemonTimingPacketThreadData *data;
    DltDaemon *daemon;
    DltDaemonLocal *daemon_local;

    if (ptr==0)
    {
    	dlt_log(LOG_ERR, "No data pointer passed to timingpacket thread\n");
        return;
    }

    data = (DltDaemonTimingPacketThreadData*)ptr;
    daemon = data->daemon;
    daemon_local = data->daemon_local;

    if ((daemon==0) || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_timingpacket_thread()");
        return;
    }

    if (dlt_daemon_make_periodic (1000000, &info, daemon_local->flags.vflag)<0)
    {
        dlt_log(LOG_CRIT,"Can't initialize thread timer!\n");
        return;
    }

    while (1)
    {
        /* If enabled, send timing packets to all clients */
        if (daemon->timingpackets)
        {
            for (j = 0; j <= daemon_local->fdmax; j++)
            {
                /* send to everyone! */
                if (FD_ISSET(j, &(daemon_local->master)))
                {
                    /* except the listener and ourselves */
                    if ((j != daemon_local->fp) && (j != daemon_local->sock))
                    {
                        dlt_daemon_control_message_time(j, daemon, daemon_local->flags.vflag);
                    }
                }
            }
        }
        /* Wait for next period */
        dlt_daemon_wait_period (&info, daemon_local->flags.vflag);
    }
}

void dlt_daemon_ecu_version_thread(void *ptr)
{
	DltDaemonECUVersionThreadData *data = (DltDaemonECUVersionThreadData *)ptr;
	DltDaemonPeriodicData info;
	char version[DLT_DAEMON_TEXTBUFSIZE];
	if(data->daemon_local->flags.pathToECUSoftwareVersion[0] == 0)
	{
		dlt_get_version(version);
	}
	else
	{
		size_t bufpos 	= 0;
		size_t read		= 0;
		FILE *f 		= fopen(data->daemon_local->flags.pathToECUSoftwareVersion, "r");

		if(f == NULL)
		{
			dlt_log(LOG_ERR, "Failed to open ECU Software version file.\n");
			return;
		}

		while(!feof(f))
		{
			char buf[DLT_DAEMON_TEXTBUFSIZE];
			read = fread(buf, 1, DLT_DAEMON_TEXTBUFSIZE, f);
			if(ferror(f))
			{
				dlt_log(LOG_ERR, "Failed to read ECU Software version file.\n");
				return;
			}
			if(bufpos + read > DLT_DAEMON_TEXTBUFSIZE)
			{
				dlt_log(LOG_ERR, "Too long file for ecu version info.\n");
				fclose(f);
				return;
			}
			strncpy(version+bufpos, buf, read);
			bufpos += read;
		}
		fclose(f);
	}

    if (dlt_daemon_make_periodic (1000000*60, &info, data->daemon_local->flags.vflag)<0)
    {
        dlt_log(LOG_CRIT,"Can't initialize thread timer!\n");
        return;
    }

	while(1)
	{
		int i;
		for (i = 0; i <= data->daemon_local->fdmax; i++)
		{
			/* send to everyone! */
			if (FD_ISSET(i, &(data->daemon_local->master)))
			{
				/* except the listener and ourselves */
				if ((i != data->daemon_local->fp) && (i != data->daemon_local->sock))
				{
					dlt_daemon_control_send_ecu_version(i, data->daemon, version, data->daemon_local->flags.vflag);
				}
			}
		}
		dlt_daemon_wait_period (&info, data->daemon_local->flags.vflag);
	}

}

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
void dlt_daemon_systemd_watchdog_thread(void *ptr)
{
    char *watchdogUSec;
    int watchdogTimeoutSeconds;
    int notifiyPeriodNSec;
    DltDaemonPeriodicData info;
    DltDaemonTimingPacketThreadData *data;
    DltDaemon *daemon;
    DltDaemonLocal *daemon_local;

    if (ptr==0)
    {
    	dlt_log(LOG_ERR, "No data pointer passed to systemd watchdog thread\n");
        return;
    }

    data = (DltDaemonTimingPacketThreadData*)ptr;
    daemon = data->daemon;
    daemon_local = data->daemon_local;

    if ((daemon==0) || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_timingpacket_thread()");
        return;
    }

    watchdogUSec = getenv("WATCHDOG_USEC");

	if(watchdogUSec)
	{
		watchdogTimeoutSeconds = atoi(watchdogUSec);

		if( watchdogTimeoutSeconds > 0 ){

			// Calculate half of WATCHDOG_USEC in ns for timer tick
			notifiyPeriodNSec = watchdogTimeoutSeconds / 2 ;

			sprintf(str,"systemd watchdog timeout: %i nsec - timer will be initialized: %i nsec\n", watchdogTimeoutSeconds, notifiyPeriodNSec );
			dlt_log(LOG_INFO, str);

			if (dlt_daemon_make_periodic (notifiyPeriodNSec, &info, daemon_local->flags.vflag)<0)
			{
				dlt_log(LOG_CRIT, "Could not initialize systemd watchdog timer");
				return;
			}

			while (1)
			{
				if(sd_notify(0, "WATCHDOG=1") < 0)
				{
					dlt_log(LOG_CRIT, "Could not reset systemd watchdog\n");
				}
				//dlt_log(LOG_INFO, "systemd watchdog waited periodic\n");
				/* Wait for next period */
				dlt_daemon_wait_period (&info, daemon_local->flags.vflag);
			}
		}
		else
		{
			sprintf(str,"systemd watchdog timeout incorrect: %i\n", watchdogTimeoutSeconds);
			dlt_log(LOG_CRIT, str);
		}
	}
	else
	{
		dlt_log(LOG_CRIT, "systemd watchdog timeout (WATCHDOG_USEC) is null!\n");
	}
}
#endif

int dlt_daemon_make_periodic (unsigned int period, DltDaemonPeriodicData *info, int verbose)
{
    int ret;
    unsigned int ns;
    unsigned int sec;
    int fd;
    struct itimerspec itval;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (info==0)
    {
    	dlt_log(LOG_ERR,"No data pointer passed!\n");
        return -1;
    }

    /* Create the timer */
    fd = timerfd_create (CLOCK_MONOTONIC, 0);

    info->wakeups_missed = 0;
    info->timer_fd = fd;

    if (fd == -1)
    {
    	dlt_log(LOG_ERR,"Can't create timer filedescriptor");
        return -1;
    }

    /* Make the timer periodic */
    sec = period/1000000;
    ns = (period - (sec * 1000000)) * 1000;
    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;

    ret = timerfd_settime (fd, 0, &itval, NULL);

    return ret;
}

void dlt_daemon_wait_period (DltDaemonPeriodicData *info, int verbose)
{
    unsigned long long missed;
    int ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    ret = read (info->timer_fd, &missed, sizeof (missed));
    if (0 > ret){
            dlt_log(LOG_ERR,"dlt_daemon_wait_period: Read failed");
    }

    if (missed > 0)
    {
        info->wakeups_missed += (missed - 1);
    }
}

/**
  \}
*/
