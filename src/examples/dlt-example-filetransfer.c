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
 * \file dlt-example-filetransfer.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-client.c                                             **
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
*******************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <dlt_filetransfer.h>     /*Needed for transferring files with the dlt protocol*/
#include <dlt.h>            /*Needed for dlt logging*/


#define MAXSTRLEN 1024

#define FLTR_APP_DESC      "Filetransfer application"
#define FLTR_CONTEXT_DESC  "Filetransfer context"

#define FLTR_APP "FLTR"
#define FLTR_CONTEXT "FLTR"

#define TIMEOUT 1

/*!Declare some context for the file transfer. It's not a must have to do this, but later you can set a filter on this context in the dlt viewer. */
DLT_DECLARE_CONTEXT(fileContext)


/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-example-filetransfer [options] absolute-path-to-file\n");
    printf("Simple filetransfer example");
    printf("%s \n", version);
    printf("Options:\n");
    printf("-a apid      - Set application id to apid (default: FLTR)\n");
    printf("-c ctid      - Set context id to ctid (default: FLTR)\n");
    printf("-t ms        - Timeout between file packages in ms (minimum 1 ms)\n");
    printf("-d           - Flag to delete the file after the transfer (default: false)\n");
    printf("-i           - Flag to log file infos to DLT before transfer file (default: false)\n");
    printf("-h           - This help\n");

}


/*!Main program dlt-test-filestransfer starts here */
int main(int argc, char *argv[])
{
    /*char str[MAXSTRLEN]; */
    int opt, timeout;

    char apid[DLT_ID_SIZE];
    char ctid[DLT_ID_SIZE];

    /*char version[255]; */
    int index;
    int dflag = 0;
    int iflag = 0;
    char *file = 0;
    char *tvalue = 0;

    dlt_set_id(apid, FLTR_APP);
    dlt_set_id(ctid, FLTR_CONTEXT);

    while ((opt = getopt(argc, argv, "idf:t:a:c:h")) != -1)
        switch (opt) {
        case 'd':
        {
            dflag = 1;
            break;
        }
        case 'i':
        {
            iflag = 1;
            break;
        }
        case 't':
        {
            tvalue = optarg;
            break;
        }
        case 'a':
        {
            dlt_set_id(apid, optarg);
            break;
        }
        case 'c':
        {
            dlt_set_id(ctid, optarg);
            break;
        }
        case 'h':
        {
            usage();
            break;
        }
        case '?':
        {
            if ((optopt == 'a') || (optopt == 'c') || (optopt == 't'))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        }



    for (index = optind; index < argc; index++)
        file = argv[index];

    if (file == 0) {
        /* no message, show usage and terminate */
        fprintf(stderr, "ERROR: No absolute path to file specified\n");
        usage();
        return -1;
    }

    if (tvalue)
        timeout = atoi(tvalue);
    else
        timeout = TIMEOUT;

    /*Register the application at the dlt-daemon */
    DLT_REGISTER_APP(apid, FLTR_APP_DESC);

    /*Register the context of the main program at the dlt-daemon */
    DLT_REGISTER_CONTEXT(fileContext, ctid, FLTR_CONTEXT_DESC);

    /*More details in corresponding methods */
    if (iflag)
        dlt_user_log_file_infoAbout(&fileContext, file);

    if (dlt_user_log_file_complete(&fileContext, file, dflag, timeout) < 0)
        printf("File couldn't be transferred. Please check the dlt log messages.\n");

    /*Unregister the context in which the file transfer happened from the dlt-daemon */
    DLT_UNREGISTER_CONTEXT(fileContext);
    /*Unregister the context of the main program from the dlt-daemon */
    DLT_UNREGISTER_APP();

    return 0;
}
