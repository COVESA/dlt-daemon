/*
 * DLT multi process tester
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Lassi Marttala <Lassi.LM.Marttala@partner.bmw.de>
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
 **  SRC-MODULE: dlt-test-multi-process-client.c                               **
 **                                                                            **
 **  TARGET    : linux                                                         **
 **                                                                            **
 **  PROJECT   : DLT                                                           **
 **                                                                            **
 **  AUTHOR    : Lassi Marttala <Lassi.LM.Marttala@partner.bmw.de>             **
 **                                                                            **
 **  PURPOSE   : Receive, validate and measure data from multi process tester  **
 **                                                                            **
 **  REMARKS   :                                                               **
 **                                                                            **
 **  PLATFORM DEPENDANT [yes/no]: yes                                          **
 **                                                                            **
 **  TO BE CHANGED BY USER [yes/no]: no                                        **
 **                                                                            **
 *******************************************************************************/
// System includes
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// DLT Library includes
#include "dlt_client.h"
#include "dlt_protocol.h"
#include "dlt_user.h"
// PRivate includes
#include "dlt-test-multi-process.h"

// Local data structures
typedef struct {
	int verbose;
} s_parameters;

typedef struct {
	 int messages_received;
	 int broken_messages_received;
	 int bytes_received;
	 int first_message_time;
} s_statistics;

// Forward declarations
int receive(DltMessage *msg, void *data);

/**
 * Print usage information
 */
void usage(char *name) {
	printf("Usage: %s [options] <remote address>\n", name);
}

/**
 * Initialize reasonable default parameters.
 */
void init_params(s_parameters *params) {
	params->verbose = 0;
}

/**
 * Read the command line parameters
 */
int read_params(s_parameters *params, int argc, char *argv[]) {
	init_params(params);
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "v")) != -1) {
		switch (c) {
		case 'v':
			params->verbose = 1;
			break;
		case '?':
			/*if(optopt == '')
			 {
			 fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			 }*/
			if (isprint(optopt)) {
				fprintf(stderr, "Unknown option '-%c'.\n", optopt);
			} else {
				fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
			}
			return -1;
			break;
		default:
			return -1;
		}
	}
	return 0;
}

/**
 * Set the connection parameters for dlt client
 */
int init_dlt_connect(DltClient *client, int argc, char *argv[]) {
	char id[4];
	if (argc < 2)
		return -1;
	client->servIP = argv[argc - 1];
	dlt_set_id(id, ECUID);
	return 0;
}

/**
 * Entry point
 */
int main(int argc, char *argv[]) {
	s_parameters params;
	DltClient client;

	int err = read_params(&params, argc, argv);

	if (err != 0) {
		usage(argv[0]);
		return err;
	}

	dlt_client_init(&client, params.verbose);
	dlt_client_register_message_callback(receive);

	err = init_dlt_connect(&client, argc, argv);
	if (err != 0) {
		usage(argv[0]);
		return err;
	}

	err = dlt_client_connect(&client, params.verbose);
	if (err != 0) {
		printf("Failed to connect %s\n", client.servIP);
		return err;
	}
	dlt_client_main_loop(&client, NULL, params.verbose);
	return 0;
}

/**
 * Print current test statistics
 */
void print_stats(s_statistics stats)
{
	static int last_print_time;
	if(last_print_time >= time(NULL)) return; // Only print once a second
	printf("\033[2J\033[1;1H"); // Clear screen.
	printf("Statistics:\n");
	printf(" Messages received             : %d\n", stats.messages_received);
	printf(" Broken messages received      : %d\n", stats.broken_messages_received);
	printf(" Bytes received                : %d\n", stats.bytes_received);
	printf(" Time running (seconds)        : %ld\n", time(NULL)-stats.first_message_time);
	printf(" Throughput (msgs/sec)/(B/sec) : %ld/%ld\n",
			stats.messages_received/((time(NULL)-stats.first_message_time)+1),
			(stats.bytes_received)/((time(NULL)-stats.first_message_time)+1));
	fflush(stdout);
	last_print_time = time(NULL);
}
/**
 * Callback for dlt client
 */
int receive(DltMessage *msg, void *data) {
	static s_statistics stats;
	char apid[5];
	memset(apid, 0, 5);
	memcpy(apid, msg->extendedheader->apid, 4);

	if(strcmp(apid, DMPT_NAME) != 0) // Skip other messages
		return 0;

	if(stats.first_message_time == 0)
	{
		stats.first_message_time = time(NULL);
	}

	int buflen = msg->datasize + 1;
	char *buf = malloc(buflen);
	memset(buf, 0, buflen);

	dlt_message_payload(msg,buf,buflen-1,DLT_OUTPUT_ASCII,0);

	if(strcmp(buf, PAYLOAD_DATA) == 0)
	{
		stats.messages_received++;
	}
	else
	{
		stats.broken_messages_received++;
	}
	stats.bytes_received += buflen;

	free(buf);

	print_stats(stats);
	return 0;
}
