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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_client_cfg.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_client_cfg.h                                              **
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
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#ifndef DLT_CLIENT_CFG_H
#define DLT_CLIENT_CFG_H

/*************/
/* Changable */
/*************/

/* Dummy application id of DLT client */
#define DLT_CLIENT_DUMMY_APP_ID "CA1"

/* Dummy context id of DLT client */
#define DLT_CLIENT_DUMMY_CON_ID "CC1"

/* Size of buffer */
#define DLT_CLIENT_TEXTBUFSIZE          512

/* Initial baudrate */
#if !defined (__WIN32__) && !defined(_MSC_VER)
#define DLT_CLIENT_INITIAL_BAUDRATE B115200
#else
#define DLT_CLIENT_INITIAL_BAUDRATE 0
#endif

/* Name of environment variable for specifying the daemon port */
#define DLT_CLIENT_ENV_DAEMON_TCP_PORT "DLT_DAEMON_TCP_PORT"

/************************/
/* Don't change please! */
/************************/

#endif /* DLT_CLIENT_CFG_H */
