/*
 * Dlt- Diagnostic Log and Trace client library
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
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
