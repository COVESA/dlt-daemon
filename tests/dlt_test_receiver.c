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
 * \file dlt-receive.c
 * For further information see http://www.genivi.org/.
 * @licence end@
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
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */
#include <string.h>

#include "dlt_client.h"
#include <zlib.h>

#define DLT_RECEIVE_TEXTBUFSIZE 10024  /* Size of buffer for text output */

#define DLT_RECEIVE_ECU_ID "RECV"

/* Function prototypes */
int dlt_receive_filetransfer_callback(DltMessage *message, void *data);

typedef struct {
    int aflag;
    int sflag;
    int xflag;
    int mflag;
    int vflag;
    int yflag;
    char *ovalue;
    char *fvalue;
    char *evalue;
    int bvalue;
    char ecuid[4];
    int ohandle;
    DltFile file;
    DltFilter filter;
} DltReceiveData;

FILE *fp;

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version,255);

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
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("  -o filename   Output messages in new DLT file\n");
    printf("  -f filename   Enable filtering of messages\n");
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    DltClient      dltclient;
    DltReceiveData dltdata;
    int c;
    int index;

    /* Initialize dltdata */
    dltdata.aflag = 0;
    dltdata.sflag = 0;
    dltdata.xflag = 0;
    dltdata.mflag = 0;
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.ovalue = 0;
    dltdata.fvalue = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;
    dltdata.ohandle=-1;

    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vashyxmf:o:e:b:")) != -1)
        switch (c)
        {
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
        case 'y':
            {
                dltdata.yflag = 1;
                break;
            }
        case 'f':
            {
                dltdata.fvalue = optarg;
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
                if (optopt == 'o' || optopt == 'f')
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
                abort ();
                return -1;//for parasoft
            }
        }

    /* Initialize DLT Client */
    dlt_client_init(&dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    // fp = open ....
    dlt_client_register_message_callback(dlt_receive_filetransfer_callback);

    /* Setup DLT Client structure */
    dltclient.serial_mode = dltdata.yflag;

    if (dltclient.serial_mode==0)
    {
        for (index = optind; index < argc; index++)
        {
            dltclient.servIP = argv[index];
        }

        if (dltclient.servIP == 0)
        {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr,"ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&dltclient,dltdata.vflag);
            return -1;
        }
    }
    else
    {
        for (index = optind; index < argc; index++)
        {
            dltclient.serialDevice = argv[index];
        }

        if (dltclient.serialDevice == 0)
        {
            /* no serial device name selected, show usage and terminate */
            fprintf(stderr,"ERROR: No serial device name specified\n");
            usage();
            return -1;
        }

        dlt_client_setbaudrate(&dltclient,dltdata.bvalue);
    }

    /* initialise structure to use DLT file */
    dlt_file_init(&(dltdata.file),dltdata.vflag);

    /* first parse filter file if filter parameter is used */
    dlt_filter_init(&(dltdata.filter),dltdata.vflag);

    if (dltdata.fvalue)
    {
        if (dlt_filter_load(&(dltdata.filter),dltdata.fvalue,dltdata.vflag)<0)
        {
            dlt_file_free(&(dltdata.file),dltdata.vflag);
            return -1;
        }

        dlt_file_set_filter(&(dltdata.file),&(dltdata.filter),dltdata.vflag);
    }

    /* open DLT output file */
    if (dltdata.ovalue)
    {
        dltdata.ohandle = open(dltdata.ovalue,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */

        if (dltdata.ohandle == -1)
        {
            dlt_file_free(&(dltdata.file),dltdata.vflag);
            fprintf(stderr,"ERROR: Output file %s cannot be opened!\n",dltdata.ovalue);
            return -1;
        }
    }

    if (dltdata.evalue)
    {
        dlt_set_id(dltdata.ecuid,dltdata.evalue);
    }
    else
    {
        dlt_set_id(dltdata.ecuid,DLT_RECEIVE_ECU_ID);
    }

    /* Connect to TCP socket or open serial device */
    if (dlt_client_connect(&dltclient, dltdata.vflag)!=-1)
    {

        /* Dlt Client Main Loop */
        dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&dltclient,dltdata.vflag);
    }

    /* dlt-receive cleanup */
    if (dltdata.ovalue)
    {
        close(dltdata.ohandle);
    }

    dlt_file_free(&(dltdata.file),dltdata.vflag);

    dlt_filter_free(&(dltdata.filter),dltdata.vflag);

    return 0;
}

