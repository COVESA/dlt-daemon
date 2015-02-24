/**
 * @licence app begin@
 * Copyright (C) 2012-2014  BMW AG
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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \file dlt-daemon.c
 * For further information see http://www.genivi.org/.
 * @licence end@
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

#ifdef linux
#include <sys/timerfd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#ifdef linux
#include <linux/stat.h>
#endif

#include "dlt_types.h"
#include "dlt-daemon.h"
#include "dlt-daemon_cfg.h"
#include "dlt_daemon_common_cfg.h"

#include "dlt_daemon_socket.h"
#include "dlt_daemon_serial.h"

#include "dlt_daemon_client.h"

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

static int dlt_daemon_log_internal(DltDaemon *daemon, DltDaemonLocal *daemon_local, char *str, int verbose);

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
static uint32_t watchdog_trigger_interval;  // watchdog trigger interval in [s]
#endif


/**
 * Print usage information of tool.
 */
void usage()
{
	char version[DLT_DAEMON_TEXTBUFSIZE];
	dlt_get_version(version,DLT_DAEMON_TEXTBUFSIZE);

    //printf("DLT logging daemon %s %s\n", _DLT_PACKAGE_VERSION, _DLT_PACKAGE_VERSION_STATE);
    //printf("Compile options: %s %s %s %s",_DLT_SYSTEMD_ENABLE, _DLT_SYSTEMD_WATCHDOG_ENABLE, _DLT_TEST_ENABLE, _DLT_SHM_ENABLE);
    printf("%s", version);
    printf("Usage: dlt-daemon [options]\n");
    printf("Options:\n");
    printf("  -d            Daemonize\n");
    printf("  -h            Usage\n");
    printf("  -c filename   DLT daemon configuration file (Default: " CONFIGURATION_FILES_DIR "/dlt.conf)\n");
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
            strncpy(daemon_local->flags.cvalue,optarg,NAME_MAX);
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
        int value_length = 1024;
        char line[value_length-1];
        char token[value_length];
        char value[value_length];
    char *pch;
    const char *filename;

	/* set default values for configuration */
	daemon_local->flags.sharedMemorySize = DLT_SHM_SIZE;
	daemon_local->flags.sendMessageTime = 0;
	daemon_local->flags.offlineTraceDirectory[0] = 0;
	daemon_local->flags.offlineTraceFileSize = 1000000;
	daemon_local->flags.offlineTraceMaxSize = 0;
	daemon_local->flags.loggingMode = DLT_LOG_TO_CONSOLE;
	daemon_local->flags.loggingLevel = LOG_INFO;
	strncpy(daemon_local->flags.loggingFilename, DLT_USER_DIR "/dlt.log",sizeof(daemon_local->flags.loggingFilename)-1);
	daemon_local->flags.loggingFilename[sizeof(daemon_local->flags.loggingFilename)-1]=0;
	daemon_local->timeoutOnSend = 4;
	daemon_local->RingbufferMinSize = DLT_DAEMON_RINGBUFFER_MIN_SIZE;
	daemon_local->RingbufferMaxSize = DLT_DAEMON_RINGBUFFER_MAX_SIZE;
	daemon_local->RingbufferStepSize = DLT_DAEMON_RINGBUFFER_STEP_SIZE;
	daemon_local->flags.sendECUSoftwareVersion = 0;
	memset(daemon_local->flags.pathToECUSoftwareVersion, 0, sizeof(daemon_local->flags.pathToECUSoftwareVersion));
	daemon_local->flags.sendTimezone = 0;

	/* open configuration file */
	if(daemon_local->flags.cvalue[0])
		filename = daemon_local->flags.cvalue;
	else
		filename = CONFIGURATION_FILES_DIR "/dlt.conf";
    //printf("Load configuration from file: %s\n",filename);
	pFile = fopen (filename,"r");

	if (pFile!=NULL)
	{
		while(1)
		{
			/* fetch line from configuration file */
                        if ( fgets (line , value_length - 1 , pFile) != NULL )
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
                        strncpy(token,pch,sizeof(token) - 1);
                        token[sizeof(token) - 1]=0;
					}
					else
					{
                        strncpy(value,pch,sizeof(value) - 1);
                        value[sizeof(value) - 1]=0;
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
                            strncpy(daemon_local->flags.yvalue,value,NAME_MAX);
                            daemon_local->flags.yvalue[NAME_MAX]=0;
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"RS232Baudrate")==0)
						{
                            strncpy(daemon_local->flags.bvalue,value,NAME_MAX);
                            daemon_local->flags.bvalue[NAME_MAX]=0;
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"ECUId")==0)
						{
                            strncpy(daemon_local->flags.evalue,value,NAME_MAX);
                            daemon_local->flags.evalue[NAME_MAX]=0;
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"PersistanceStoragePath")==0)
						{
                            strncpy(daemon_local->flags.ivalue,value,NAME_MAX);
                            daemon_local->flags.ivalue[NAME_MAX]=0;
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
                            strncpy(daemon_local->flags.loggingFilename,value,sizeof(daemon_local->flags.loggingFilename) - 1);
                            daemon_local->flags.loggingFilename[sizeof(daemon_local->flags.loggingFilename) - 1]=0;
							//printf("Option: %s=%s\n",token,value);
						}
                       	else if(strcmp(token,"TimeOutOnSend")==0)
						{
							daemon_local->timeoutOnSend = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
                       	else if(strcmp(token,"RingbufferMinSize")==0)
						{
							sscanf(value,"%lu",&(daemon_local->RingbufferMinSize));
						}
                       	else if(strcmp(token,"RingbufferMaxSize")==0)
						{
							sscanf(value,"%lu",&(daemon_local->RingbufferMaxSize));
						}
                       	else if(strcmp(token,"RingbufferStepSize")==0)
						{
							sscanf(value,"%lu",&(daemon_local->RingbufferStepSize));
						}
						else if(strcmp(token,"SharedMemorySize")==0)
						{
							daemon_local->flags.sharedMemorySize = atoi(value);
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"OfflineTraceDirectory")==0)
						{
                            strncpy(daemon_local->flags.offlineTraceDirectory,value,sizeof(daemon_local->flags.offlineTraceDirectory) - 1);
                            daemon_local->flags.offlineTraceDirectory[sizeof(daemon_local->flags.offlineTraceDirectory) - 1]=0;
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
                            strncpy(daemon_local->flags.pathToECUSoftwareVersion,value,sizeof(daemon_local->flags.pathToECUSoftwareVersion) - 1);
                            daemon_local->flags.pathToECUSoftwareVersion[sizeof(daemon_local->flags.pathToECUSoftwareVersion) - 1]=0;
							//printf("Option: %s=%s\n",token,value);
						}
						else if(strcmp(token,"SendTimezone")==0)
						{
							daemon_local->flags.sendTimezone = atoi(value);
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
    dlt_get_version(version,DLT_DAEMON_TEXTBUFSIZE);

    snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Starting DLT Daemon; %s\n", version );
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

    // create fd for watchdog
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    {
        char* watchdogUSec = getenv("WATCHDOG_USEC");
        int watchdogTimeoutSeconds = 0;

        dlt_log(LOG_DEBUG, "Systemd watchdog initialization\n");
        if( watchdogUSec )
        {
            watchdogTimeoutSeconds = atoi(watchdogUSec)/2000000;
        }
        watchdog_trigger_interval = watchdogTimeoutSeconds;
        create_timer_fd(&daemon_local, watchdogTimeoutSeconds, watchdogTimeoutSeconds, &daemon_local.timer_wd, "Systemd watchdog");
    }
#endif

    // create fd for timer timing packets
    create_timer_fd(&daemon_local, 1, 1, &daemon_local.timer_one_s, "Timing packet");

    // create fd for timer ecu version
    if(daemon_local.flags.sendECUSoftwareVersion > 0 || daemon_local.flags.sendTimezone > 0)
    {
        //dlt_daemon_init_ecuversion(&daemon_local);
        create_timer_fd(&daemon_local, 60, 60, &daemon_local.timer_sixty_s, "ECU version");
    }

	// For offline tracing we still can use the same states 
	// as for socket sending. Using this trick we see the traces 
	// In the offline trace AND in the socket stream.
    if(daemon_local.flags.yvalue[0])
    	dlt_daemon_change_state(&daemon,DLT_DAEMON_STATE_SEND_DIRECT);
    else
		dlt_daemon_change_state(&daemon,DLT_DAEMON_STATE_BUFFER);

    dlt_daemon_log_internal(&daemon, &daemon_local, "Daemon launched. Starting to output traces...", daemon_local.flags.vflag);

    while (1)
    {

        /* wait for events from all FIFO and sockets */
        daemon_local.read_fds = daemon_local.master;
        if (select(daemon_local.fdmax+1, &(daemon_local.read_fds), NULL, NULL, NULL) == -1)
        {
            int error = errno;
            /* retry if SIGINT was received, else error out */
            if ( error != EINTR ) {
                snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"select() failed: %s\n", strerror(error) );
                dlt_log(LOG_ERR, str);
                return -1;
            }
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
                    	dlt_log(LOG_ERR,"Connect to dlt client failed!\n");
                        return -1;
                    }
                }
                else if (i == daemon_local.fp)
                {
                    /* event from the FIFO happened */
                    if (dlt_daemon_process_user_messages(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_WARNING,"Processing of messages from user connection failed!\n");
                        return -1;
                    }
                }
                else if ((i == daemon_local.fdserial) && (daemon_local.flags.yvalue[0]))
                {
                    /* event from serial connection to client received */
                    if (dlt_daemon_process_client_messages_serial(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_WARNING,"Processing of messages from serial connection failed!\n");
                        return -1;
                    }
                }
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
                else if (i == daemon_local.timer_wd)
                {
                    uint64_t expir=0;
                    ssize_t res = read(daemon_local.timer_wd, &expir, sizeof(expir));
                    if(res < 0) {
                        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Failed to read timer_wd; %s\n", strerror(errno) );
                        dlt_log(LOG_WARNING, str);
                        // Activity received on timer_wd, but unable to read the fd:
                        // let's go on sending notification
                    }

                    dlt_log(LOG_DEBUG, "Timer watchdog\n");

                    if(sd_notify(0, "WATCHDOG=1") < 0)
                    {
                        dlt_log(LOG_WARNING, "Could not reset systemd watchdog\n");
                    }
                }
