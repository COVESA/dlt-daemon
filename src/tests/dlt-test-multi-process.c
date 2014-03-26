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
 * \author Lassi Marttala <Lassi.LM.Marttala@partner.bmw.de> BMW 2011-2012
 *
 * \file dlt-test-multi-process.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-multi-process.c                                      **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala Lassi.LM.Marttala@partner.bmw.de               **
**                                                                            **
**  PURPOSE   : Stress test timing using multiple processes                   **
**                                                                            **
**  REMARKS   : Requires POSIX fork()                                         **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <syslog.h>

#include "dlt.h"
#include "dlt_common.h"
#include "dlt-test-multi-process.h"

// Constants
#define MAX_PROCS 100
#define MAX_THREADS 100

// Structs
typedef struct {
	int nmsgs;			// Number of messages to send
	int nprocs;			// Number of processes to start
	int nthreads;		// Number of threads to start
	int delay;			// Delay between logs messages for each process
	int delay_fudge;	// Fudge the delay by 0-n to cause desynchronization
} s_parameters;

typedef struct {
	s_parameters 	params;
	DltContext		ctx;
} s_thread_data;

// Forward declarations
void init_params(s_parameters * params);
void quit_handler(int signum);
void cleanup();
void do_forks(s_parameters params);
void run_threads(s_parameters params);
void do_logging(s_thread_data *data);
int wait_for_death();

// State information
volatile sig_atomic_t in_handler = 0;

// Globals for cleanup from main and signal handler
pid_t pids[MAX_PROCS];
unsigned int pidcount = 0;

/**
 * Print instructions.
 */
void usage(char *prog_name)
{
	char version[255];
	dlt_get_version(version,255);
	s_parameters defaults;
	init_params(&defaults);

	printf("Usage: %s [options]\n", prog_name);
	printf("Test application for stress testing the daemon with multiple processes and threads.\n");
	printf("%s\n", version);
	printf("Options (Default):\n");
	printf(" -m number		Number of messages per thread to send. (%d)\n", defaults.nmsgs);
	printf(" -p number		Number of processes to start. (%d), Max %d.\n", defaults.nprocs, MAX_PROCS);
	printf(" -t number		Number of threads per process. (%d), Max %d.\n", defaults.nthreads, MAX_THREADS);
	printf(" -d delay		Delay in milliseconds to wait between log messages. (%d)\n", defaults.delay);
	printf(" -f delay		Random fudge in milliseconds to add to delay. (%d)\n", defaults.delay_fudge);
}

/**
 * Set nice default values for parameters
 */
void init_params(s_parameters * params) {
	params->nmsgs		= 100;
	params->nprocs		= 10;
	params->nthreads	= 2;
	params->delay		= 1000;
	params->delay_fudge	= 100;
}

/**
 * Read the command line and modify parameters
 */
int read_cli(s_parameters *params, int argc, char **argv)
{
	int c;
	opterr = 0;
    while ((c = getopt (argc, argv, "m:p:t:d:f:")) != -1)
	{
    	switch(c)
    	{
    		case 'm':
    			params->nmsgs		= atoi(optarg);
    			break;
    		case 'p':
				params->nprocs 		= atoi(optarg);
				if(params->nprocs > MAX_PROCS)
				{
					fprintf(stderr, "Too many processes selected.\n");
					return -1;
				}
				break;
			case 't':
				params->nthreads	= atoi(optarg);
				if(params->nprocs > MAX_PROCS)
				{
					fprintf(stderr, "Too many threads selected.\n");
					return -1;
				}
				break;
			case 'd':
				params->delay		= atoi(optarg);
				break;
			case 'f':
				params->delay_fudge	= atoi(optarg);
				break;
			case '?':
				if(optopt == 'n' || optopt == 'd' || optopt == 'f')
				{
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt))
				{
					fprintf(stderr, "Unknown option '-%c'.\n", optopt);
				}
				else
				{
					fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
				}
				return -1;
				break;
			default:
				abort();
                                return -1;//for parasoft
    	}
	}
	return 0;
}

/**
 * Entry point
 */
