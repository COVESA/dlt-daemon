/**
 * Copyright (C) 2013 - 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO. *
 * This file is part of COVESA Project Dlt - Diagnostic Log and Trace console apps.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Syed Hameed <shameed@jp.adit-jv.com> ADIT 2013 - 2015
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-logstorage-ctrl.c
 * For further information see http://www.covesa.org/.
 */
/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-logstorage-ctrl.c                                         **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Syed Hameed shameed@jp.adit-jv.com                            **
**              Christoph Lipka clipka@jp.adit-jv.com                         **
**              AnithaAmmaji.baggam@in.bosch.com                              **
**              Frederic Berat fberat@de.adit-jv.com                          **
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
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  sh          Syed Hameed                ADIT                               **
**  cl          Christoph Lipka            ADIT                               **
**  BA          Anitha BA                  ADIT                               **
**  fb          Frederic Berat             ADIT                               **
*******************************************************************************/

#define pr_fmt(fmt) "Logstorage control: "fmt

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>

#include <poll.h>

#if defined(__linux__)
#   include "sd-daemon.h"
#endif

#include "dlt_protocol.h"
#include "dlt_client.h"
#include "dlt-control-common.h"
#include "dlt-logstorage-common.h"
#include "dlt-logstorage-ctrl.h"

#define POLL_TIME_OUT   500
#define EV_MASK_REJECTED (POLLERR | POLLHUP | POLLNVAL)

#define DLT_LOGSTORAGE_CTRL_EXIT 1
static int must_exit;
struct dlt_event {
    struct pollfd pfd;
    void *func;
};

/** @brief Triggers the application exit
 *
 * The application will exit on next poll timeout.
 */
void dlt_logstorage_exit(void)
{
    must_exit = DLT_LOGSTORAGE_CTRL_EXIT;
}

/** @brief Check if the application must exit
 *
 * The application will exit on next poll timeout.
 */
int dlt_logstorage_must_exit(void)
{
    return must_exit;
}

/** @brief Signal handler.
 *
 * Triggers the exit of the application in case of specific signals
 *
 * @param signo The value of the signal received.
 */
static void catch_signal(int signo)
{
    if (signo) {
        pr_error("Signal %d received, exiting.", signo);
        dlt_logstorage_exit();
    }
}

/** @brief Install a handler for some signals
 *
 * Handler are installed on exit related signals. That allows to exit from
 * the main loop gracefully.
 */
static void install_signal_handler(void)
{
    int signals[] = { SIGINT, SIGQUIT, SIGTERM, 0 };
    unsigned int i;
    struct sigaction sa;

    pr_verbose("Installing signal handler.\n");

    /* install a signal handler for the above listed signals */
    for (i = 0; signals[i]; i++) {
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = catch_signal;

        if (sigaction(signals[i], &sa, NULL) < 0)
            pr_error("Failed to install signal %u handler. Error: %s\n",
                     signals[i], strerror(errno));
    }
}

#define MAX_RESPONSE_LENGTH 32
/** @brief Analyze the daemon answer to a request
 *
 * This function checks whether if the daemon answered positively to
 * the request or not.
 *
 * @param data    The textual answer
 * @param payload The answer payload
 * @param len     The answer payload length
 * @return 0 on success, -1 otherwise.
 */
