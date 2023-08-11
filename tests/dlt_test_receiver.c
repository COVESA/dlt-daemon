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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Stefan Held <stefan_held@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_test_receiver.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-receive.cpp                                               **
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
#include <string.h>
#include <syslog.h>

#include "dlt_client.h"

#define DLT_RECEIVE_ECU_ID "RECV"

/* Function prototypes */
int dlt_receive_filetransfer_callback(DltMessage *message, void *data);

typedef struct {
    int vflag;
    int yflag;
    char *ovalue;
    char *evalue;
    int bvalue;
    int filetransfervalue;
    int systemjournalvalue;
    int systemloggervalue;
    char ecuid[4];
    int ohandle;
    int sendSerialHeaderFlag;
    int resyncSerialHeaderFlag;
    DltFile file;
    DltFilter filter;
} DltReceiveData;

FILE *fp;
int result = 0;

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
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -S            Send message with serial header (Default: Without serial header)\n");
    printf("  -R            Enable resync serial header\n");
    printf("  -y            Serial device mode\n");
    printf("  -f            Activate filetransfer test case\n");
    printf("  -s            Activate systemd journal test case\n");
    printf("  -l            Activate system logger test case");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("  -o filename   Output messages in new DLT file\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char *argv[])
{
    DltClient dltclient;
    DltReceiveData dltdata;
    int c;
    int index;

    /* Initialize dltdata */
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.ovalue = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;
    dltdata.sendSerialHeaderFlag = 0;
    dltdata.resyncSerialHeaderFlag = 0;
    dltdata.ohandle = -1;
    dltdata.filetransfervalue = 0;
    dltdata.systemjournalvalue = 0;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vshSRyfla:o:e:b:")) != -1)
        switch (c) {
        case 'v':
        {
            dltdata.vflag = 1;
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
        case 'f':
        {
            dltdata.filetransfervalue = 1;
            break;
        }
        case 's':
        {
            dltdata.systemjournalvalue = 1;
            break;
        }
        case 'l':
        {
            dltdata.systemloggervalue = 1;
            break;
        }
        case 'o':
        {
            dltdata.ovalue = optarg;
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
        case '?':
        {
            if (optopt == 'o')
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
    dlt_client_register_message_callback(dlt_receive_filetransfer_callback);

    /* Setup DLT Client structure */
    dltclient.mode = dltdata.yflag;

    if (dltclient.mode == 0) {
        for (index = optind; index < argc; index++)
            if (dlt_client_set_server_ip(&dltclient, argv[index]) == -1) {
                fprintf(stderr, "set server ip didn't succeed\n");
                return -1;
            }



        if (dltclient.servIP == 0) {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr, "ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&dltclient, dltdata.vflag);
            return -1;
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

    /* open DLT output file */
    if (dltdata.ovalue) {
        dltdata.ohandle = open(dltdata.ovalue, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

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

    dlt_file_free(&(dltdata.file), dltdata.vflag);

    dlt_filter_free(&(dltdata.filter), dltdata.vflag);

    return 0;
}

int dlt_receive_filetransfer_callback(DltMessage *message, void *data)
{
    DltReceiveData *dltdata;
    static char text[DLT_RECEIVE_BUFSIZE];
    char filename[255];
    struct iovec iov[2];
    int bytes_written;

    if ((message == 0) || (data == 0))
        return -1;

    dltdata = (DltReceiveData *)data;

    if (dltdata->filetransfervalue) {
        dlt_message_print_ascii(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);

        /* 1st find starting point of tranfering data packages */
        if (strncmp(text, "FLST", 4) == 0) {
            char *tmpFilename;
            tmpFilename = strrchr(text, '/') + 1;
            unsigned int i;

            for (i = 0; i < strlen(tmpFilename); i++)
                if (isspace(tmpFilename[i])) {
                    tmpFilename[i] = '\0';
                    break;
                }

            /* create file for each received file, as named as crc value */
            snprintf(filename, 255, "/tmp/%s", tmpFilename);
            fp = fopen(filename, "w+");
        }

        /* 3rd close fp */
        if (strncmp(text, "FLFI", 4) == 0) {
            printf("TEST FILETRANSFER PASSED\n");
            fclose(fp);
        }

        /* 2nd check if incomming data are filetransfer data */
        if (strncmp(text, "FLDA", 4) == 0) {
            /* truncate beginning of data stream ( FLDA, File identifier and package number) */
            char *position = strchr(text, 32); /* search for space */
            strncpy(text, position + 1, DLT_RECEIVE_BUFSIZE);
            position = strchr(text, 32);
            strncpy(text, position + 1, DLT_RECEIVE_BUFSIZE);
            position = strchr(text, 32);
            strncpy(text, position + 1, DLT_RECEIVE_BUFSIZE);

            /* truncate ending of data stream ( FLDA ) */
            int len = strlen(text);
            text[len - 4] = '\0';
            /* hex to ascii and store at /tmp */
            char tmp[3];
            int i;

            for (i = 0; i < (int)strlen(text); i = i + 3) {
                tmp[0] = text[i];
                tmp[1] = text[i + 1];
                tmp[2] = '\0';
                unsigned long h = strtoul(tmp, NULL, 16);
                fprintf(fp, "%c", (int)h);
            }
        }
    }

    if (dltdata->systemjournalvalue) {
        dlt_message_print_ascii(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);
        /* 1st find the relevant packages */
        char *tmp = message->extendedheader->ctid;
        tmp[4] = '\0';

        if (strcmp(tmp, (const char *)"JOUR") == 0) {
            if (strstr(text, "DLT SYSTEM JOURNAL TEST")) {
                result++;

                if (result == 1000)
                    exit(159);
            }
        }
    }

    if (dltdata->systemloggervalue) {
        dlt_message_print_ascii(message, text, DLT_RECEIVE_BUFSIZE, dltdata->vflag);
        /* 1st find the relevant packages */
        char *tmp = message->extendedheader->ctid;
        tmp[4] = '\0';
        const char *substring = text;
        const char *founding = "Test Systemlogger";
        int length = strlen(founding);

        if (strcmp(tmp, (const char *)"PROC") == 0) {
            substring = strstr(text, founding);

            while (substring != NULL) {
                result++;
                substring += length;

                if (result == 1000)
                    exit(159);
            }
        }
    }

    /* if file output enabled write message */
    if (dltdata->ovalue) {
        iov[0].iov_base = message->headerbuffer;
        iov[0].iov_len = message->headersize;
        iov[1].iov_base = message->databuffer;
        iov[1].iov_len = message->datasize;

        bytes_written = writev(dltdata->ohandle, iov, 2);

        if (0 > bytes_written) {
            printf("dlt_receive_message_callback: writev(dltdata->ohandle, iov, 2); returned an error!");
            return -1;
        }
    }

    return 0;
}