int main(int argc, char **argv)
{
	// Prepare parameters
	s_parameters params;
	init_params(&params);
	if(read_cli(&params, argc, argv) != 0) {
		usage(argv[0]);
		exit(-1);
	}

	// Launch the child processes
	do_forks(params);

	// Register signal handlers
	if(signal(SIGINT, quit_handler) == SIG_IGN)
		signal(SIGINT, SIG_IGN); 	// C-c
	if(signal(SIGHUP, quit_handler) == SIG_IGN)
		signal(SIGHUP, SIG_IGN); 	// Terminal closed
	if(signal(SIGTERM, quit_handler) == SIG_IGN)
		signal(SIGTERM, SIG_IGN); 	// kill (nice)

	printf("Setup done. Listening. My pid: %d\n", getpid());    
	fflush(stdout);

	int err = wait_for_death();
	cleanup();
	return err;
}

/**
 * Start the child processes
 */

void do_forks(s_parameters params)
{
    int i;

	// Launch child processes
	for(i=0;i<params.nprocs;i++)
	{
		pid_t pid = fork();
		switch(pid)
		{
		case -1: // An error occured
			if(errno == EAGAIN)
			{
				fprintf(stderr, "Could not allocate resources for child process.\n");
				cleanup();
				abort();
			}
			if(errno == ENOMEM)
			{
				fprintf(stderr, "Could not allocate memory for child process' kernel structure.\n");
				cleanup();
				abort();
			}
			break;
		case 0: // Child process, start threads
			run_threads(params);
			break;
		default: // Parent process, store the childs pid
			pids[pidcount++] = pid;
			break;
		}
	}
}

/**
 * Clean up the child processes.
 * Reraise signal to default handler.
 */
void quit_handler(int signum)
{
	if(in_handler)
		raise(signum);
	in_handler = 1;

	cleanup();

	signal(signum, SIG_DFL);
	raise(signum);
}

/**
 * Ask the child processes to die
 */
void cleanup()
{
	unsigned int i;
	for(i=0;i<pidcount;i++)
	{
		kill(pids[i], SIGINT);
	}
}

/**
 * Generate the next sleep time
 */
time_t mksleep_time(int delay, int fudge)
{
    if (!fudge)
        return delay*1000;
    else
        return (delay+rand()%fudge)*1000;
}

/**
 * Open logging channel and proceed to spam messages
 */
void do_logging(s_thread_data *data)
{
	DltContext 		mycontext;
	char 			ctid[5];
	char 			ctid_name[256];


    snprintf(ctid,5,"%.2x", rand() & 0x0000ffff);
	snprintf(ctid_name,256, "Child %s in dlt-test-multi-process", ctid);
	DLT_REGISTER_CONTEXT(mycontext, ctid, ctid_name);

	int msgs_left = data->params.nmsgs;
	while(msgs_left-- > 0)
	{
		DLT_LOG(mycontext, DLT_LOG_INFO, DLT_STRING(PAYLOAD_DATA));
		usleep(mksleep_time(data->params.delay, data->params.delay_fudge));
	}
	DLT_UNREGISTER_CONTEXT(mycontext);
}

/**
 * Start the threads and wait for them to return.
 */
void run_threads(s_parameters params)
{
	pthread_t		thread[params.nthreads];
	s_thread_data	thread_data;
	char 			apid[5];
	char			apid_name[256];
	int 			i;

	srand(getpid());

    snprintf(apid,5,"MT%02u", pidcount);
    snprintf(apid_name,256, "Apps %s.", apid);

	DLT_REGISTER_APP(apid, apid_name);

	thread_data.params 	= params;

	for(i=0;i<params.nthreads;i++)
	{
		if(pthread_create(&(thread[i]), NULL, (void *) &do_logging, &thread_data) != 0)
		{
			printf("Error creating thread.\n");
			abort();
		}
	}

	for(i=0;i<params.nthreads;i++)
	{
		pthread_join(thread[i], NULL);
	}


	DLT_UNREGISTER_APP();
	// We can exit now
	exit(0);
}

/**
 * Wait for child processes to complete their work.
 */
int wait_for_death()
{
	int pids_left = pidcount;
	while(pids_left > 0)
	{
		int status;
		pid_t w = waitpid(WAIT_ANY, &status, 0);
		if(status < 0)
		{
			return -1;
		}
		else
		{
			unsigned int i;
			for(i=0;i<pidcount;i++)
			{
				if(pids[i] == w)
				{
					pids_left--;
					break;
				}
			}
		}
	}
	return 0;
}
