/**
 * @licence app begin@
 * Copyright (C) 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-control-common.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-control-common.c                                          **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christoph Lipka clipka@jp.adit-jv.com                         **
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
**  cl          Christoph Lipka            ADIT                               **
**  fb          Frederic Berat             ADIT                               **
*******************************************************************************/
#define pr_fmt(fmt) "Common control: "fmt

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dlt_common.h"
#include "dlt_protocol.h"
#include "dlt_client.h"

#include "dlt-control-common.h"

#define DLT_CTRL_APID    "DLTC"
#define DLT_CTRL_CTID    "DLTC"
#define DLT_DAEMON_DEFAULT_CTRL_SOCK_PATH "/tmp/dlt-ctrl.sock"

/** @brief Analyze the daemon answer
 *
 * This function as to be provided by the user of the connection.
 *
 * @param answer  The textual answer of the daemon
 * @param payload The daemons answer payload
 * @param length  The daemons answer payload length
 *
 * @return User defined.
 */
static int (*response_analyzer_cb)(char *, void *, int);

static pthread_t daemon_connect_thread;
static DltClient g_client;
static int callback_return = -1;
static pthread_mutex_t answer_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t answer_cond = PTHREAD_COND_INITIALIZER;

static int local_verbose;
static char local_ecuid[DLT_CTRL_ECUID_LEN]; /* Name of ECU */
static long local_timeout;

int get_verbosity(void)
{
    return local_verbose;
}

void set_verbosity(int v)
{
    local_verbose = !!v;
}

char *get_ecuid(void)
{
    return local_ecuid;
}

void set_ecuid(char *ecuid)
{
    char *ecuid_conf = NULL;

    if (local_ecuid != ecuid) {
        /* If user pass NULL, read ECUId from dlt.conf */
        if (ecuid == NULL) {
            if (dlt_parse_config_param("ECUId", &ecuid_conf) == 0) {
                memset(local_ecuid, 0, DLT_CTRL_ECUID_LEN);
                strncpy(local_ecuid, ecuid_conf, DLT_CTRL_ECUID_LEN);
                local_ecuid[DLT_CTRL_ECUID_LEN - 1] = '\0';
            }
            else {
                pr_error("Cannot read ECUid from dlt.conf\n");
            }
        }
        else {
            /* Set user passed ECUID */
            memset(local_ecuid, 0, DLT_CTRL_ECUID_LEN);
            strncpy(local_ecuid, ecuid, DLT_CTRL_ECUID_LEN);
            local_ecuid[DLT_CTRL_ECUID_LEN - 1] = '\0';
        }
    }
}

long get_timeout(void)
{
    return local_timeout;
}

void set_timeout(long t)
{
    local_timeout = DLT_CTRL_TIMEOUT;

    if (t > 1)
        local_timeout = t;
    else
        pr_error("Timeout to small. Set to default: %d",
                 DLT_CTRL_TIMEOUT);
}

int dlt_parse_config_param(char *config_id, char **config_data)
{
    FILE *pFile = NULL;
    int value_length = DLT_LINE_LEN;
    char line[DLT_LINE_LEN - 1] = { 0 };
    char token[DLT_LINE_LEN] = { 0 };
    char value[DLT_LINE_LEN] = { 0 };
    char *pch = NULL;
    const char *filename = NULL;

    if (*config_data != NULL)
        *config_data = NULL;

    /* open configuration file */
    filename = CONFIGURATION_FILES_DIR "/dlt.conf";
    pFile = fopen(filename, "r");

    if (pFile != NULL) {
        while (1) {
            /* fetch line from configuration file */
            if (fgets(line, value_length - 1, pFile) != NULL) {
                if (strncmp(line, config_id, strlen(config_id)) == 0) {
                    pch = strtok(line, " =\r\n");
                    token[0] = 0;
                    value[0] = 0;

                    while (pch != NULL) {
                        if (token[0] == 0) {
                            strncpy(token, pch, sizeof(token) - 1);
                            token[sizeof(token) - 1] = 0;
                        }
                        else {
                            strncpy(value, pch, sizeof(value) - 1);
                            value[sizeof(value) - 1] = 0;
                            break;
                        }

                        pch = strtok(NULL, " =\r\n");
                    }

                    if (token[0] && value[0]) {
                        if (strcmp(token, config_id) == 0) {
                            *(config_data) = (char *)
                                calloc(DLT_DAEMON_FLAG_MAX, sizeof(char));
                            memcpy(*config_data,
                                   value,
                                   DLT_DAEMON_FLAG_MAX - 1);
                        }
                    }
                }
            }
            else {
                break;
            }
        }

        fclose (pFile);
    }
    else {
        fprintf(stderr, "Cannot open configuration file: %s\n", filename);
    }

    if (*config_data == NULL)
        return -1;

    return 0;
}