#endif
                else if (i == daemon_local.timer_one_s)
                {
                    uint64_t expir=0;
                    ssize_t res = read(daemon_local.timer_one_s, &expir, sizeof(expir));
                    if(res < 0) {
                        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Failed to read timer_timingpacket; %s\n", strerror(errno) );
                        dlt_log(LOG_WARNING, str);
                        // Activity received on timer_wd, but unable to read the fd:
                        // let's go on sending notification
                    }
                    if(daemon.state == DLT_DAEMON_STATE_SEND_BUFFER || daemon.state == DLT_DAEMON_STATE_BUFFER_FULL)
                    {
						if (dlt_daemon_send_ringbuffer_to_client(&daemon, &daemon_local, daemon_local.flags.vflag))
						{
							dlt_log(LOG_DEBUG,"Can't send contents of ringbuffer to clients\n");
						}
                    }
                    if (daemon.timingpackets && daemon.state == DLT_DAEMON_STATE_SEND_DIRECT)
                    {
                    	dlt_daemon_control_message_time(DLT_DAEMON_SEND_TO_ALL, &daemon, &daemon_local, daemon_local.flags.vflag);
                    }
                    dlt_log(LOG_DEBUG, "Timer timingpacket\n");

                 }

                else if (i == daemon_local.timer_sixty_s)
                {
                    uint64_t expir=0;
                    ssize_t res = read(daemon_local.timer_sixty_s, &expir, sizeof(expir));
                    if(res < 0) {
                        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Failed to read timer_ecuversion; %s\n", strerror(errno) );
                        dlt_log(LOG_WARNING, str);
                        // Activity received on timer_wd, but unable to read the fd:
                        // let's go on sending notification
                    }
                	if(daemon_local.flags.sendECUSoftwareVersion > 0)
                		dlt_daemon_control_get_software_version(DLT_DAEMON_SEND_TO_ALL, &daemon,&daemon_local, daemon_local.flags.vflag);

                	if(daemon_local.flags.sendTimezone > 0)
                	{
                		dlt_daemon_control_message_timezone(DLT_DAEMON_SEND_TO_ALL,&daemon,&daemon_local,daemon_local.flags.vflag);
                	}
                    dlt_log(LOG_DEBUG, "Timer ecuversion\n");

                }
                else
                {
                    /* event from tcp connection to client received */
                    daemon_local.receiverSock.fd = i;
                    if (dlt_daemon_process_client_messages(&daemon, &daemon_local, daemon_local.flags.vflag)==-1)
                    {
                    	dlt_log(LOG_WARNING,"Processing of messages from client connection failed!\n");
						return -1;
                    }
                } /* else */
            } /* if */
        } /* for */
    } /* while */

    dlt_daemon_log_internal(&daemon, &daemon_local, "Exiting Daemon...", daemon_local.flags.vflag);

    dlt_daemon_local_cleanup(&daemon, &daemon_local, daemon_local.flags.vflag);

    dlt_log(LOG_NOTICE, "Leaving DLT daemon\n");

    return 0;

} /* main() */

