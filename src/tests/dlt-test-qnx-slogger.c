/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2021 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#include "dlt.h"
#include "dlt_common.h" /* for dlt_get_version() */

#define COUNT 10
#define DELAY 500
#define LENGTH 100

void usage()
{
    char version[255];

    dlt_get_version(version, 255);

    printf("Usage: dlt-test-qnx-slogger [options]\n");
    printf("Generate messages and send them to slogger2\n");
    printf("%s\n", version);
    printf("Options:\n");
    printf("  -h          Usage\n");
    printf("  -n count    Number of messages to be generated (Default: %d)\n", COUNT);
    printf("  -d delay    Milliseconds to wait between sending messages (Default: %d)\n", DELAY);
    printf("  -l length   Message payload length (Default: %d bytes)\n", LENGTH);
}

int main(int argc, char *argv[]) {
    int i = 0;
    int count = COUNT;
    int delay = DELAY;
    int length = LENGTH;
    char *str = NULL;
    struct timespec ts;

    int c;

    while ((c = getopt(argc, argv, "hn:d:l:")) != -1)
    {
        switch(c)
        {
        case 'n':
        {
            count = atoi(optarg);
            break;
        }
        case 'd':
        {
            delay = atoi(optarg);
            break;
        }
        case 'l':
        {
            length = atoi(optarg);
            break;
        }
        case 'h':
        {
            usage();
            return -1;
        }
        case '?':
        {
            if ((optopt == 'n') || (optopt == 'd') || (optopt == 'l'))
                fprintf(stderr, "Option -%c requires an argument\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c`\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x`\n", optopt);

            /* unknown or wrong option used, show usage information and terminate */
            usage();
            return -1;
        }
        default:
        {
            usage();
            return -1;
        }
        }
    }

    /* Generate string */
    if (length > 0)
    {
        str = (char *) malloc((size_t) length + 1);
        if (str == NULL)
        {
            fprintf(stderr, "Cannot allocate memory\n");
            return -1;
        }
        memset(str, 'X', (size_t) length);
        str[length] = '\n';
    }

    /* Calculate delay */
    if (delay > 0) {
        ts.tv_sec = delay / 1000;
        ts.tv_nsec = (delay % 1000) * 1000000;
    }


    for (i = 0; i < count; i++)
    {
        slogf(_SLOG_SETCODE(_SLOGC_TEST, 0), _SLOG_INFO, "%s", str);
        nanosleep(&ts, NULL);
    }

    if (str != NULL)
    {
        free(str);
        str = NULL;
    }

    return 0;
}