/** @brief Send a message to the daemon through the socket.
 *
 * The socket as to be opened and active before sending.
 *
 * @param sock The socket to send the message through
 * @param msg The message to be send in DLT format.
 *
 * @return 0 on success, -1 otherwise.
 */
static int dlt_control_send_message_to_socket(int sock, DltMessage *msg)
{
    if (send(sock,
             (const char *)(msg->headerbuffer + sizeof(DltStorageHeader)),
             msg->headersize - sizeof(DltStorageHeader), 0) == -1) {
        pr_error("Sending message header failed: %s\n", strerror(errno));
        return -1;
    }

    if (send(sock, (const char *)msg->databuffer, msg->datasize, 0) == -1) {
        pr_error("Sending message failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/** @brief Prepare the extra headers of a DLT message
 *
 * Modifies the extra headers of the message so that it can be sent.
 *
 * @param msg The message to be prepared
 * @param header The base header to be used.
 *
 * @return 0 on success, -1 otherwise.
 */
static int prepare_extra_headers(DltMessage *msg, uint8_t *header)
{
    int shift = 0;

    pr_verbose("Preparing extra headers.\n");

    if (!msg || !header)
        return -1;

    shift = sizeof(DltStorageHeader) +
        sizeof(DltStandardHeader) +
        DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp);

    /* Set header extra parameters */
    dlt_set_id(msg->headerextra.ecu, get_ecuid());

    msg->headerextra.tmsp = dlt_uptime();

    /* Copy header extra parameters to header buffer */
    if (dlt_message_set_extraparameters(msg, get_verbosity()) == -1) {
        pr_error("Cannot copy header extra parameter\n");
        return -1;
    }

    /* prepare extended header */
    msg->extendedheader = (DltExtendedHeader *)(header + shift);

    msg->extendedheader->msin = DLT_MSIN_CONTROL_REQUEST;

    msg->extendedheader->noar = 1; /* one payload packet */

    /* Dummy values have to be set */
    dlt_set_id(msg->extendedheader->apid, DLT_CTRL_APID);
    dlt_set_id(msg->extendedheader->ctid, DLT_CTRL_CTID);

    return 0;
}

/** @brief Prepare the headers of a DLT message
 *
 * Modifies the headers of the message so that it can be sent.
 *
 * @param msg The message to be prepared
 * @param header The base header to be used.
 *
 * @return 0 on success, -1 otherwise.
 */
static int prepare_headers(DltMessage *msg, uint8_t *header)
{
    uint32_t len = 0;

    pr_verbose("Preparing headers.\n");

    if (!msg || !header)
        return -1;

    msg->storageheader = (DltStorageHeader *)header;

    if (dlt_set_storageheader(msg->storageheader, "") == -1) {
        pr_error("Storage header initialization failed.\n");
        return -1;
    }

    /* prepare standard header */
    msg->standardheader =
        (DltStandardHeader *)(header + sizeof(DltStorageHeader));

    msg->standardheader->htyp = DLT_HTYP_WEID |
        DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1;

#if (BYTE_ORDER == BIG_ENDIAN)
    msg->standardheader->htyp = (msg->standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg->standardheader->mcnt = 0;

    /* prepare length information */
    msg->headersize = sizeof(DltStorageHeader) +
        sizeof(DltStandardHeader) +
        sizeof(DltExtendedHeader) +
        DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp);

    len = msg->headersize - sizeof(DltStorageHeader) + msg->datasize;

    if (len > UINT16_MAX) {
        pr_error("Message header is too long.\n");
        return -1;
    }

    msg->standardheader->len = DLT_HTOBE_16(len);

    return 0;
}

/** @brief Prepare a DLT message.
 *
 * The DLT message is built using the data given by the user.
 * The data is basically composed of a buffer and a size.
 *
 * @param data The message body to be used to build the DLT message.
 *
 * @return 0 on success, -1 otherwise.
 */
static DltMessage *dlt_control_prepare_message(DltControlMsgBody *data)
{
    DltMessage *msg = NULL;

    pr_verbose("Preparing message.\n");

    if (data == NULL) {
        pr_error("Data for message body is NULL\n");
        return NULL;
    }

    msg = calloc(1, sizeof(DltMessage));

    if (msg == NULL) {
        pr_error("Cannot allocate memory for Dlt Message\n");
        return NULL;
    }

    if (dlt_message_init(msg, get_verbosity()) == -1) {
        pr_error("Cannot initialize Dlt Message\n");
        free(msg);
        return NULL;
    }

    /* prepare payload of data */
    msg->databuffersize = msg->datasize = data->size;

    /* Allocate memory for Dlt Message's buffer */
    msg->databuffer = (uint8_t *)calloc(1, data->size);

    if (msg->databuffer == NULL) {
        pr_error("Cannot allocate memory for data buffer\n");
        free(msg);
        return NULL;
    }

    /* copy data into message */
    memcpy(msg->databuffer, data->data, data->size);

    /* prepare storage header */
    if (prepare_headers(msg, msg->headerbuffer)) {
        dlt_message_free(msg, get_verbosity());
        free(msg);
        return NULL;
    }

    /* prepare extra headers */
    if (prepare_extra_headers(msg, msg->headerbuffer)) {
        dlt_message_free(msg, get_verbosity());
        free(msg);
        return NULL;
    }

    return msg;
}

/** @brief Initialize the connection with the daemon
 *
 * The connection is initialized using an internal callback. The user's
 * response analyzer will be finally executed by this callback.
 * The client pointer is used to established the connection.
 *
 * @param client A pointer to a valid client structure
 * @param cb The internal callback to be executed while receiving a new message
 *
 * @return 0 on success, -1 otherwise.
 */
static int dlt_control_init_connection(DltClient *client, void *cb)
{
    int (*callback)(DltMessage *message, void *data) = cb;

    if (!cb || !client) {
        pr_error("%s Invalid parameters (%p, %p)\n", __func__, client, cb);
        return -1;
    }

    pr_verbose("Initializing the connection.\n");

    if (dlt_client_init(client, get_verbosity()) != 0) {
        pr_error("Failed to register callback (NULL)\n");
        return -1;
    }

    dlt_client_register_message_callback(callback);

    client->socketPath = NULL;

    if (dlt_parse_config_param("ControlSocketPath", &client->socketPath) != 0) {
        /* Failed to read from conf, copy default */
        if (dlt_client_set_socket_path(client, DLT_DAEMON_DEFAULT_CTRL_SOCK_PATH) == -1) {
            pr_error("set socket path didn't succeed\n");
            return -1;
        }
    }

    client->mode = DLT_CLIENT_MODE_UNIX;

    return dlt_client_connect(client, get_verbosity());
}

/** @brief Daemon listener function
 *
 * This function will continuously read on the DLT socket, until an error occurs
 * or the thread executing this function is canceled.
 *
 * @param data Thread parameter
 *
 * @return The thread parameter given as argument.
 */
static void *dlt_control_listen_to_daemon(void *data)
{
    pr_verbose("Ready to receive DLT answers.\n");
    dlt_client_main_loop(&g_client, NULL, get_verbosity());
    return data;
}

/** @brief Internal callback for DLT response
 *
 * This function is called by the dlt_client_main_loop once a response is read
 * from the DLT socket.
 * After some basic checks, the user's response analyzer is called. The return
 * value of the analyzer is then provided back to the dlt_control_send_message
 * function to be given back as a return value.
 * As this function is called in a dedicated thread, the return value is
 * provided using a global variable.
 * Access to this variable is controlled through a dedicated mutex.
 * New values are signaled using a dedicated condition variable.
 *
 * @param message The DLT answer
 * @data Unused
 *
 * @return The analyzer return value or -1 on early errors.
 */
static int dlt_control_callback(DltMessage *message, void *data)
{
    char text[DLT_RECEIVE_BUFSIZE] = { 0 };
    (void)data;

    if (message == NULL) {
        pr_error("Received message is null\n");
        return -1;
    }

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    else
        dlt_set_storageheader(message->storageheader, "LCTL");

    dlt_message_header(message, text, DLT_RECEIVE_BUFSIZE, get_verbosity());

    /* Extracting payload */
    dlt_message_payload(message, text,
                        DLT_RECEIVE_BUFSIZE,
                        DLT_OUTPUT_ASCII,
                        get_verbosity());

    /*
     * Checking payload with the provided callback and return the result
     */
    pthread_mutex_lock(&answer_lock);
    callback_return = response_analyzer_cb(text,
                                           message->databuffer,
                                           message->datasize);
    pthread_cond_signal(&answer_cond);
    pthread_mutex_unlock(&answer_lock);

    return callback_return;
}

/** @brief Send a message to the daemon and wait for the asynchronous answer.
 *
 * The answer is received and analyzed by a dedicated thread. Thus we need
 * to wait for the signal from this thread and then read the return value
 * to be provided to the user.
 * In case of timeout, this function fails.
 * The message provided by the user is formated in DLT format before sending.
 *
 * @param body The message provided by the user
 * @param timeout The time to wait before considering that no answer will come
 *
 * @return The user response analyzer return value, -1 in case of early error.
 */
int dlt_control_send_message(DltControlMsgBody *body, int timeout)
{
    struct timespec t;
    DltMessage *msg = NULL;

    if (!body) {
        pr_error("Invalid input (%p).\n", body);
        return -1;
    }

    if (clock_gettime(CLOCK_REALTIME, &t) == -1) {
        pr_error("Cannot read system time.\n");
        return -1;
    }

    t.tv_sec += timeout;

    /* send command to daemon here */
    msg = dlt_control_prepare_message(body);

    if (msg == NULL) {
        pr_error("Control message preparation failed\n");
        return -1;
    }

    pthread_mutex_lock(&answer_lock);

    /* Re-init the return value */
    callback_return = -1;

    if (dlt_control_send_message_to_socket(g_client.sock, msg) != 0) {
        pr_error("Sending message to daemon failed\n");
        free(msg);
        return -1;
    }

    /* If we timeout the lock is not taken back */
    if (!pthread_cond_timedwait(&answer_cond, &answer_lock, &t))
        pthread_mutex_unlock(&answer_lock);

    /* Destroying the message */
    dlt_message_free(msg, get_verbosity());
    free(msg);

    /* At this point either the value is already correct, either it's still -1.
     * Then, we don't care to lock the access.
     */
    return callback_return;
}

/** @brief Control communication initialization
 *
 * This will prepare the DLT connection and the thread dedicated to the
 * response listening.
 *
 * @param response_analyzer User defined function used to analyze the response
 * @param ecuid The ECUID to provide to the daemon
 * @param verbosity The verbosity level
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_control_init(int (*response_analyzer)(char *, void *, int),
                     char *ecuid,
                     int verbosity)
{
    if (!response_analyzer || !ecuid) {
        pr_error("Invalid input (%p %p).\n", response_analyzer, ecuid);
        return -1;
    }

    response_analyzer_cb = response_analyzer;
    set_ecuid(ecuid);
    set_verbosity(verbosity);

    if (dlt_control_init_connection(&g_client, dlt_control_callback) != 0) {
        pr_error("Connection initialization failed\n");
        dlt_client_cleanup(&g_client, get_verbosity());
        return -1;
    }

    /* Contact DLT daemon */
    if (pthread_create(&daemon_connect_thread,
                       NULL,
                       dlt_control_listen_to_daemon,
                       NULL) != 0) {
        pr_error("Cannot create thread to communicate with DLT daemon.\n");
        return -1;
    }

    return 0;
}

/** @brief Control communication clean-up
 *
 * Cancels the listener thread and clean=up the dlt client structure.
 *
 * @return 0 on success, -1 otherwise.
 */
int dlt_control_deinit(void)
{
    /* Stopping the listener thread */
    pthread_cancel(daemon_connect_thread);
    /* Closing the socket */
    return dlt_client_cleanup(&g_client, get_verbosity());
}