int dlt_daemon_local_init_p1(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	int ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_local_init_p1()\n");
        return -1;
    }

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
    ret = sd_booted();

    if(ret == 0){
    	dlt_log(LOG_CRIT, "System not booted with systemd!\n");
//    	return -1;
    }
    else if(ret < 0)
    {
    	dlt_log(LOG_CRIT, "sd_booted failed!\n");
    	return -1;
    }
    else
    {
    	dlt_log(LOG_INFO, "System booted with systemd\n");
    }
#endif

    /* create dlt pipes directory */
    ret=mkdir(DLT_USER_DIR, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH  | S_IWOTH | S_ISVTX );
    if (ret==-1 && errno != EEXIST)
    {
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"FIFO user dir %s cannot be created (%s)!\n", DLT_USER_DIR, strerror(errno));
        dlt_log(LOG_WARNING, str);
        return -1;
    }

    // S_ISGID cannot be set by mkdir, let's reassign right bits
    ret=chmod(DLT_USER_DIR, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH  | S_IWOTH | S_IXOTH | S_ISGID | S_ISVTX );
    if (ret==-1)
    {
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"FIFO user dir %s cannot be chmoded (%s)!\n", DLT_USER_DIR, strerror(errno));
        dlt_log(LOG_WARNING, str);
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
    if (dlt_daemon_init(daemon,daemon_local->RingbufferMinSize,daemon_local->RingbufferMaxSize,daemon_local->RingbufferStepSize,daemon_local->flags.ivalue,daemon_local->flags.vflag)==-1)
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

    /* Get ECU version info from a file. If it fails, use dlt_version as fallback. */
    if(dlt_daemon_local_ecu_version_init(daemon, daemon_local, daemon_local->flags.vflag) < 0)
    {
    	daemon->ECUVersionString = malloc(DLT_DAEMON_TEXTBUFSIZE);
    	if(daemon->ECUVersionString==0)
        {
        	dlt_log(LOG_WARNING,"Could not allocate memory for version string\n");
            return -1;
        }
        dlt_get_version(daemon->ECUVersionString,DLT_DAEMON_TEXTBUFSIZE);
    }

    return 0;
}

int dlt_daemon_local_connection_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    int ret;

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

    ret=mkfifo(DLT_USER_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
    if (ret==-1)
    {
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"FIFO user %s cannot be created (%s)!\n",DLT_USER_FIFO, strerror(errno));
        dlt_log(LOG_WARNING, str);
        return -1;
    } /* if */

    daemon_local->fp = open(DLT_USER_FIFO, O_RDWR);
    if (daemon_local->fp==-1)
    {
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"FIFO user %s cannot be opened (%s)!\n",DLT_USER_FIFO, strerror(errno));
        dlt_log(LOG_WARNING, str);
        return -1;
    } /* if */

    /* create and open socket to receive incoming connections from client */
    if(dlt_daemon_socket_open(&(daemon_local->sock)))
    	return -1;

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
            snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Failed to open serial device %s\n", daemon_local->flags.yvalue);
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
                snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Failed to configure serial device %s (%s) \n", daemon_local->flags.yvalue, strerror(errno));
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
                dlt_log(LOG_DEBUG, "Serial init done\n");
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

int dlt_daemon_local_ecu_version_init(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	char *version	= NULL;
	FILE *f 		= NULL;

	PRINT_FUNCTION_VERBOSE(verbose);

	/* By default, version string is null. */
	daemon->ECUVersionString = NULL;

	/* Open the file. Bail out if error occurs */
	f = fopen(daemon_local->flags.pathToECUSoftwareVersion, "r");
	if(f == NULL)
	{
		/* Error level notice, because this might be deliberate choice */
		dlt_log(LOG_NOTICE, "Failed to open ECU Software version file.\n");
		return -1;
	}

	/* Get the file size. Bail out if stat fails. */
	int fd = fileno(f);
	struct stat s_buf;
	if(fstat(fd, &s_buf) < 0)
	{
		dlt_log(LOG_WARNING, "Failed to stat ECU Software version file.\n");
		fclose(f);
		return -1;
	}

	/* Bail out if file is too large. Use DLT_DAEMON_TEXTBUFSIZE max.
	 * Reserve one byte for trailing '\0' */
	off_t size = s_buf.st_size;
	if(size >= DLT_DAEMON_TEXTBUFSIZE)
	{
		dlt_log(LOG_WARNING, "Too large file for ECU version.\n");
		fclose(f);
		return -1;
	}

	/* Allocate permanent buffer for version info */
	version = malloc(size + 1);
	if(version==0)
	{
		dlt_log(LOG_WARNING, "Cannot allocate memory for ECU version.\n");
		fclose(f);
		return -1;
	}
	off_t offset = 0;
	while(!feof(f))
	{
		offset += fread(version + offset, 1, size, f);
		if(ferror(f))
		{
			dlt_log(LOG_WARNING, "Failed to read ECU Software version file.\n");
			free(version);
            fclose(f);
			return -1;
		}
        if(offset > size)
		{
			dlt_log(LOG_WARNING, "Too long file for ECU Software version info.\n");
			free(version);
			fclose(f);
			return -1;
		}
	}
	version[offset] = '\0';//append null termination at end of version string
	daemon->ECUVersionString = version;
	fclose(f);
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
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Exiting DLT daemon due to signal: %s\n", strsignal(sig) );
        dlt_log(LOG_NOTICE, str);

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
        dlt_log(LOG_CRIT, "This case should never happen!");
	break;
    }
    } /* switch */
} /* dlt_daemon_signal_handler() */

