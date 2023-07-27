/**
 * Copyright (C) 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt - Diagnostic Log and Trace console apps.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file dlt-passive-node-ctrl.c
 * For further information see http://www.covesa.org/.
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-passive-node-ctrl.c                                       **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka <clipka@jp.adit-jv.com>                       **
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
**  CL          Christoph Lipka             ADIT                              **
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "dlt_protocol.h"
#include "dlt_client.h"
#include "dlt-control-common.h"
#include "dlt_daemon_connection_types.h"

#define MAX_RESPONSE_LENGTH     32

#define DLT_NODE_CONNECT        1
#define DLT_NODE_DISCONNECT     0
#define DLT_NODE_CONNECT_UNDEF  999

#define DLT_GATEWAY_CONNECTED   2
#define DLT_NODE_CONNECTED_STR    "Connected"
#define DLT_NODE_DISCONNECTED_STR "Disconnected"

#define UNDEFINED 999

static struct PassiveNodeOptions {
    unsigned int command;          /**< passive node control command */
    unsigned int connection_state; /**< connection state */
    char node_id[DLT_ID_SIZE];     /**< node identifier */
    long timeout;                  /**< Default timeout */
} g_options = {
    .command = UNDEFINED,
    .connection_state = UNDEFINED,
    .node_id = { '\0' },
};

unsigned int get_command(void)
{
    return g_options.command;
}

void set_command(unsigned int c)
{
    g_options.command = c;
}

unsigned int get_connection_state(void)
{
    return g_options.connection_state;
}

void set_connection_state(unsigned int s)
{
    if ((s == DLT_NODE_CONNECT) || (s == DLT_NODE_DISCONNECT)) {
        g_options.connection_state = s;
        set_command(DLT_SERVICE_ID_PASSIVE_NODE_CONNECT);
    }
    else {
        pr_error("Connection status %u invalid\n", s);
        exit(-1);
    }
}

void set_node_id(char *id)
{
    if (id == 0) {
        pr_error("node identifier is NULL\n");
        exit(-1);
    }
    else {
        strncpy(g_options.node_id, id, DLT_ID_SIZE);
    }
}

char *get_node_id()
{
    return g_options.node_id;
}

/**
 * @brief Print passive node status information
 *
 * @param info DltServicePassiveNodeConnectionInfo
 */
static void dlt_print_passive_node_status(
    DltServicePassiveNodeConnectionInfo *info)
{
    unsigned int i = 0;
    char *status;

    if (info == NULL)
        return;

    printf("\nPassive Node connection status:\n"
           "---------------------------------\n");

    for (i = 0; i < info->num_connections; i++) {
        if (info->connection_status[i] == DLT_GATEWAY_CONNECTED)
            status = DLT_NODE_CONNECTED_STR;
        else
            status = DLT_NODE_DISCONNECTED_STR;

        printf("%.4s: %s\n", &info->node_id[i * DLT_ID_SIZE], status);
    }

    printf("\n");
}

/**
 * @brief Analyze received DLT Daemon response
 *
 * This function checks the received message. In particular, it checks the
 * answer string 'service(\<ID\>, {ok, error, perm_denied})'. In any case the
 * g_callback_return variable will be set as well which is evaluated in the
 * main function after the communication thread returned.
 *
 * @param answer Recieved response
 * @param payload Received DLT Message
 * @param len Length of received DLT message
 * @return 0 if daemon returns 'ok' message, -1 otherwise
 */
static int dlt_passive_node_analyze_response(char *answer,
                                             void *payload,
                                             int len)
{
    int ret = -1;
    char resp_ok[MAX_RESPONSE_LENGTH] = { 0 };

    if ((answer == NULL) || (payload == NULL))
        return -1;

    snprintf(resp_ok,
             MAX_RESPONSE_LENGTH,
             "service(%u), ok",
             get_command());

    pr_verbose("Response received: '%s'\n", answer);
    pr_verbose("Response expected: '%s'\n", resp_ok);

    if (strncmp(answer, resp_ok, strlen(resp_ok)) == 0) {
        ret = 0;

        if (get_command() == DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS) {
            if ((int)sizeof(DltServicePassiveNodeConnectionInfo) > len) {
                pr_error("Received payload is smaller than expected\n");
                pr_verbose("Expected: %zu,\nreceived: %d",
                           sizeof(DltServicePassiveNodeConnectionInfo),
                           len);
                ret = -1;
            }
            else {
                DltServicePassiveNodeConnectionInfo *info =
                    (DltServicePassiveNodeConnectionInfo *)(payload);

                if (info == NULL) {
                    fprintf(stderr, "Received response is NULL\n");
                    return -1;
                }

                dlt_print_passive_node_status(info);
            }
        }
    }

    return ret;
}

/**
 * @brief Prepare message body to be send to DLT Daemon
 *
 * @return Pointer ot DltControlMsgBody, NULL otherwise
 */
DltControlMsgBody *dlt_passive_node_prepare_message_body()
{
    DltControlMsgBody *mb = calloc(1, sizeof(DltControlMsgBody));
    char *ecuid = get_node_id();

    if (mb == NULL)
        return NULL;

    if (get_command() == DLT_SERVICE_ID_PASSIVE_NODE_CONNECT) {
        mb->data = calloc(1, sizeof(DltServicePassiveNodeConnect));

        if (mb->data == NULL) {
            free(mb);
            return NULL;
        }

        mb->size = sizeof(DltServicePassiveNodeConnect);
        DltServicePassiveNodeConnect *serv = (DltServicePassiveNodeConnect *)
            mb->data;
        serv->service_id = DLT_SERVICE_ID_PASSIVE_NODE_CONNECT;
        serv->connection_status = get_connection_state();

        memcpy(serv->node_id, ecuid, DLT_ID_SIZE);
    }
    else { /* DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS */
        mb->data = calloc(1, sizeof(DltServicePassiveNodeConnectionInfo));

        if (mb->data == NULL) {
            free(mb);
            return NULL;
        }

        mb->size = sizeof(DltServicePassiveNodeConnectionInfo);
        DltServicePassiveNodeConnectionInfo *serv =
            (DltServicePassiveNodeConnectionInfo *)mb->data;
        serv->service_id = DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS;
    }

    return mb;
}