static int analyze_response(char *data, void *payload, int len)
{
    int ret = -1;
    char resp_ok[MAX_RESPONSE_LENGTH] = { 0 };
    char resp_warning[MAX_RESPONSE_LENGTH] = { 0 };
    char resp_perm_denied[MAX_RESPONSE_LENGTH] = { 0 };

    if ((data == NULL) || (payload == NULL))
        return -1;

    /* satisfy compiler */
    (void)payload;
    (void)len;

    snprintf(resp_ok,
             MAX_RESPONSE_LENGTH,
             "service(%d), ok",
             DLT_SERVICE_ID_OFFLINE_LOGSTORAGE);

    snprintf(resp_warning,
             MAX_RESPONSE_LENGTH,
             "service(%d), warning",
             DLT_SERVICE_ID_OFFLINE_LOGSTORAGE);

    snprintf(resp_perm_denied,
             MAX_RESPONSE_LENGTH,
             "service(%d), perm_denied",
             DLT_SERVICE_ID_OFFLINE_LOGSTORAGE);

    if (strncmp(data, resp_ok, strlen(resp_ok)) == 0)
        ret = 0;

    if (strncmp(data, resp_warning, strlen(resp_warning)) == 0) {
        pr_error("Warning:Some filter configurations are ignored due to configuration issues \n");
        ret = 0;
    }

    if (strncmp(data, resp_perm_denied, strlen(resp_perm_denied)) == 0) {
        pr_error("Warning: Permission denied.\n");
        ret = 0;
    }

    pr_verbose("Response received: '%s'\n", data);
    pr_verbose("Response expected: '%s'\n", resp_ok);

    return ret;
}

/** @brief Add a new event to watch
 *
 * This function could be exported to be used by udev/prop so that they can
 * register several events.
 *
 * @param ev_hdl The structure containing the file descriptors
 * @param fd The file descriptor to watch
 * @param cb The callback to be called on event.
 *
 * @return 0 on success, -1 if the parameters are invalid.
 */
static int dlt_logstorage_ctrl_add_event(struct dlt_event *ev_hdl,
                                         int fd,
                                         void *cb)
{
    if ((fd < 0) || !cb || !ev_hdl) {
        pr_error("Wrong parameter to add event (%d %p)\n", fd, cb);
        return -1;
    }

    pr_verbose("Setting up the event handler with (%d, %p).\n", fd, cb);

    ev_hdl->func = cb;
    ev_hdl->pfd.fd = fd;

    return 0;
}

/** @brief Main execution loop
 *
 * Waits on events, and executes the callbacks retrieved
 * back from the event structure.
 *
 * @return 0 on success, -1 otherwise.
 */
static int dlt_logstorage_ctrl_execute_event_loop(struct dlt_event *ev)
{
    int ret = 0;
    int (*callback)() = ev->func;

    ret = poll(&ev->pfd, 1, POLL_TIME_OUT);

    if (ret <= 0) {
        if (errno == EINTR)
            ret = 0;

        if (ret < 0)
            pr_error("poll error: %s\n", strerror(errno));

        return ret;
    }

    if (ev->pfd.revents == 0)
        return 0;

    if (ev->pfd.events & EV_MASK_REJECTED) {
        pr_error("Error while polling. Event received: 0x%x\n", ev->pfd.events);
        /* We only support one event producer.
         * Error means that this producer died.
         */
        pr_error("Now closing fd and exiting.\n");
        close(ev->pfd.fd);
        ev->pfd.fd = -1;
        dlt_logstorage_exit();
        return -1;
    }

    if (!callback) {
        pr_error("Callback not found, exiting.\n");
        dlt_logstorage_exit();
        return -1;
    }

    pr_verbose("Got new event, calling %p.\n", callback);

    if (callback() < 0) {
        pr_error("Error while calling the callback, exiting.\n");
        dlt_logstorage_exit();
        return -1;
    }

    return 0;
}

/** @brief Start event loop and receive messages from DLT.
 *
 * The function will first install the signal handler,
 * then create the poll instance, initialize the communication controller,
 * initialize the event handler and finally start the polling.
 *
 * @return 0 on success, -1 on error
 */