void dlt_daemon_daemonize(int verbose)
{
    int i,lfp;
    ssize_t pid_len;

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
    if (-1 < i)
	{
    	if(dup(i) < 0)
    	    dlt_log(LOG_WARNING, "Failed to direct stdout to /dev/null.\n");/* stdout */
    	if(dup(i) < 0)
    	    dlt_log(LOG_WARNING, "Failed to direct stderr to /dev/null.\n"); /* stderr */
	}

    /* Set umask */
    umask(DLT_DAEMON_UMASK);

    /* Change to known directory */
    if(chdir(DLT_USER_DIR) < 0)
        dlt_log(LOG_WARNING, "Failed to chdir to DLT_USER_DIR.\n");;

    /* Ensure single copy of daemon;
       run only one instance at a time */
    lfp=open(DLT_DAEMON_LOCK_FILE,O_RDWR|O_CREAT,DLT_DAEMON_LOCK_FILE_PERM);
    if (lfp<0)
    {
    	dlt_log(LOG_CRIT, "Can't open lock file, exiting DLT daemon\n");
        exit(-1); /* can not open */
    }
    if (lockf(lfp,F_TLOCK,0)<0)
    {
    	dlt_log(LOG_CRIT, "Can't lock lock file, exiting DLT daemon\n");
        exit(-1); /* can not lock */
    }
    /* only first instance continues */

    snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"%d\n",getpid());
    pid_len = strlen(str);
    if(write(lfp,str,pid_len) != pid_len) /* record pid to lockfile */
        dlt_log(LOG_WARNING, "Could not write pid to file in dlt_daemon_daemonize.\n");

    /* Catch signals */
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);

} /* dlt_daemon_daemonize() */

/* This function logs str to the configured output sink (socket, serial, offline trace).
   To avoid recursion this function must be called only from DLT highlevel functions.   
   E. g. calling it to output a failure when the open of the offline trace file fails
   would cause an endless loop because dlt_daemon_log_internal() would itself again try
   to open the offline trace file. 
   This is a dlt-daemon only function. The libdlt has no equivalent function available. */
