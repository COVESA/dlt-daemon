/*
 * Dlt Daemon - Diagnostic Log and Trace
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

/** \page Contents
  * The package automotive-dlt includes the following items:
  * - dlt daemon (dlt-daemon)
  * - adptors to to interface the daemon (dlt-adaptor-stdin, dlt-adaptor-udp)
  * - dlt client gui (dlt-viewer)
  * - dlt console tools (dlt-receive, dlt-convert)
  * - examples (dlt-example-user, dlt-example-user-func, dlt-example-ringbuffer)
  * - a library including user-application, client and common functions
  */

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

/**
 * Print usage information of tool.
 */
void usage()
{
    printf("Usage: dlt-daemon [options]\n");
    printf("DLT logging daemon\n");
    printf("Options:\n");
    printf("  -d            Daemonize\n");
    printf("  -v            Verbose mode\n");
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

    while ((c = getopt (argc, argv, "hvdc:")) != -1)
    {
        switch (c)
        {
        case 'v':
        {
            daemon_local->flags.vflag = 1;
            break;
        }
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
						if(strcmp(token,"Verbose")==0)
						{
							daemon_local->flags.vflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PrintASCII")==0)
						{
							daemon_local->flags.aflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PrintHex")==0)
						{
							daemon_local->flags.xflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PrintHeadersOnly")==0)
						{
							daemon_local->flags.sflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"Daemonize")==0)
						{
							daemon_local->flags.dflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendSerialHeader")==0)
						{
							daemon_local->flags.lflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendContextRegistration")==0)
						{
							daemon_local->flags.rflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendMessageTime")==0)
						{
							daemon_local->flags.sendMessageTime = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232SyncSerialHeader")==0)
						{
							daemon_local->flags.mflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"TCPSyncSerialHeader")==0)
						{
							daemon_local->flags.nflag = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232DeviceName")==0)
						{
							strncpy(daemon_local->flags.yvalue,value,sizeof(daemon_local->flags.yvalue));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232Baudrate")==0)
						{
							strncpy(daemon_local->flags.bvalue,value,sizeof(daemon_local->flags.bvalue));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"ECUId")==0)
						{
							strncpy(daemon_local->flags.evalue,value,sizeof(daemon_local->flags.evalue));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PersistanceStoragePath")==0)
						{
							strncpy(daemon_local->flags.ivalue,value,sizeof(daemon_local->flags.ivalue));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SharedMemorySize")==0)
						{
							daemon_local->flags.sharedMemorySize = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceDirectory")==0)
						{
							strncpy(daemon_local->flags.offlineTraceDirectory,value,sizeof(daemon_local->flags.offlineTraceDirectory));
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceFileSize")==0)
						{
							daemon_local->flags.offlineTraceFileSize = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceMaxSize")==0)
						{
							daemon_local->flags.offlineTraceMaxSize = atoi(value);
							printf("Option: %s=%s\n",token,value);
						}
						else
						{
							fprintf(stderr, "Unknown option: %s=%s\n",token,value);
						}
					}
					//printf ("Token: %s\n",pch);

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
	
    /* Initialize logging facility */
    dlt_log_init(daemon_local.flags.dflag);

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
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_init_p1()\n");
        return -1;
    }

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

#if 0
    /* open DLT output file */
    daemon_local->ohandle=-1;
    if (daemon_local->flags.ovalue[0])
    {
        daemon_local->ohandle = open(daemon_local->flags.ovalue,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */
        if (daemon_local->ohandle == -1)
        {
        	/* Return value ignored, dlt daemon will exit */
            dlt_file_free(&(daemon_local->file),daemon_local->flags.vflag);
            sprintf(str,"Output file %s cannot be opened!\n",daemon_local->flags.ovalue);
            dlt_log(LOG_ERR, str);
            return -1;
        } /* if */
    } /* if */
#endif
	/* init offline trace */
	if(((daemon->mode == DLT_USER_MODE_INTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH)) && daemon_local->flags.offlineTraceDirectory[0]) 
	{
		if (dlt_offline_trace_init(&(daemon_local->offlineTrace),daemon_local->flags.offlineTraceDirectory,daemon_local->flags.offlineTraceFileSize,daemon_local->flags.offlineTraceMaxSize)==-1)
		{
			dlt_log(LOG_ERR,"Could not initialize offline trace\n");
			return -1;
		}
	}
	
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

	/* init shared memory */
    if (dlt_shm_init_server(&(daemon_local->dlt_shm),DLT_SHM_KEY,daemon_local->flags.sharedMemorySize)==-1)
    {
    	dlt_log(LOG_ERR,"Could not initialize shared memory\n");
		return -1;
    }
	
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

	/* free shared memory */
	dlt_shm_free_server(&(daemon_local->dlt_shm));

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
    ret=dup(i); /* stderr */

    /* Set umask */
    umask(DLT_DAEMON_UMASK);

    /* Change to known directory */
    ret=chdir(DLT_USER_DIR);

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
		/* send ringbuffer done in old implementation */
		/* nothing to do with shared memory */
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
    if (dlt_receiver_receive_fd(&(daemon_local->receiver))<=0)
    {
        dlt_log(LOG_ERR, "dlt_receiver_receive_fd() for user messages failed!\n");
        return -1;
    }

    /* look through buffer as long as data is in there */
    do
    {
        if (daemon_local->receiver.bytesRcvd < sizeof(DltUserHeader))
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
        while ((sizeof(DltUserHeader)+offset)<=daemon_local->receiver.bytesRcvd);

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
        case DLT_USER_MESSAGE_LOG_SHM:
        {
            if (dlt_daemon_process_user_message_log(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
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

    if (daemon_local->receiver.bytesRcvd < (sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterApplication)))
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

    if (daemon_local->receiver.bytesRcvd < (sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)))
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
        if (msg.databuffer)
        {
            free(msg.databuffer);
        }
        msg.databuffer = (uint8_t *) malloc(msg.datasize);
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

    if (context->user_handle!=0)
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

    if (daemon_local->receiver.bytesRcvd < (sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterApplication)))
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

    if (daemon_local->receiver.bytesRcvd < (sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext)))
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
    //int bytes_to_be_removed;
    int j,sent,third_value;
    ssize_t ret;
    uint8_t rcv_buffer[10000];
    int size;

    static char text[DLT_DAEMON_TEXTSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_log()\n");
        return -1;
    }

	//dlt_shm_status(&(daemon_local->dlt_shm));
	while ( (size = dlt_shm_copy(&(daemon_local->dlt_shm),rcv_buffer,10000)) > 0)
    {
		if (dlt_message_read(&(daemon_local->msg),rcv_buffer,size,0,verbose)==0)
		{
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
					if (daemon_local->flags.yvalue!=0)
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
				/* dlt message was sent, remove from buffer */
				dlt_shm_remove(&(daemon_local->dlt_shm));
				
			}
			else
			{
				/* dlt message was not sent, keep in buffer */
				break;
			}
			
		}
		else
		{
			dlt_log(LOG_ERR,"Can't read messages from receiver\n");
			return -1;
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

    if (daemon_local->receiver.bytesRcvd < (sizeof(DltUserHeader)+sizeof(DltUserControlMsgAppLogLevelTraceStatus )))
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
                    if ((context->user_handle==0) ||
                            (dlt_daemon_user_send_log_level(daemon, context, verbose)!=0))
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

    if (daemon_local->receiver.bytesRcvd < (sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext)))
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

    if (missed > 0)
    {
        info->wakeups_missed += (missed - 1);
    }
}

/**
  \}
*/