/**
 * @brief Destroy message body
 */
void dlt_passive_node_destroy_message_body(DltControlMsgBody *msg_body)
{
    if (msg_body == NULL)
        return;

    if (msg_body->data != NULL)
        free(msg_body->data);

    free(msg_body);
}

/**
 * @brief Send a single command to DLT daemon and wait for response
 *
 * @return 0 on success, -1 on error
 */
static int dlt_passive_node_ctrl_single_request()
{
    int ret = -1;

    /* Initializing the communication with the daemon */
    if (dlt_control_init(dlt_passive_node_analyze_response,
                         get_ecuid(),
                         get_verbosity()) != 0) {
        pr_error("Failed to initialize connection with the daemon.\n");
        return ret;
    }

    /* prepare message body */
    DltControlMsgBody *msg_body = NULL;
    msg_body = dlt_passive_node_prepare_message_body();

    if (msg_body == NULL) {
        pr_error("Data for Dlt Message body is NULL\n");
        return ret;
    }

    ret = dlt_control_send_message(msg_body, get_timeout());

    dlt_passive_node_destroy_message_body(msg_body);

    dlt_control_deinit();

    return ret;
}

static void usage()
{
    printf("Usage: dlt-passive-node-ctrl [options]\n");
    printf("Send a trigger to DLT daemon to (dis)connect a passive node "
           "or get current passive node status \n");
    printf("\n");
    printf("Options:\n");
    printf("  -c         Connection status (1 - connect, 0 - disconnect)\n");
    printf("  -h         Usage\n");
    printf("  -n         passive Node identifier (e.g. ECU2)\n");
    printf("  -s         Show passive node(s) connection status\n");
    printf("  -t         Specify connection timeout (Default: %ds)\n",
           DLT_CTRL_TIMEOUT);
    printf("  -S         Send message with serial header (Default: Without serial header)\n");
    printf("  -R         Enable resync serial header\n");
    printf("  -v         Set verbose flag (Default:%d)\n", get_verbosity());
}

/**
 * @brief Parse application arguments
 *
 * The arguments are parsed and saved in static structure for future use.
 *
 * @param argc  amount of arguments
 * @param argv  argument table
 * @return 0 on success, -1 otherwise
 */
static int parse_args(int argc, char *argv[])
{
    int c = 0;
    int state = -1;

    /* Get command line arguments */
    opterr = 0;

    while ((c = getopt(argc, argv, "c:hn:st:SRv")) != -1)
        switch (c) {
        case 'c':
            state = (int)strtol(optarg, NULL, 10);

            if ((state == DLT_NODE_CONNECT) || (state == DLT_NODE_DISCONNECT)) {
                set_connection_state((unsigned int) state);
                set_command(DLT_SERVICE_ID_PASSIVE_NODE_CONNECT);
            }
            else {
                pr_error("unknown connection state: %d\n", state);
                return -1;
            }

            break;
        case 'h':
            usage();
            return -1;
        case 'n':
            set_node_id(optarg);
            break;
        case 's':
            set_command(DLT_SERVICE_ID_PASSIVE_NODE_CONNECTION_STATUS);
            break;
        case 't':
            set_timeout((int) strtol(optarg, NULL, 10));
            break;
        case 'S':
        {
            set_send_serial_header(1);
            break;
        }
        case 'R':
        {
            set_resync_serial_header(1);
            break;
        }
        case 'v':
            set_verbosity(1);
            pr_verbose("Now in verbose mode.\n");
            break;
        case '?':

            if (isprint(optopt))
                pr_error("Unknown option -%c.\n", optopt);
            else
                pr_error("Unknown option character \\x%x.\n", optopt);

            usage();
            return -1;
        default:
            pr_error("Try %s -h for more information.\n", argv[0]);
            return -1;
        }

    return 0;
}

/**
 * @brief Entry point
 *
 * Execute the argument parser and call the main feature accordingly
 *
 * @param argc  amount of arguments
 * @param argv  argument table
 * @return 0 on success, -1 otherwise
 */
int main(int argc, char *argv[])
{
    int ret = 0;

    set_ecuid(NULL);
    set_timeout(DLT_CTRL_TIMEOUT);
    set_send_serial_header(0);
    set_resync_serial_header(0);

    /* Get command line arguments */
    if (parse_args(argc, argv) != 0)
        return -1;

    if ((get_command() == UNDEFINED) ||
        ((get_command() == DLT_SERVICE_ID_PASSIVE_NODE_CONNECT) &&
         (g_options.node_id[0] == '\0') &&
         (g_options.connection_state == DLT_NODE_CONNECT_UNDEF))) {
        pr_error("No valid parameter configuration given!\n");
        usage();
        return -1;
    }

    pr_verbose("Sending command to DLT daemon.\n");

    /* one shot request */
    ret = dlt_passive_node_ctrl_single_request();

    pr_verbose("Exiting.\n");

    return ret;
}