int dlt_daemon_log_internal(DltDaemon *daemon, DltDaemonLocal *daemon_local, char *str, int verbose)
{
    DltMessage msg;
    static uint8_t uiMsgCount = 0;
    DltStandardHeaderExtra *pStandardExtra;
    uint32_t uiType;
    uint16_t uiSize;
    uint32_t uiExtraSize;
    int sent;
    int j;
    int third_value;
    int ret;
    
    PRINT_FUNCTION_VERBOSE(verbose);

    // Set storageheader
    msg.storageheader = (DltStorageHeader *)(msg.headerbuffer);
    dlt_set_storageheader(msg.storageheader, daemon->ecuid);

    // Set standardheader    
    msg.standardheader = (DltStandardHeader *)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_UEH | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS | DLT_HTYP_PROTOCOL_VERSION1;
    msg.standardheader->mcnt = uiMsgCount++;
    
    uiExtraSize = DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp)+(DLT_IS_HTYP_UEH(msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + uiExtraSize;

    // Set extraheader
    pStandardExtra = (DltStandardHeaderExtra *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader));
    dlt_set_id(pStandardExtra->ecu, daemon->ecuid);
    pStandardExtra->tmsp = DLT_HTOBE_32(dlt_uptime());
    pStandardExtra->seid = DLT_HTOBE_32(getpid());

    // Set extendedheader
    msg.extendedheader = (DltExtendedHeader *)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_VERB | (DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) | ((DLT_LOG_INFO << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN);
    msg.extendedheader->noar = 1;
    dlt_set_id(msg.extendedheader->apid, "DLTD");
    dlt_set_id(msg.extendedheader->ctid, "INTM");

    // Set payload data...
    uiType = DLT_TYPE_INFO_STRG;
    uiSize = strlen(str) + 1;
    msg.datasize = sizeof(uint32_t) + sizeof(uint16_t) + uiSize;

	msg.databuffer = (uint8_t *) malloc(msg.datasize);
	msg.databuffersize = msg.datasize;
    if (msg.databuffer==0)
    {
    	dlt_log(LOG_WARNING,"Can't allocate buffer for get log info message\n");
		return -1;
    }

    msg.datasize = 0;
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiType), sizeof(uint32_t));
    msg.datasize += sizeof(uint32_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), (uint8_t *)(&uiSize), sizeof(uint16_t));
    msg.datasize += sizeof(uint16_t);
    memcpy((uint8_t *)(msg.databuffer + msg.datasize), str, uiSize);
    msg.datasize += uiSize;

    // Calc lengths
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    // Sending data...
    {
		/* check if overflow occurred */
		if(daemon->overflow_counter)
		{
			if(dlt_daemon_send_message_overflow(daemon,daemon_local,verbose)==0)
			{
				sprintf(str,"%u messages discarded!\n",daemon->overflow_counter);
					dlt_log(LOG_WARNING, str);
				daemon->overflow_counter=0;
			}
		}

		/* look if TCP connection to client is available */
		if((daemon->mode == DLT_USER_MODE_EXTERNAL) || (daemon->mode == DLT_USER_MODE_BOTH))
		{

			if((ret = dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL,daemon,daemon_local,msg.headerbuffer,sizeof(DltStorageHeader),msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader),
								msg.databuffer,msg.datasize,verbose)))
			{
				if(ret == DLT_DAEMON_ERROR_BUFFER_FULL)
				{
					daemon->overflow_counter++;
				}
			}
		}
    }

    free(msg.databuffer);
    
    return 0;
}

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

    /* Set socket timeout in reception */
    struct timeval timeout_send;
    timeout_send.tv_sec = daemon_local->timeoutOnSend;
    timeout_send.tv_usec = 0;
    if (setsockopt (in_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout_send, sizeof(timeout_send)) < 0)
        dlt_log(LOG_WARNING, "setsockopt failed\n");

    /* Set to non blocking mode */
    //flags = fcntl(in_sock, F_GETFL, 0);
    //fcntl(in_sock, F_SETFL, flags | O_NONBLOCK);

    //snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"Client Connection from %s\n", inet_ntoa(cli.sin_addr));
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
        snprintf(str,DLT_DAEMON_TEXTBUFSIZE, "New connection to client established, #connections: %d\n",daemon_local->client_connections);
        dlt_log(LOG_INFO, str);
    }

    // send connection info about connected
    dlt_daemon_control_message_connection_info(in_sock,daemon,daemon_local,DLT_CONNECTION_STATUS_CONNECTED,"",verbose);

    // send ecu version string
    if(daemon_local->flags.sendECUSoftwareVersion > 0)
    {
    	if(daemon_local->flags.sendECUSoftwareVersion > 0)
    		dlt_daemon_control_get_software_version(DLT_DAEMON_SEND_TO_ALL, daemon,daemon_local, daemon_local->flags.vflag);

    	if(daemon_local->flags.sendTimezone > 0)
    	{
    		dlt_daemon_control_message_timezone(DLT_DAEMON_SEND_TO_ALL,daemon,daemon_local,daemon_local->flags.vflag);
    	}
    }

    if (daemon_local->client_connections==1)
    {
        if (daemon_local->flags.vflag)
        {
            dlt_log(LOG_DEBUG, "Send ring-buffer to client\n");
        }
    	dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_BUFFER);
        if (dlt_daemon_send_ringbuffer_to_client(daemon, daemon_local, verbose)==-1)
        {
        	dlt_log(LOG_WARNING,"Can't send contents of ringbuffer to clients\n");
			return -1;
        }
		
		/* send new log state to all applications */
		daemon->connectionState = 1;
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
    	dlt_daemon_close_socket(daemon_local->receiverSock.fd, daemon, daemon_local, verbose);
    	daemon_local->receiverSock.fd = -1;
        /* check: return 0; */
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),(uint8_t*)daemon_local->receiverSock.buf,daemon_local->receiverSock.bytesRcvd,daemon_local->flags.nflag,daemon_local->flags.vflag)==DLT_MESSAGE_ERROR_OK)
    {
        /* Check for control message */
        if ( 0 < daemon_local->receiverSock.fd && DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)) )
        {
        	dlt_daemon_client_process_control(daemon_local->receiverSock.fd, daemon,daemon_local, &(daemon_local->msg), daemon_local->flags.vflag);
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
        	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for sockets\n");
			return -1;
        }

    } /* while */


    if (dlt_receiver_move_to_begin(&(daemon_local->receiverSock))==-1)
    {
    	dlt_log(LOG_WARNING,"Can't move bytes to beginning of receiver buffer for sockets\n");
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
		dlt_log(LOG_WARNING, "dlt_receiver_receive_fd() for messages from serial interface failed!\n");
        return -1;
    }

    /* Process all received messages */
    while (dlt_message_read(&(daemon_local->msg),(uint8_t*)daemon_local->receiverSerial.buf,daemon_local->receiverSerial.bytesRcvd,daemon_local->flags.mflag,daemon_local->flags.vflag)==DLT_MESSAGE_ERROR_OK)
    {
        /* Check for control message */
        if (DLT_MSG_IS_CONTROL_REQUEST(&(daemon_local->msg)))
        {
            if (dlt_daemon_client_process_control(daemon_local->receiverSerial.fd, daemon,daemon_local, &(daemon_local->msg), daemon_local->flags.vflag)==-1)
            {
				dlt_log(LOG_WARNING,"Can't process control messages\n");
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
        	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for serial connection\n");
			return -1;
        }

    } /* while */


    if (dlt_receiver_move_to_begin(&(daemon_local->receiverSerial))==-1)
	{
    	dlt_log(LOG_WARNING,"Can't move bytes to beginning of receiver buffer for serial connection\n");
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
        dlt_log(LOG_WARNING, "dlt_receiver_receive_fd() for user messages failed!\n");
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
        case DLT_USER_MESSAGE_MARKER:
        {
            if (dlt_daemon_process_user_message_marker(daemon, daemon_local, daemon_local->flags.vflag)==-1)
            {
                run_loop=0;
            }
            break;
        }
        default:
        {
            snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Invalid user message type received: %d!\n", userheader->message);
            dlt_log(LOG_ERR,str);

            /* remove user header */
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader))==-1)
            {
				dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user messages\n");
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
    	dlt_log(LOG_WARNING,"Can't move bytes to beginning of receiver buffer for user messages\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	int ret;
    DltUserControlMsgBufferOverflow *userpayload;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_WARNING, "Invalid function parameters used for function dlt_daemon_process_user_message_overflow()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgBufferOverflow)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    /* get the payload of the user message */
    userpayload = (DltUserControlMsgBufferOverflow*) (daemon_local->receiver.buf+sizeof(DltUserHeader));

    /* Store in daemon, that a message buffer overflow has occured */
    /* look if TCP connection to client is available or it least message can be put into buffer */
	if((ret=dlt_daemon_control_message_buffer_overflow(DLT_DAEMON_SEND_TO_ALL, daemon,daemon_local, userpayload->overflow_counter,userpayload->apid,verbose)))
	{
    	/* there was an error when storing message */
    	/* add the counter of lost messages to the daemon counter */
    	daemon->overflow_counter+=userpayload->overflow_counter;
	}

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgBufferOverflow))==-1)
    {
		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message overflow\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_send_message_overflow(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	int ret;
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_overflow()\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    /* Store in daemon, that a message buffer overflow has occured */
	if((ret=dlt_daemon_control_message_buffer_overflow(DLT_DAEMON_SEND_TO_ALL, daemon,daemon_local,daemon->overflow_counter,"", verbose)))
	{
		return ret;
	}

    return DLT_DAEMON_ERROR_OK;
}

int dlt_daemon_process_user_message_register_application(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    uint32_t len=0;
    DltDaemonApplication *application;
    char description[DLT_DAEMON_DESCSIZE+1];
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
        description[sizeof(description)-1]=0;

    }

    application=dlt_daemon_application_add(daemon,usercontext->apid,usercontext->pid,description,verbose);

	/* send log state to new application */
	dlt_daemon_user_send_log_state(daemon,application,verbose);

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterApplication)+len)==-1)
	{
		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register application\n");
		return -1;
    }

    if (application==0)
    {
    	snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Can't add ApplicationID '%.4s' for PID %d\n", usercontext->apid, usercontext->pid);
    	dlt_log(LOG_WARNING,str);
        return -1;
    } else
    {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "ApplicationID '%.4s' registered for PID %d, Description=%s\n", application->apid, application->pid, application->application_description);
        dlt_daemon_log_internal(daemon, daemon_local, str, daemon_local->flags.vflag);
    	dlt_log(LOG_DEBUG,str);
    }

    return 0;
}