static int dlt_logstorage_ctrl_setup_event_loop(void)
{
    int ret = 0;
    struct dlt_event ev_hdl = {
        .pfd = {
            .fd = -1,
            .events = POLLIN
        }
    };

    install_signal_handler();

    pr_verbose("Creating poll instance.\n");

    /* Initializing the communication with the daemon */
    while (dlt_control_init(analyze_response, get_ecuid(), get_verbosity()) &&
           !dlt_logstorage_must_exit()) {
        pr_error("Failed to initialize connection with the daemon.\n");
        pr_error("Retrying to connect in %ds.\n", get_timeout());
        sleep((unsigned int) get_timeout());
    }

    if (dlt_logstorage_must_exit()) {
        pr_verbose("Exiting.\n");
        return 0;
    }

    pr_verbose("Initializing event generator.\n");

    if (dlt_logstorage_init_handler() < 0) {
        pr_error("Failed to initialize handler.\n");
        dlt_control_deinit();
        return -1;
    }

    if (dlt_logstorage_ctrl_add_event(&ev_hdl,
                                      dlt_logstorage_get_handler_fd(),
                                      dlt_logstorage_get_handler_cb()) < 0) {
        pr_error("add_event error: %s\n", strerror(errno));
        dlt_logstorage_exit();
    }

    while (!dlt_logstorage_must_exit() && (ret == 0))
        ret = dlt_logstorage_ctrl_execute_event_loop(&ev_hdl);

    /* Clean up */
    dlt_logstorage_deinit_handler();
    dlt_control_deinit();

    return ret;
}

/** @brief Send a single command to DLT daemon and wait for response
 *
 * @return 0 on success, -1 otherwise.
 */
static int dlt_logstorage_ctrl_single_request()
{
    int ret = 0;

    /* in case sync all caches, an empty path is given */
    if (get_default_event_type() != EVENT_SYNC_CACHE) {
        /* Check if a 'CONF_NAME' file is present at the given path */
        if (!dlt_logstorage_check_config_file(get_default_path())) {
            pr_error("No '%s' file available at: %s\n",
                     CONF_NAME,
                     get_default_path());
            return -1;
        }

        if (!dlt_logstorage_check_directory_permission(get_default_path())) {
            pr_error("'%s' is not writable\n", get_default_path());
            return -1;
        }
    }

    /* Initializing the communication with the daemon */
    while (dlt_control_init(analyze_response, get_ecuid(), get_verbosity()) &&
           !dlt_logstorage_must_exit()) {
        pr_error("Failed to initialize connection with the daemon.\n");
        pr_error("Retrying to connect in %ds.\n", get_timeout());
        sleep( (unsigned int) get_timeout());
    }

    pr_verbose("event type is [%d]\t device path is [%s]\n",
               get_default_event_type(),
               get_default_path());

    ret = dlt_logstorage_send_event(get_default_event_type(),
                                    get_default_path());

    dlt_control_deinit();

    return ret;
}

/** @brief Print out the application help
 */
static void usage(void)
{
    printf("Usage: dlt-logstorage-ctrl [options]\n");
    printf("Send a trigger to DLT daemon to connect/disconnect"
           "a certain logstorage device\n");
    printf("\n");
    printf("Options:\n");
    printf("  -c --command               Connection type: connect = 1, disconnect = 0\n");
    printf("  -d[prop] --daemonize=prop  Run as daemon: prop = use proprietary handler\n");
    printf("                             'prop' may be replaced by any meaningful word\n");
    printf("                             If prop is not specified, udev will be used\n");
    printf("  -e --ecuid                 Set ECU ID (Default: %s)\n", DLT_CTRL_DEFAULT_ECUID);
    printf("  -h --help                  Usage\n");
    printf("  -p --path                  Mount point path\n");
    printf("  -s[path] --snapshot=path   Sync Logstorage cache\n");
    printf("                             Don't use -s together with -d and -c\n");
    printf("  -t                         Specify connection timeout (Default: %ds)\n",
           DLT_CTRL_TIMEOUT);
    printf("  -S --send-header           Send message with serial header (Default: Without serial header)\n");
    printf("  -R --resync-header         Enable resync serial header\n");
    printf("  -v --verbose               Set verbose flag (Default:%d)\n", get_verbosity());
}

