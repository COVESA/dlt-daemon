/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-receive.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-receive.c                                                 **
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
 * Initials    Date         Comment
 * aw          13.01.2010   initial
 */

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */
#include <errno.h>
#include <string.h>
#include <glob.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#ifdef __linux__
#   include <linux/limits.h>
#else
#   include <limits.h>
#endif
#include <inttypes.h>

#include "dlt_client.h"
#include "dlt-control-common.h"

#define DLT_RECEIVE_ECU_ID "RECV"

DltClient dltclient;

void signal_handler(int signal)
{
    switch (signal) {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
    case SIGQUIT:
        /* stop main loop */
        shutdown(dltclient.receiver.fd, SHUT_RD);
        break;
    default:
        /* This case should never happen! */
        break;
    } /* switch */

}

/* Function prototypes */
int dlt_receive_message_callback(DltMessage *message, void *data);

typedef struct {
    int aflag;
    int sflag;
    int xflag;
    int mflag;
    int vflag;
    int yflag;
    int uflag;
    char *ovalue;
    char *ovaluebase; /* ovalue without ".dlt" */
    char *fvalue;       /* filename for space separated filter file (<AppID> <ContextID>) */
    char *jvalue;       /* filename for json filter file */
    char *evalue;
    int bvalue;
    int sendSerialHeaderFlag;
    int resyncSerialHeaderFlag;
    int64_t climit;
    char ecuid[4];
    int ohandle;
    int64_t totalbytes; /* bytes written so far into the output file, used to check the file size limit */
    int part_num;    /* number of current output file if limit was exceeded */
    DltFile file;
    DltFilter filter;
    int port;
    char *ifaddr;
} DltReceiveData;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-receive [options] hostname/serial_device_name\n");
    printf("Receive DLT messages from DLT daemon and print or store the messages.\n");
    printf("Use filters to filter received messages.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -a            Print DLT messages; payload as ASCII\n");
    printf("  -x            Print DLT messages; payload as hex\n");
    printf("  -m            Print DLT messages; payload as hex and ASCII\n");
    printf("  -s            Print DLT messages; only headers\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -S            Send message with serial header (Default: Without serial header)\n");
    printf("  -R            Enable resync serial header\n");
    printf("  -y            Serial device mode\n");
    printf("  -u            UDP multicast mode\n");
    printf("  -i addr       Host interface address\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("  -c limit      Restrict file size to <limit> bytes when output to file\n");
    printf("                When limit is reached, a new file is opened. Use K,M,G as\n");
    printf("                suffix to specify kilo-, mega-, giga-bytes respectively\n");
    printf("  -f filename   Enable filtering of messages with space separated list (<AppID> <ContextID>)\n");
    printf("  -j filename   Enable filtering of messages with filter defined in json file\n");
    printf("  -p port       Use the given port instead the default port\n");
    printf("                Cannot be used with serial devices\n");
}


int64_t convert_arg_to_byte_size(char *arg)
{
    size_t i;
    int64_t factor;
    int64_t result;

    /* check if valid input */
    for (i = 0; i < strlen(arg) - 1; ++i)
        if (!isdigit(arg[i]))
            return -2;

    /* last character */
    factor = 1;

    if ((arg[strlen(arg) - 1] == 'K') || (arg[strlen(arg) - 1] == 'k'))
        factor = 1024;
    else if ((arg[strlen(arg) - 1] == 'M') || (arg[strlen(arg) - 1] == 'm'))
        factor = 1024 * 1024;
    else if ((arg[strlen(arg) - 1] == 'G') || (arg[strlen(arg) - 1] == 'g'))
        factor = 1024 * 1024 * 1024;
    else if (!isdigit(arg[strlen(arg) - 1]))
        return -2;

    /* range checking */
    int64_t const mult = atoll(arg);

    if (((INT64_MAX) / factor) < mult)
        /* Would overflow! */
        return -2;

    result = factor * mult;

    /* The result be at least the size of one message
     * One message consists of its header + user data:
     */
    DltMessage msg;
    int64_t min_size = sizeof(msg.headerbuffer);
    min_size += 2048 /* DLT_USER_BUF_MAX_SIZE */;

    if (min_size > result) {
        dlt_vlog(LOG_ERR,
                 "ERROR: Specified limit: %" PRId64 "is smaller than a the size of a single message: %" PRId64 "!\n",
                 result,
                 min_size);
        result = -2;
    }

    return result;
}