int dlt_daemon_process_user_message_register_context(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    uint32_t len=0;
    int8_t loglevel, tracestatus;
    DltUserControlMsgRegisterContext *usercontext;
    char description[DLT_DAEMON_DESCSIZE+1];
	DltDaemonApplication *application;
	DltDaemonContext *context;
	DltServiceGetLogInfoRequest *req;

	DltMessage msg;

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
        description[sizeof(description)-1]=0;
    }

    application = dlt_daemon_application_find(daemon,usercontext->apid,verbose);

    if (application==0)
    {
        snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "ApplicationID '%.4s' not found while registering ContextID '%.4s' in dlt_daemon_process_user_message_register_context()\n", usercontext->apid, usercontext->ctid);
        dlt_log(LOG_WARNING, str);
        if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
		{
			dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
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
				dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
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
				dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
			}
            return -1;
        }
    }

    context = dlt_daemon_context_add(daemon,usercontext->apid,usercontext->ctid, loglevel, tracestatus, usercontext->log_level_pos,application->user_handle,description,verbose);

    if (context==0)
    {
        if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
		{
			dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
		}

		snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Can't add ContextID '%.4s' for ApplicationID '%.4s'\n", usercontext->ctid, usercontext->apid);
		dlt_log(LOG_WARNING,str);
        return -1;
    } else
    {
		snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "ContextID '%.4s' registered for ApplicationID '%.4s', Description=%s\n", context->ctid, context->apid, context->context_description);
        dlt_daemon_log_internal(daemon, daemon_local, str, daemon_local->flags.vflag);
		dlt_log(LOG_DEBUG,str);
    }
    /* Create automatic get log info response for registered context */
    if (daemon_local->flags.rflag)
    {
        /* Prepare request for get log info with one application and one context */
        if (dlt_message_init(&msg, verbose)==-1)
        {
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
        	{
        		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
        		return -1;
        	}
        	dlt_log(LOG_WARNING,"Can't initialize message");
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
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
        	{
        		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
        		return -1;
        	}
        	dlt_log(LOG_WARNING,"Can't allocate buffer for get log info message\n");
			return -1;
        }

        req = (DltServiceGetLogInfoRequest*) msg.databuffer;

        req->service_id = DLT_SERVICE_ID_GET_LOG_INFO;
        req->options = 7;
        dlt_set_id(req->apid, usercontext->apid);
        dlt_set_id(req->ctid, usercontext->ctid);
        dlt_set_id(req->com,"remo");

        dlt_daemon_control_get_log_info(DLT_DAEMON_SEND_TO_ALL , daemon,daemon_local, &msg, verbose);

        dlt_message_free(&msg, verbose);
    }

    if (context->user_handle >= DLT_FD_MINIMUM)
    {
        /* This call also replaces the default values with the values defined for default */
        if (dlt_daemon_user_send_log_level(daemon, context, verbose)==-1)
        {
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
        	{
        		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
        		return -1;
        	}
			snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Can't send current log level as response to user message register context for (%.4s;%.4s)\n", context->apid, context->ctid);
			dlt_log(LOG_WARNING,str);
			return -1;
        }
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgRegisterContext)+len)==-1)
	{
		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message register context\n");
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
                        if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterApplication))==-1)
                        {
                        	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message unregister application\n");
                    		return -1;
                        }
                    	snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Can't delete ContextID '%.4s' in ApplicationID '%.4s' for user message unregister application\n", context->ctid, context->apid);
                    	dlt_log(LOG_WARNING,str);
                    	return -1;
                    }
                }
            }

            /* Delete this application entry from internal table*/
            if (dlt_daemon_application_del(daemon, application, verbose)==-1)
            {
                if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterApplication))==-1)
                {
                	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message unregister application\n");
            		return -1;
                }
            	snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Can't delete ApplicationID '%.4s' for user message unregister application\n", application->apid);
            	dlt_log(LOG_WARNING,str);
				return -1;
            } else
            {
            	snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Unregistered ApplicationID '%.4s'\n", usercontext->apid);
                dlt_daemon_log_internal(daemon, daemon_local, str, daemon_local->flags.vflag);
            	dlt_log(LOG_DEBUG,str);
            }
        }
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterApplication))==-1)
    {
    	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message unregister application\n");
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
            if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext))==-1)
            {
            	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message unregister context\n");
        		return -1;
            }
        	snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Can't delete ContextID '%.4s' for ApplicationID '%.4s' for user message unregister context\n", usercontext->ctid, usercontext->apid);
			dlt_log(LOG_WARNING,str);
			return -1;
		} else
		{
        	snprintf(str, DLT_DAEMON_TEXTBUFSIZE, "Unregistered ContextID '%.4s' for ApplicationID '%.4s'\n", usercontext->ctid, usercontext->apid);
            dlt_daemon_log_internal(daemon, daemon_local, str, daemon_local->flags.vflag);
        	dlt_log(LOG_DEBUG,str);
		}
    }

    /* Create automatic unregister context response for unregistered context */
    if (daemon_local->flags.rflag)
    {
		dlt_daemon_control_message_unregister_context(DLT_DAEMON_SEND_TO_ALL,daemon,daemon_local,usercontext->apid, usercontext->ctid, "remo",verbose);
    }

    /* keep not read data in buffer */
    if (dlt_receiver_remove(&(daemon_local->receiver),sizeof(DltUserHeader)+sizeof(DltUserControlMsgUnregisterContext))==-1)
    {
    	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message unregister context\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_log(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	int ret;
    int bytes_to_be_removed;

    static char text[DLT_DAEMON_TEXTSIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_log()\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    ret=dlt_message_read(&(daemon_local->msg),(unsigned char*)daemon_local->receiver.buf+sizeof(DltUserHeader),daemon_local->receiver.bytesRcvd-sizeof(DltUserHeader),0,verbose);
    if(ret!=DLT_MESSAGE_ERROR_OK)
    {
    	if(ret!=DLT_MESSAGE_ERROR_SIZE)
    	{
            /* This is a normal usecase: The daemon reads the data in 10kb chunks. 
               Thus the last trace in this chunk is probably not complete and will be completed
               with the next chunk read. This happens always when the FIFO is filled with more than 10kb before
               the daemon is able to read from the FIFO. 
               Thus the loglevel of this message is set to DEBUG. 
               A cleaner solution would be to check more in detail whether the message is not complete (normal usecase) 
               or the headers are corrupted (error case). */
    		dlt_log(LOG_DEBUG,"Can't read messages from receiver\n");
    	}
		return DLT_DAEMON_ERROR_UNKNOWN;
	}

	/* set overwrite ecu id */
	if (daemon_local->flags.evalue!=0)
	{
		/* Set header extra parameters */
		dlt_set_id(daemon_local->msg.headerextra.ecu, daemon->ecuid );
		//msg.headerextra.seid = 0;
		if (dlt_message_set_extraparameters(&(daemon_local->msg),0)==-1)
		{
            dlt_log(LOG_WARNING,"Can't set message extra parameters in process user message log\n");
			return DLT_DAEMON_ERROR_UNKNOWN;
		}

		/* Correct value of timestamp, this was changed by dlt_message_set_extraparameters() */
		daemon_local->msg.headerextra.tmsp = DLT_BETOH_32(daemon_local->msg.headerextra.tmsp);
	}

	/* prepare storage header */
	if (DLT_IS_HTYP_WEID(daemon_local->msg.standardheader->htyp))
	{
		if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon_local->msg.headerextra.ecu)==-1)
		{
			dlt_log(LOG_WARNING,"Can't set storage header in process user message log\n");
			return DLT_DAEMON_ERROR_UNKNOWN;
		}
	}
	else
	{
		if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon->ecuid)==-1)
		{
			dlt_log(LOG_WARNING,"Can't set storage header in process user message log\n");
			return DLT_DAEMON_ERROR_UNKNOWN;
		}
	}

	{
		/* if no filter set or filter is matching display message */
		if (daemon_local->flags.xflag)
		{
			if (dlt_message_print_hex(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_WARNING,"dlt_message_print_hex() failed!\n");
			}
		} /*  if */
		else if (daemon_local->flags.aflag)
		{
			if (dlt_message_print_ascii(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_WARNING,"dlt_message_print_ascii() failed!\n");
			}
		} /* if */
		else if (daemon_local->flags.sflag)
		{
			if (dlt_message_print_header(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_WARNING,"dlt_message_print_header() failed!\n");
			}
			/* print message header only */
		} /* if */


		/* check if overflow occurred */
		if(daemon->overflow_counter)
		{
			if(dlt_daemon_send_message_overflow(daemon,daemon_local,verbose)==0)
			{
				snprintf(str,DLT_DAEMON_TEXTBUFSIZE,"%u messages discarded!\n",daemon->overflow_counter);
				dlt_log(LOG_WARNING, str);
				daemon->overflow_counter=0;
			}
		}

		/* send message to client or write to log file */
		if((ret = dlt_daemon_client_send(DLT_DAEMON_SEND_TO_ALL,daemon,daemon_local,daemon_local->msg.headerbuffer,sizeof(DltStorageHeader),daemon_local->msg.headerbuffer+sizeof(DltStorageHeader),daemon_local->msg.headersize-sizeof(DltStorageHeader),
							daemon_local->msg.databuffer,daemon_local->msg.datasize,verbose)))
		{
			if(ret == DLT_DAEMON_ERROR_BUFFER_FULL)
			{
				daemon->overflow_counter++;
			}
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
		dlt_log(LOG_WARNING,"Can't remove bytes from receiver\n");
		return DLT_DAEMON_ERROR_UNKNOWN;
	}

    return DLT_DAEMON_ERROR_OK;
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
			dlt_log(LOG_WARNING,"Can't read messages from shm\n");
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
				dlt_log(LOG_WARNING,"Can't set message extra parameters in process user message log\n");
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
				dlt_log(LOG_WARNING,"Can't set storage header in process user message log\n");
				dlt_shm_remove(&(daemon_local->dlt_shm));
				return -1;
			}
		}
		else
		{
			if (dlt_set_storageheader(daemon_local->msg.storageheader,daemon->ecuid)==-1)
			{
				dlt_log(LOG_WARNING,"Can't set storage header in process user message log\n");
				dlt_shm_remove(&(daemon_local->dlt_shm));
				return -1;
			}
		}

		/* display message */
		if (daemon_local->flags.xflag)
		{
			if (dlt_message_print_hex(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_WARNING,"dlt_message_print_hex() failed!\n");
			}
		} /*  if */
		else if (daemon_local->flags.aflag)
		{
			if (dlt_message_print_ascii(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_WARNING,"dlt_message_print_ascii() failed!\n");
			}
		} /* if */
		else if (daemon_local->flags.sflag)
		{
			if (dlt_message_print_header(&(daemon_local->msg),text,DLT_DAEMON_TEXTSIZE,verbose)==-1)
			{
				dlt_log(LOG_WARNING,"dlt_message_print_header() failed!\n");
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
		dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message overflow\n");
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
		dlt_log(LOG_WARNING,"Can't remove bytes from receiver\n");
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

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgLogMode)))
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
    	dlt_log(LOG_WARNING,"Can't remove bytes from receiver for user message log mode\n");
		return -1;
    }

    return 0;
}

