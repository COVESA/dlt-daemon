/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_client.h
*/


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_client.h                                                  **
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
 * $LastChangedRevision$
 * $LastChangedDate$
 * $LastChangedBy$
 */

#ifndef DLT_CLIENT_H
#define DLT_CLIENT_H

/**
  \defgroup clientapi DLT Client API
  \addtogroup clientapi
  \{
*/

#include "dlt_types.h"
#include "dlt_common.h"

typedef enum
{
    DLT_CLIENT_MODE_UNDEFINED = -1,
    DLT_CLIENT_MODE_TCP,
    DLT_CLIENT_MODE_SERIAL,
    DLT_CLIENT_MODE_UNIX
} DltClientMode;

typedef struct
{
    DltReceiver receiver;  /**< receiver pointer to dlt receiver structure */
    int sock;              /**< sock Connection handle/socket */
    char *servIP;          /**< servIP IP adress/Hostname of TCP/IP interface */
    int port;              /**< Port for TCP connections (optional) */
    char *serialDevice;    /**< serialDevice Devicename of serial device */
    char *socketPath;      /**< socketPath Unix socket path */
    char ecuid[4];           /**< ECUiD */
    speed_t baudrate;      /**< baudrate Baudrate of serial interface, as speed_t */
    DltClientMode mode;    /**< mode DltClientMode */
} DltClient;

#ifdef __cplusplus
extern "C" {
#endif

void dlt_client_register_message_callback(int (*registerd_callback) (DltMessage *message, void *data));

/**
 * Initialising dlt client structure with a specific port
 * @param client pointer to dlt client structure
 * @param port The port for the tcp connection
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_client_init_port(DltClient *client, int port, int verbose);

/**
 * Initialising dlt client structure
 * @param client pointer to dlt client structure
 * @param verbose if set to true verbose information is printed out.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_init(DltClient *client, int verbose);
/**
 * Connect to dlt daemon using the information from the dlt client structure
 * @param client pointer to dlt client structure
 * @param verbose if set to true verbose information is printed out.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_connect(DltClient *client, int verbose);
/**
 * Cleanup dlt client structure
 * @param client pointer to dlt client structure
 * @param verbose if set to true verbose information is printed out.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_cleanup(DltClient *client, int verbose);
/**
 * Main Loop of dlt client application
 * @param client pointer to dlt client structure
 * @param data pointer to data to be provided to the main loop
 * @param verbose if set to true verbose information is printed out.
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_main_loop(DltClient *client, void *data, int verbose);
/**
 * Send ancontrol message to the dlt daemon
 * @param client pointer to dlt client structure
 * @param apid application id
 * @param ctid context id
 * @param payload Buffer filled with control message data
 * @param size Size of control message data
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_ctrl_msg(DltClient *client, char *apid, char *ctid, uint8_t *payload, uint32_t size);
/**
 * Send an injection message to the dlt daemon
 * @param client pointer to dlt client structure
 * @param apid application id
 * @param ctid context id
 * @param serviceID service id
 * @param buffer Buffer filled with injection message data
 * @param size Size of injection data within buffer
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_inject_msg(DltClient *client, char *apid, char *ctid, uint32_t serviceID, uint8_t *buffer, uint32_t size);
/**
 * Send an set  log level message to the dlt daemon
 * @param client pointer to dlt client structure
 * @param apid application id
 * @param ctid context id
 * @param SendLogLevel Log Level
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_log_level(DltClient *client, char *apid, char *ctid, uint8_t logLevel);
/**
 * Send an request to get log info message to the dlt daemon
 * @param client pointer to dlt client structure
 * @return negative value if there was an error
 */
int dlt_client_get_log_info(DltClient *client);
/**
 * Initialise get log info structure
 * @param void
 * @return void
 */
void dlt_getloginfo_init( void );
/**
 * To free the memory allocated for app description in get log info
 * @param void
 * @return void
 */
void dlt_getloginfo_free( void );
/**
 * Send a set trace status message to the dlt daemon
 * @param client pointer to dlt client structure
 * @param apid application id
 * @param ctid context id
 * @param defaultTraceStatus Default Trace Status
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_trace_status(DltClient *client, char *apid, char *ctid, uint8_t traceStatus);
/**
 * Send the default log level to the dlt daemon
 * @param client pointer to dlt client structure
 * @param defaultLogLevel Default Log Level
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_default_log_level(DltClient *client, uint8_t defaultLogLevel);
/**
 * Send the log level to all contexts registered with dlt daemon
 * @param client pointer to dlt client structure
 * @param LogLevel Log Level to be set
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_all_log_level(DltClient *client, uint8_t LogLevel);
/**
 * Send the default trace status to the dlt daemon
 * @param client pointer to dlt client structure
 * @param defaultTraceStatus Default Trace Status
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_default_trace_status(DltClient *client, uint8_t defaultTraceStatus);
/**
 * Send the trace status to all contexts registered with dlt daemon
 * @param client pointer to dlt client structure
 * @param traceStatus trace status to be set
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_all_trace_status(DltClient *client, uint8_t traceStatus);
/**
 * Send the timing pakets status to the dlt daemon
 * @param client pointer to dlt client structure
 * @param timingPakets Timing pakets enabled
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_timing_pakets(DltClient *client, uint8_t timingPakets);
/**
 * Send the store config command to the dlt daemon
 * @param client pointer to dlt client structure
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_store_config(DltClient *client);
/**
 * Send the reset to factory default command to the dlt daemon
 * @param client pointer to dlt client structure
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_reset_to_factory_default(DltClient *client);

/**
 * Set baudrate within dlt client structure
 * @param client pointer to dlt client structure
 * @param baudrate Baudrate
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_setbaudrate(DltClient *client, int baudrate);

/**
 * Set server ip
 * @param client pointer to dlt client structure
 * @param pointer to command line argument
 * @return negative value if there was an error
 */
int dlt_client_set_server_ip(DltClient *client, char *ipaddr);

/**
 * Set serial device
 * @client pointer to dlt client structure
 * @param param pointer to command line argument
 * @return negative value if there was an error
 */
int dlt_client_set_serial_device(DltClient *client, char *serial_device);

/**
 * Set socket path
 * @client pointer to dlt client structure
 * @param param pointer to socket path string
 * @return negative value if there was an error
 */
int dlt_client_set_socket_path(DltClient *client, char *socket_path);

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_CLIENT_H */