/*
 * open output file
 */
int dlt_receive_open_output_file(DltReceiveData *dltdata)
{
    /* if (file_already_exists) */
    glob_t outer;

    if (glob(dltdata->ovalue,
#ifndef __ANDROID_API__
             GLOB_TILDE |
#endif
             GLOB_NOSORT, NULL, &outer) == 0) {
        if (dltdata->vflag)
            dlt_vlog(LOG_INFO, "File %s already exists, need to rename first\n", dltdata->ovalue);

        if (dltdata->part_num < 0) {
            char pattern[PATH_MAX + 1];
            pattern[PATH_MAX] = 0;
            snprintf(pattern, PATH_MAX, "%s.*.dlt", dltdata->ovaluebase);
            glob_t inner;

            /* sort does not help here because we have to traverse the
             * full result in any case. Remember, a sorted list would look like:
             * foo.1.dlt
             * foo.10.dlt
             * foo.1000.dlt
             * foo.11.dlt
             */
            if (glob(pattern,
#ifndef __ANDROID_API__
                     GLOB_TILDE |
#endif
                     GLOB_NOSORT, NULL, &inner) == 0) {
                /* search for the highest number used */
                size_t i;

                for (i = 0; i < inner.gl_pathc; ++i) {
                    /* convert string that follows the period after the initial portion,
                     * e.g. gt.gl_pathv[i] = foo.1.dlt -> atoi("1.dlt");
                     */
                    int cur = atoi(&inner.gl_pathv[i][strlen(dltdata->ovaluebase) + 1]);

                    if (cur > dltdata->part_num)
                        dltdata->part_num = cur;
                }
            }

            globfree(&inner);

            ++dltdata->part_num;

        }

        char filename[PATH_MAX + 1];
        filename[PATH_MAX] = 0;

        snprintf(filename, PATH_MAX, "%s.%i.dlt", dltdata->ovaluebase, dltdata->part_num++);

        if (rename(dltdata->ovalue, filename) != 0)
            dlt_vlog(LOG_ERR, "ERROR: rename %s to %s failed with error %s\n",
                     dltdata->ovalue, filename, strerror(errno));
        else if (dltdata->vflag)
            dlt_vlog(LOG_INFO, "Renaming existing file from %s to %s\n",
                     dltdata->ovalue, filename);
    } /* if (file_already_exists) */

    globfree(&outer);

    dltdata->ohandle = open(dltdata->ovalue, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return dltdata->ohandle;
}


void dlt_receive_close_output_file(DltReceiveData *dltdata)
{
    if (dltdata->ohandle) {
        close(dltdata->ohandle);
        dltdata->ohandle = -1;
    }
}


/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    DltReceiveData dltdata;
    memset(&dltdata, 0, sizeof(dltdata));
    int c;
    int index;

    /* Initialize dltdata */
    dltdata.climit = -1; /* default: -1 = unlimited */
    dltdata.ohandle = -1;
    dltdata.part_num = -1;
    dltdata.port = 3490;

    /* Config signal handler */
    struct sigaction act;

    /* Initialize signal handler struct */
    memset(&act, 0, sizeof(act));
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashSRyuxmf:j:o:e:b:c:p:i:")) != -1)
        switch (c) {
        case 'v':
        {
            dltdata.vflag = 1;
            break;
        }
        case 'a':
        {
            dltdata.aflag = 1;
            break;
        }
        case 's':
        {
            dltdata.sflag = 1;
            break;
        }
        case 'x':
        {
            dltdata.xflag = 1;
            break;
        }
        case 'm':
        {
            dltdata.mflag = 1;
            break;
        }
        case 'h':
        {
            usage();
            return -1;
        }
        case 'S':
        {
            dltdata.sendSerialHeaderFlag = 1;
            break;
        }
        case 'R':
        {
            dltdata.resyncSerialHeaderFlag = 1;
            break;
        }
        case 'y':
        {
            dltdata.yflag = 1;
            break;
        }
        case 'u':
        {
            dltdata.uflag = 1;
            break;
        }
        case 'i':
        {
            dltdata.ifaddr = optarg;
            break;
        }
        case 'f':
        {
            dltdata.fvalue = optarg;
            break;
        }
        case 'j':
        {
            #ifdef EXTENDED_FILTERING
            dltdata.jvalue = optarg;
            break;
            #else
            fprintf (stderr,
                     "Extended filtering is not supported. Please build with the corresponding cmake option to use it.\n");
            return -1;
            #endif
        }
        case 'o':
        {
            dltdata.ovalue = optarg;
            size_t to_copy = strlen(dltdata.ovalue);

            if (strcmp(&dltdata.ovalue[to_copy - 4], ".dlt") == 0)
                to_copy = to_copy - 4;

            dltdata.ovaluebase = (char *)calloc(1, to_copy + 1);

            if (dltdata.ovaluebase == NULL) {
                fprintf (stderr, "Memory allocation failed.\n");
                return -1;
            }

            dltdata.ovaluebase[to_copy] = '\0';
            memcpy(dltdata.ovaluebase, dltdata.ovalue, to_copy);
            break;
        }
        case 'e':
        {
            dltdata.evalue = optarg;
            break;
        }
        case 'b':
        {
            dltdata.bvalue = atoi(optarg);
            break;
        }
        case 'p':
        {
            dltdata.port = atoi(optarg);
            break;
        }

        case 'c':
        {
            dltdata.climit = convert_arg_to_byte_size(optarg);

            if (dltdata.climit < -1) {
                fprintf (stderr, "Invalid argument for option -c.\n");
                /* unknown or wrong option used, show usage information and terminate */
                usage();
                return -1;
            }

            break;
        }
        case '?':
        {
            if ((optopt == 'o') || (optopt == 'f') || (optopt == 'c'))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            abort ();
            return -1;    /*for parasoft */
        }
        }

    /* Initialize DLT Client */
    dlt_client_init(&dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_receive_message_callback);

    /* Setup DLT Client structure */
    if(dltdata.uflag) {
        dltclient.mode = DLT_CLIENT_MODE_UDP_MULTICAST;
    }
    else {
        dltclient.mode = dltdata.yflag;
    }

    if (dltclient.mode == DLT_CLIENT_MODE_TCP || dltclient.mode == DLT_CLIENT_MODE_UDP_MULTICAST) {
        dltclient.port = dltdata.port;

        unsigned int servIPLength = 1; // Counting the terminating 0 byte
        for (index = optind; index < argc; index++) {
            servIPLength += strlen(argv[index]);
            if (index > optind) {
                servIPLength++; // For the comma delimiter
            }
        }
        if (servIPLength > 1) {
            char* servIPString = malloc(servIPLength);
            strcpy(servIPString, argv[optind]);

            for (index = optind + 1; index < argc; index++) {
                strcat(servIPString, ",");
                strcat(servIPString, argv[index]);
            }

            int retval = dlt_client_set_server_ip(&dltclient, servIPString);
            free(servIPString);

            if (retval == -1) {
                fprintf(stderr, "set server ip didn't succeed\n");
                return -1;
            }
        }

        if (dltclient.servIP == 0) {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr, "ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&dltclient, dltdata.vflag);
            return -1;
        }

        if (dltdata.ifaddr != 0) {
            if (dlt_client_set_host_if_address(&dltclient, dltdata.ifaddr) != DLT_RETURN_OK) {
                fprintf(stderr, "set host interface address didn't succeed\n");
                return -1;
            }
        }
    }
    else {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_serial_device(&dltclient, argv[index]) == -1) {
                fprintf(stderr, "set serial device didn't succeed\n");
                return -1;
            }

        if (dltclient.serialDevice == 0) {
            /* no serial device name selected, show usage and terminate */
            fprintf(stderr, "ERROR: No serial device name specified\n");
            usage();
            return -1;
        }

        dlt_client_setbaudrate(&dltclient, dltdata.bvalue);
    }

    /* Update the send and resync serial header flags based on command line option */
    dltclient.send_serial_header = dltdata.sendSerialHeaderFlag;
    dltclient.resync_serial_header = dltdata.resyncSerialHeaderFlag;

    /* initialise structure to use DLT file */
    dlt_file_init(&(dltdata.file), dltdata.vflag);

    /* first parse filter file if filter parameter is used */
    dlt_filter_init(&(dltdata.filter), dltdata.vflag);

    if (dltdata.fvalue) {
        if (dlt_filter_load(&(dltdata.filter), dltdata.fvalue, dltdata.vflag) < DLT_RETURN_OK) {
            dlt_file_free(&(dltdata.file), dltdata.vflag);
            return -1;
        }

        dlt_file_set_filter(&(dltdata.file), &(dltdata.filter), dltdata.vflag);
    }

    #ifdef EXTENDED_FILTERING

    if (dltdata.jvalue) {
        if (dlt_json_filter_load(&(dltdata.filter), dltdata.jvalue, dltdata.vflag) < DLT_RETURN_OK) {
            dlt_file_free(&(dltdata.file), dltdata.vflag);
            return -1;
        }

        dlt_file_set_filter(&(dltdata.file), &(dltdata.filter), dltdata.vflag);
    }

    #endif

    /* open DLT output file */
    if (dltdata.ovalue) {
        if (dltdata.climit > -1) {
            dlt_vlog(LOG_INFO, "Using file size limit of %" PRId64 "bytes\n",
                     dltdata.climit);
            dltdata.ohandle = dlt_receive_open_output_file(&dltdata);
        }
        else { /* in case no limit for the output file is given, we simply overwrite any existing file */
            dltdata.ohandle = open(dltdata.ovalue, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }

        if (dltdata.ohandle == -1) {
            dlt_file_free(&(dltdata.file), dltdata.vflag);
            fprintf(stderr, "ERROR: Output file %s cannot be opened!\n", dltdata.ovalue);
            return -1;
        }
    }

    if (dltdata.evalue)
        dlt_set_id(dltdata.ecuid, dltdata.evalue);
    else
        dlt_set_id(dltdata.ecuid, DLT_RECEIVE_ECU_ID);

    /* Connect to TCP socket or open serial device */
    if (dlt_client_connect(&dltclient, dltdata.vflag) != DLT_RETURN_ERROR) {

        /* Dlt Client Main Loop */
        dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&dltclient, dltdata.vflag);
    }

    /* dlt-receive cleanup */
    if (dltdata.ovalue)
        close(dltdata.ohandle);

    free(dltdata.ovaluebase);

    dlt_file_free(&(dltdata.file), dltdata.vflag);

    dlt_filter_free(&(dltdata.filter), dltdata.vflag);

    return 0;
}

