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

typedef struct
{
    DltReceiver receiver;  /**< receiver pointer to dlt receiver structure */
    int sock;              /**< sock Connection handle/socket */
    char *servIP;          /**< servIP IP adress/Hostname of TCP/IP interface */
    char *serialDevice;    /**< serialDevice Devicename of serial device */
    speed_t baudrate;      /**< baudrate Baudrate of serial interface, as speed_t */
    int serial_mode;       /**< serial_mode Serial mode enabled =1, disabled =0 */
} DltClient;

#ifdef __cplusplus
extern "C" {
#endif

void dlt_client_register_message_callback(int (*registerd_callback) (DltMessage *message, void *data));

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
 * Send the default trace status to the dlt daemon
 * @param client pointer to dlt client structure
 * @param defaultTraceStatus Default Trace Status
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_client_send_default_trace_status(DltClient *client, uint8_t defaultTraceStatus);
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

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_CLIENT_H */