static struct option long_options[] = {
    {"command",       required_argument,  0,  'c'},
    {"daemonize",     optional_argument,  0,  'd'},
    {"ecuid",         required_argument,  0,  'e'},
    {"help",          no_argument,        0,  'h'},
    {"path",          required_argument,  0,  'p'},
    {"snapshot",      optional_argument,  0,  's'},
    {"timeout",       required_argument,  0,  't'},
    {"send-header",   no_argument,        0,  'S'},
    {"resync-header", no_argument,        0,  'R'},
    {"verbose",       no_argument,        0,  'v'},
    {0,               0,                  0,  0}
};

/** @brief Parses the application arguments
 *
 * The arguments are parsed and saved in static structure for future use.
 *
 * @param argc The amount of arguments
 * @param argv The table of arguments
 *
 * @return 0 on success, -1 otherwise
 */
static int parse_args(int argc, char *argv[])
{
    int c = -1;
    int long_index = 0;

    while ((c = getopt_long(argc,
                            argv,
                            ":s::t:hSRe:p:d::c:v",
                            long_options,
                            &long_index)) != -1)
        switch (c) {
        case 's':
            set_default_event_type(EVENT_SYNC_CACHE);

            if ((optarg != NULL) && (strlen(optarg) >= DLT_MOUNT_PATH_MAX)) {
                pr_error("Mount path '%s' too long\n", optarg);
                return -1;
            }

            set_default_path(optarg);
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
        case 'h':
            usage();
            return -1;
        case 'e':
            set_ecuid(optarg);
            break;
        case 'd':
            pr_verbose("Choosing handler.\n");
            set_handler_type(optarg);
            pr_verbose("Handler chosen: %d.\n", get_handler_type());
            break;
        case 'p':

            if (strlen(optarg) >= DLT_MOUNT_PATH_MAX) {
                pr_error("Mount path '%s' too long\n", optarg);
                return -1;
            }

            set_default_path(optarg);
            break;
        case 'c':
            set_default_event_type(strtol(optarg, NULL, 10));
            break;
        case 'v':
            set_verbosity(1);
            pr_verbose("Now in verbose mode.\n");
            break;
        case ':':
            pr_error("Option -%c requires an argument.\n", optopt);
            usage();
            return -1;
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



    if ((get_default_event_type() == EVENT_SYNC_CACHE) &&
        (get_handler_type() != CTRL_NOHANDLER)) {
        pr_error("Sync caches not available in daemon mode\n");
        return -1;
    }

    return 0;
}

#if !defined(DLT_SYSTEMD_ENABLE)
int sd_notify(int unset_environment, const char *state)
{
    /* Satisfy Compiler for warnings */
    (void)unset_environment;
    (void)state;
    return 0;
}
#endif

/** @brief Entry point
 *
 * Execute the argument parser and call the main feature accordingly.
 *
 * @param argc The amount of arguments
 * @param argv The table of arguments
 *
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

    /* all parameter valid, start communication with daemon or setup
     * communication with control daemon */
    if (get_handler_type() == CTRL_NOHANDLER) {
        pr_verbose("One shot.\n");

        ret = dlt_logstorage_ctrl_single_request();

        if (ret < 0)
            pr_error("Message failed to be send. Please check DLT config.\n");
    }
    else {
        pr_verbose("Entering in daemon mode.\n");

        /* Let's daemonize */
        if (sd_notify(0, "READY=1") <= 0) {
            pr_verbose("SD notify failed, manually daemonizing.\n");

            /* No message can be sent or Systemd is not available.
             * Daemonizing manually.
             */
            if (daemon(1, 1)) {
                pr_error("Failed to daemonize: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
        }

        pr_verbose("Executing the event loop\n");
        ret = dlt_logstorage_ctrl_setup_event_loop();
    }

    pr_verbose("Exiting.\n");
    return ret;
}