int dlt_receive_message_callback(DltMessage *message, void *data)
{
    DltReceiveData *dltdata;
    static char text[DLT_RECEIVE_BUFSIZE];

    struct iovec iov[2];
    int bytes_written;

    if ((message == 0) || (data == 0))
        return -1;

    dltdata = (DltReceiveData *)data;

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    else
        dlt_set_storageheader(message->storageheader, dltdata->ecuid);

    if (((dltdata->fvalue || dltdata->jvalue) == 0) ||
        (dlt_message_filter_check(message, &(dltdata->filter), dltdata->vflag) == DLT_RETURN_TRUE)) {
        /* if no filter set or filter is matching display message */
        if (dltdata->xflag) {
            dlt_message_print_hex(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);
        }
        else if (dltdata->aflag)
        {

            dlt_message_header(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);

            printf("%s ", text);

            dlt_message_payload(message, text, DLT_RECEIVE_BUFSIZE, DLT_OUTPUT_ASCII, dltdata->vflag);

            printf("[%s]\n", text);
        }
        else if (dltdata->mflag)
        {
            dlt_message_print_mixed_plain(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);
        }
        else if (dltdata->sflag)
        {

            dlt_message_header(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);

            printf("%s \n", text);
        }

        /* if file output enabled write message */
        if (dltdata->ovalue) {
            iov[0].iov_base = message->headerbuffer;
            iov[0].iov_len = (uint32_t)message->headersize;
            iov[1].iov_base = message->databuffer;
            iov[1].iov_len = (uint32_t)message->datasize;

            if (dltdata->climit > -1) {
                uint32_t bytes_to_write = message->headersize + message->datasize;

                if ((bytes_to_write + dltdata->totalbytes > dltdata->climit)) {
                    dlt_receive_close_output_file(dltdata);

                    if (dlt_receive_open_output_file(dltdata) < 0) {
                        printf(
                            "ERROR: dlt_receive_message_callback: Unable to open log when maximum filesize was reached!\n");
                        return -1;
                    }

                    dltdata->totalbytes = 0;
                }
            }

            bytes_written = (int)writev(dltdata->ohandle, iov, 2);

            dltdata->totalbytes += bytes_written;

            if (0 > bytes_written) {
                printf("dlt_receive_message_callback: writev(dltdata->ohandle, iov, 2); returned an error!");
                return -1;
            }
        }
    }

    return 0;
}
