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
 * \file dlt_client_cfg.h
 * For further information see http://www.genivi.org/.
 * @licence end@
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

/* Size of receive buffer */
#define DLT_CLIENT_RCVBUFSIZE         10024 

/* Initial baudrate */
#if !defined (__WIN32__) && !defined(_MSC_VER)
#define DLT_CLIENT_INITIAL_BAUDRATE B115200
#else
#define DLT_CLIENT_INITIAL_BAUDRATE 0
#endif

/************************/
/* Don't change please! */
/************************/

#endif /* DLT_CLIENT_CFG_H */
