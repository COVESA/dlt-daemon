/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2018 Advanced Driver Information Technology.
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

/*!
 * \author
 * Aditya Paluri <venkataaditya.paluri@in.bosch.com>
 *
 * \copyright Copyright © 2018 Advanced Driver Information Technology. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_gateway_internal.h
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_gateway_internal.h                                        **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Aditya Paluri venkataaditya.paluri@in.bosch.com               **
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
**  ap          Aditya Paluri              ADIT                               **
*******************************************************************************/

#ifndef DLT_GATEWAY_INTERNAL_H_
#define DLT_GATEWAY_INTERNAL_H_

DLT_STATIC DltReturnValue dlt_gateway_check_ip(DltGatewayConnection *con,
                                               char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_port(DltGatewayConnection *con,
                                                 char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_ecu(DltGatewayConnection *con,
                                                char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_connect_trigger(DltGatewayConnection *con,
                                                            char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_timeout(DltGatewayConnection *con,
                                                    char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_send_serial(DltGatewayConnection *con,
                                                        char *value);

DLT_STATIC DltReturnValue dlt_gateway_allocate_control_messages(DltGatewayConnection *con);

DLT_STATIC DltReturnValue dlt_gateway_check_control_messages(DltGatewayConnection *con,
                                                             char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_periodic_control_messages(DltGatewayConnection *con,
                                                                      char *value);

DLT_STATIC DltReturnValue dlt_gateway_check_param(DltGateway *gateway,
                                                  DltGatewayConnection *con,
                                                  DltGatewayConfType ctype,
                                                  char *value);

int dlt_gateway_configure(DltGateway *gateway, char *config_file, int verbose);

int dlt_gateway_store_connection(DltGateway *gateway,
                                 DltGatewayConnection *tmp,
                                 int verbose);

DLT_STATIC DltReturnValue dlt_gateway_parse_get_log_info(DltDaemon *daemon,
                                                         char *ecu,
                                                         DltMessage *msg,
                                                         int req,
                                                         int verbose);

#endif