int dlt_receive_filetransfer_callback(DltMessage *message, void *data)
{
    DltReceiveData *dltdata;
    static char text[DLT_RECEIVE_TEXTBUFSIZE];

    static char crc[11];
    char filename[255];
    unsigned long FLENGTH = 1024*1024;          // --> 1MB


    struct iovec iov[2];
    int bytes_written;

    if ((message==0) || (data==0))
    {
        return -1;
    }

    dltdata = (DltReceiveData*)data;

    dlt_message_payload(message,text,DLT_RECEIVE_TEXTBUFSIZE,DLT_OUTPUT_ASCII,dltdata->vflag);
    printf("[%s]\n",text);

    // 1st get the crc to check the received data
    if( strncmp(text, "CRC", 3) == 0)
    {
        char *tmpFilename;
        strncpy(crc, text + 4, 10);
        crc[10] = '\0';
        tmpFilename = strrchr(text, '/') + 1;
        // create file for each received file, as named as crc value
        snprintf(filename, 255, "/tmp/%s", tmpFilename);
        fp = fopen(filename, "w+");
    }

    // 3rd create crc and verify on finish
    if( strncmp(text, "FLFI", 4) == 0)
    {
        unsigned long crcnew = crc32(0L, Z_NULL, 0);
        unsigned char buffer[FLENGTH];
        fseek(fp, SEEK_SET, 0);
        size_t readBytes = fread(buffer, sizeof(char), FLENGTH, fp);
        crcnew = crc32(crcnew, buffer, readBytes);

        if( (unsigned) atol(crc) == crcnew)
            printf("FILETRANSFER SUCCESSFULL");
        else
            printf("ERROR ON FILETRANSFER");
         fclose(fp);
    }

    // 2nd check if incomming data are filetransfer data
    if( strncmp(text, "FLDA", 4) == 0)
    {
        // truncate beginning of data stream ( FLDA, File identifier and package number)
        char *position = strchr(text, 32); // search for space
        snprintf(text, DLT_RECEIVE_TEXTBUFSIZE, position+1);
        position = strchr(text, 32);
        snprintf(text, DLT_RECEIVE_TEXTBUFSIZE, position+1);
        position = strchr(text, 32);
        snprintf(text, DLT_RECEIVE_TEXTBUFSIZE, position+1);

        // truncate ending of data stream ( FLDA )
        int len = strlen(text);
        text[len - 4] = '\0';
        printf("## [%s]\n", text);
        // hex to ascii and store at /tmp
        char tmp[3];
        int i;
        for(i = 0;i< (int) strlen(text); i = i+3)
        {
            tmp[0] = text[i];
            tmp[1] = text[i+1];
            tmp[2] = '\0';
            unsigned long h = strtoul(tmp, NULL, 16);
            fprintf(fp, "%c", (int) h);
        }
    }

    /* if file output enabled write message */
    if (dltdata->ovalue)
    {
        iov[0].iov_base = message->headerbuffer;
        iov[0].iov_len = message->headersize;
        iov[1].iov_base = message->databuffer;
        iov[1].iov_len = message->datasize;

        bytes_written = writev(dltdata->ohandle, iov, 2);

        if (0 > bytes_written){
                printf("dlt_receive_message_callback: writev(dltdata->ohandle, iov, 2); returned an error!" );
                return -1;
        }
    }

    return 0;
}