int dlt_daemon_process_user_message_marker(DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_process_user_message_marker()\n");
        return -1;
    }

    if (daemon_local->receiver.bytesRcvd < (int32_t)(sizeof(DltUserHeader)))
    {
    	/* Not enough bytes received */
        return -1;
    }

    /* Create automatic unregister context response for unregistered context */
    dlt_daemon_control_message_marker(DLT_DAEMON_SEND_TO_ALL,daemon,daemon_local,verbose);

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
	int ret;
    static uint8_t data[DLT_DAEMON_RCVBUFSIZE];
    int length;
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    uint32_t curr_time;
#endif

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0)  || (daemon_local==0))
    {
    	dlt_log(LOG_ERR, "Invalid function parameters used for function dlt_daemon_send_ringbuffer_to_client()\n");
        return DLT_DAEMON_ERROR_UNKNOWN;
    }

    if(dlt_buffer_get_message_count(&(daemon->client_ringbuffer)) <= 0)
    {
    	dlt_daemon_change_state(daemon, DLT_DAEMON_STATE_SEND_DIRECT);
    	return DLT_DAEMON_ERROR_OK;
    }

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    if(sd_notify(0, "WATCHDOG=1") < 0)
    {
        dlt_log(LOG_WARNING, "Could not reset systemd watchdog\n");
    }
    curr_time = dlt_uptime();
