/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2020, CriticalTechworks
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 */

/*!
 * \author Joao Sousa <joao.sa.sousa@ctw.bmwgroup.com>
 *
 * \copyright Copyright © 2020 CriticalTechworks. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-example-multicast-client.c
 */
#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include <sys/time.h>

#include "dlt_client.h"
#include "dlt_client_cfg.h"
#include "dlt_types.h"

#define MULTICAST_GROUP_ADDRESS "239.1.2.3"

// From dlt-example-multicast-clientmsg-view example
int message_callback(DltMessage *message, void *data)
{
    (void)data;  // supress unused warning
    static char text[1024];

    if ((message == NULL)) {
        printf("NULL message in dlt_receive_message_callback_udp\n");
        return -1;
    }

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    else
        dlt_set_storageheader(message->storageheader, "ECU1");

    dlt_message_header(message, text, 1024, 0);

    printf("%s ", text);

    dlt_message_payload(message, text, 1024, DLT_OUTPUT_ASCII, 0);

    printf("[%s]\n", text);

    return 0;
}

/*
* Get and print DLT messages in an endless loop
*/
int main()
{
    DltClient client;

    if (dlt_client_init(&client, 1) == DLT_RETURN_ERROR)
    {
        printf("Error on client_init\n");
        return -1;
    }

    if (dlt_client_set_server_ip(&client, MULTICAST_GROUP_ADDRESS) == DLT_RETURN_ERROR)
    {
        printf("Error setting multicast group address\n");
        return -1;
    }

    if (dlt_client_set_mode(&client, DLT_CLIENT_MODE_UDP_MULTICAST) == DLT_RETURN_ERROR)
    {
        printf("Error setting mode\n");
        return -1;
    }

    if (dlt_client_connect(&client, 1) == DLT_RETURN_ERROR)
    {
        printf("Error on connection\n");
        return -1;
    }

    dlt_client_register_message_callback(&message_callback);

    /* Dlt Client Main Loop */
    dlt_client_main_loop(&client, NULL, 0);

    /* Dlt Client Cleanup */
    dlt_client_cleanup(&client, 1);
    return 0;
}