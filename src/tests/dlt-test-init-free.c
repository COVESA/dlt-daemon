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
 * \author Sven Hassler <sven_hassler@mentor.com>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-init-free.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dlt_common.h"
#include "dlt_user.h"

void exec(const char *cmd, char *buffer, size_t length);
void printMemoryUsage();
char *occupyMemory(int size);
void do_example_test();
void do_dlt_test();

int num_repetitions;

int main(int argc, char **argv)
{
    if (argc > 1)
        num_repetitions = (int) strtol(argv[1], 0, 10);
    else
        num_repetitions = 1000;

    printf("Will do %d repetitions.\n", num_repetitions);

    /*do_example_test(); */
    do_dlt_test();

    printf("Done.\n");

    return 0;
}

/* Should increase and then decrease memory amount. */
void do_example_test()
{
    const int immediatelyFree = 0;

    int numBufs = 1024;
    int sizePerBuf = 1024 * 1024; /* 1MB */

    char **bufs = (char **)malloc(numBufs * sizeof(char *));

    for (int i = 0; i < numBufs; i++) {
        bufs[i] = occupyMemory(sizePerBuf);

        printf("after alloc: ");
        printMemoryUsage();

        if (immediatelyFree) {
            free(bufs[i]);

            printf("after free: ");
            printMemoryUsage();
        }
    }

    printf("deleting memory:\n");

    if (!immediatelyFree)
        for (int i = 0; i < numBufs; i++) {
            /*for (int i = numBufs - 1; i >= 0; i--) // other way round works, too */
            free(bufs[i]);

            printf("after free: ");
            printMemoryUsage();
        }

    free(bufs);
}

/* Should give stable amount of memory across all iterations. */
void do_dlt_test()
{
    for (int i = 0; i < num_repetitions; i++) {
        dlt_init();
        dlt_free();

        printf("Iteration %d) - currently used memory amount: ", i);
        printMemoryUsage();
    }
}

void exec(const char *cmd, char *buffer, size_t length)
{
    FILE *pipe = NULL;
    strncpy(buffer, "ERROR", length);

    if ((pipe = popen(cmd, "r")) == NULL)
        return;

    while (fgets(buffer, (int) length, pipe) != NULL);

    if (pipe != NULL)
        pclose(pipe);
}

void printMemoryUsage()
{
    char result[128] = { 0 };
    char command[128] = { 0 };

    snprintf(command, sizeof(command), "pmap %d | grep total", getpid());

    exec(command, result, sizeof(result));

    printf("%s", result);
}

char *occupyMemory(int size)
{
    char *buf = (char *)malloc(size * sizeof(char));

    for (int i = 0; i < 1; i++)
        buf[i] = 1;

    return buf;
}