#endif
    while ( (length = dlt_buffer_copy(&(daemon->client_ringbuffer), data, sizeof(data)) ) > 0)
    {
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
        if ((dlt_uptime() - curr_time) / 10000 >= watchdog_trigger_interval)
        {
            if(sd_notify(0, "WATCHDOG=1") < 0)
            {
                dlt_log(LOG_WARNING, "Could not reset systemd watchdog\n");
            }
            curr_time = dlt_uptime();
        }
#endif

    	if((ret = dlt_daemon_client_send(DLT_DAEMON_SEND_FORCE,daemon,daemon_local,0,0,data,length,0,0,verbose)))
    	{
    		return ret;
    	}
    	dlt_buffer_remove(&(daemon->client_ringbuffer));
    	if(daemon->state != DLT_DAEMON_STATE_SEND_BUFFER)
    		dlt_daemon_change_state(daemon,DLT_DAEMON_STATE_SEND_BUFFER);

        if(dlt_buffer_get_message_count(&(daemon->client_ringbuffer)) <= 0)
        {
        	dlt_daemon_change_state(daemon,DLT_DAEMON_STATE_SEND_DIRECT);
        	return DLT_DAEMON_ERROR_OK;
        }
    }

    return DLT_DAEMON_ERROR_OK;
}

int create_timer_fd(DltDaemonLocal *daemon_local, int period_sec, int starts_in, int* fd, const char* timer_name)
{
    int local_fd = -1;
    struct itimerspec l_timer_spec;

    if(timer_name == NULL)
    {
        timer_name = "timer_not_named";
    }

    if( fd == NULL )
    {
        snprintf(str, sizeof(str), "<%s> fd is NULL pointer\n", timer_name );
        dlt_log(LOG_WARNING, str);
        return -1;
    }

    if( period_sec > 0 ) {
#ifdef linux
        local_fd = timerfd_create(CLOCK_MONOTONIC, 0);
        if( local_fd < 0)
        {
            snprintf(str, sizeof(str), "<%s> timerfd_create failed: %s\n", timer_name, strerror(errno));
            dlt_log(LOG_WARNING, str);
        }

        l_timer_spec.it_interval.tv_sec = period_sec;
        l_timer_spec.it_interval.tv_nsec = 0;
        l_timer_spec.it_value.tv_sec = starts_in;
        l_timer_spec.it_value.tv_nsec = 0;

        if( timerfd_settime( local_fd, 0, &l_timer_spec, NULL) < 0)
        {
            snprintf(str, sizeof(str), "<%s> timerfd_settime failed: %s\n", timer_name, strerror(errno));
            dlt_log(LOG_WARNING, str);
            local_fd = -1;
        }
#endif
    }
    else {
        // timer not activated via the service file
        snprintf(str, sizeof(str), "<%s> not set: period=0\n", timer_name);
        dlt_log(LOG_INFO, str);
        local_fd = -1;
    }

    // If fd is fully initialized, let's add it to the fd sets
    if(local_fd>0)
    {
        snprintf(str, sizeof(str), "<%s> initialized with %ds timer\n", timer_name, period_sec);
        dlt_log(LOG_INFO, str);

        FD_SET(local_fd, &(daemon_local->master));
        //FD_SET(local_fd, &(daemon_local->timer_fds));
        if (local_fd > daemon_local->fdmax)
        {
            daemon_local->fdmax = local_fd;
        }
    }

    *fd = local_fd;

    return local_fd;
}

/* Close connection function */
int dlt_daemon_close_socket(int sock, DltDaemon *daemon, DltDaemonLocal *daemon_local, int verbose)
{
	dlt_daemon_socket_close(sock);

	FD_CLR(sock, &(daemon_local->master));

	if (daemon_local->client_connections)
	{
		daemon_local->client_connections--;
	}

	if(daemon_local->client_connections==0)
	{
		/* send new log state to all applications */
		daemon->connectionState = 0;
		dlt_daemon_user_send_all_log_state(daemon,verbose);

		// For offline tracing we still can use the same states 
		// as for socket sending. Using this trick we see the traces 
		// In the offline trace AND in the socket stream.
        if(daemon_local->flags.yvalue[0] == 0)
    	    dlt_daemon_change_state(daemon,DLT_DAEMON_STATE_BUFFER);
	}

	if (daemon_local->flags.vflag)
	{
		snprintf(str,DLT_DAEMON_TEXTBUFSIZE, "Connection to client lost, #connections: %d\n",daemon_local->client_connections);
		dlt_log(LOG_INFO, str);
	}

	dlt_daemon_control_message_connection_info(DLT_DAEMON_SEND_TO_ALL,daemon,daemon_local,DLT_CONNECTION_STATUS_DISCONNECTED,"",verbose);

	return 0;
}

/**
  \}
*/
