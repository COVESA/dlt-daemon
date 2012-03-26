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
 * \author Alexander Wenzel <alexander.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt_client.h
 * For further information see http://www.genivi.org/.
 * @licence end@
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
 * @return negative value if there was an error
 */
int dlt_client_init(DltClient *client, int verbose);
/**
 * Connect to dlt daemon using the information from the dlt client structure
 * @param client pointer to dlt client structure
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_client_connect(DltClient *client, int verbose);
/**
 * Cleanup dlt client structure
 * @param client pointer to dlt client structure
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_client_cleanup(DltClient *client, int verbose);
/**
 * Main Loop of dlt client application
 * @param client pointer to dlt client structure
 * @param data pointer to data to be provided to the main loop
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_client_main_loop(DltClient *client, void *data, int verbose);
/**
 * Send an injection message to the dlt daemon
 * @param client pointer to dlt client structure
 * @param apid application id
 * @param ctid context id
 * @param serviceID service id
 * @param buffer Buffer filled with injection message data
 * @param size Size of injection data within buffer
 * @return negative value if there was an error
 */
int dlt_client_send_inject_msg(DltClient *client, char *apid, char *ctid, uint32_t serviceID, uint8_t *buffer, uint32_t size);
/**
 * Set baudrate within dlt client structure
 * @param client pointer to dlt client structure
 * @param baudrate Baudrate
 * @return negative value if there was an error
 */
int dlt_client_setbaudrate(DltClient *client, int baudrate);

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_CLIENT_H */
