/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Lassi Marttala <Lassi.LM.Marttala@partner.bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-multi-process-client.c
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
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/stat.h>

// DLT Library includes
#include "dlt_client.h"
#include "dlt_protocol.h"
#include "dlt_user.h"
// PRivate includes
#include "dlt-test-multi-process.h"

// Local data structures
typedef struct {
    int max_messages;
    int verbose;
    int serial;
    int baudrate;
    char *output;
    int output_handle;
    int messages_left;
    DltClient *client_ref;
} s_parameters;

typedef struct {
     int messages_received;
     int broken_messages_received;
     int bytes_received;
     int first_message_time;
     int output_bytes;
} s_statistics;

// Forward declarations
int receive(DltMessage *msg, void *data);

/**
 * Print usage information
 */
void usage(char *name) {
    char version[255];
    dlt_get_version(version,255);

    printf("Usage: %s [options] <remote address|serial device>\n", name);
    printf("Receive messages from dlt-test-multi-process.\n");
    printf("%s", version);
    printf("Options:\n");
    printf(" -m             Total messages to receive. (Default: 10000)\n");
    printf(" -y             Serial device mode.\n");
    printf(" -b baudrate    Serial device baudrate. (Default: 115200)\n");
    printf(" -v             Verbose. Increases the verbosity level of dlt client library.\n");
    printf(" -o filename    Output messages in new DLT file.\n");
}

/**
 * Initialize reasonable default parameters.
 */
void init_params(s_parameters *params) {
    params->max_messages = 10000;
    params->verbose = 0;
    params->serial = 0;
    params->output = NULL;
    params->output_handle = -1;
    params->baudrate = 115200;
}

/**
 * Read the command line parameters
 */
int read_params(s_parameters *params, int argc, char *argv[]) {
    init_params(params);
    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "m:yb:vo:")) != -1) {
        switch (c) {
        case 'm':
            params->max_messages = atoi(optarg);
            break;
        case 'y':
            params->serial = 1;
            break;
        case 'b':
            params->baudrate = atoi(optarg);
            break;
        case 'v':
            params->verbose = 1;
            break;
        case 'o':
            params->output = optarg;
            break;
        case '?':
            if(optopt == 'm' || optopt == 'b' || optopt == 'o')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }
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
int init_dlt_connect(DltClient *client, const s_parameters *params, int argc, char *argv[]) {
    char id[4];
    if (argc < 2)
        return -1;
    if(params->serial > 0)
    {
        client->mode = 1;
        client->serialDevice = argv[argc - 1];
        dlt_client_setbaudrate(client, params->baudrate);
    }
    else
    {
        client->servIP = argv[argc - 1];
    }
    dlt_set_id(id, ECUID);
    return 0;
}

/**
 * Entry point
 */
int main(int argc, char *argv[]) {
    s_parameters params;
    DltClient client;
    params.client_ref = &client;
    int err = read_params(&params, argc, argv);

    if (err != 0) {
        usage(argv[0]);
        return err;
    }

    dlt_client_init(&client, params.verbose);
    dlt_client_register_message_callback(receive);

    err = init_dlt_connect(&client, &params, argc, argv);
    if (err != 0) {
        usage(argv[0]);
        return err;
    }

    err = dlt_client_connect(&client, params.verbose);
    if (err != DLT_RETURN_OK) {
            printf("Failed to connect %s.\n", client.mode > 0 ? client.serialDevice : client.servIP);
        return err;
    }

    if(params.output)
    {
        params.output_handle = open(params.output,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (params.output_handle == -1)
        {
            fprintf(stderr,"Failed to open %s for writing.\n",params.output);
            return -1;
        }
    }

    params.messages_left = params.max_messages;

    dlt_client_main_loop(&client, &params, params.verbose);

    if(params.output_handle > 0)
    {
        close(params.output_handle);
    }
    return 0;
}

/**
 * Print current test statistics
 */
void print_stats(s_statistics stats, s_parameters params)
{
    static int last_print_time;
    if(last_print_time >= time(NULL) && // Only print once a second
       (stats.messages_received+stats.broken_messages_received) % 1000 != 0 &&
       params.messages_left != 0) // Print also every 1000th message
    {
        return;
    }
    printf("\033[2J\033[1;1H"); // Clear screen.
    printf("Statistics:\n");
    printf(" Messages received             : %d\n", stats.messages_received);
    printf(" Broken messages received      : %d\n", stats.broken_messages_received);
    printf(" Bytes received                : %d\n", stats.bytes_received);
    printf(" Time running (seconds)        : %ld\n", time(NULL)-stats.first_message_time);
    printf(" Throughput (msgs/sec)/(B/sec) : %ld/%ld\n",
            stats.messages_received/((time(NULL)-stats.first_message_time)+1),
            (stats.bytes_received)/((time(NULL)-stats.first_message_time)+1));
    if(params.messages_left == 0)
    {
        if(stats.broken_messages_received == 0)
            printf("All messages received succesfully!\n");
        else
            printf("Test failure! There were %d broken messages.", stats.broken_messages_received);

    }
    fflush(stdout);
    last_print_time = time(NULL);
}
/**
 * Callback for dlt client
 */
int receive(DltMessage *msg, void *data) {
    static s_statistics stats;
    char apid[5];
    struct iovec iov[2];
    s_parameters *params = (s_parameters *)data;

    memset(apid, 0, 5);
    memcpy(apid, msg->extendedheader->apid, 4);

    if(apid[0] != 'M' || apid[1] != 'T') // TODO: Check through the app description
        return 0;

    params->messages_left--;

    if(stats.first_message_time == 0)
    {
        stats.first_message_time = time(NULL);
    }

    int buflen = msg->datasize + 1;
    char *buf = malloc(buflen);
    if(buf==0)
    {
        printf("Out of memory\n");
        return -1;
    }
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
    stats.bytes_received += msg->datasize+msg->headersize-sizeof(DltStorageHeader);;

    free(buf);

    print_stats(stats, *params);

    if (params->output_handle > 0)
    {
        iov[0].iov_base = msg->headerbuffer;
        iov[0].iov_len = msg->headersize;
        iov[1].iov_base = msg->databuffer;
        iov[1].iov_len = msg->datasize;

        stats.output_bytes += writev(params->output_handle, iov, 2);
    }
    if(params->messages_left < 1)
    {
        dlt_client_cleanup(params->client_ref, params->verbose);
    }
    return 0;
}
