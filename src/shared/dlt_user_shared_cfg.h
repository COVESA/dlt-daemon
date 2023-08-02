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
 * \file dlt_user_shared_cfg.h
 */



/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_shared_cfg.h                                         **
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

#ifndef DLT_USER_SHARED_CFG_H
#define DLT_USER_SHARED_CFG_H

/*************/
/* Changable */
/*************/

/************************/
/* Don't change please! */
/************************/

/* The different types of internal messages between user application and daemon. */
#define DLT_USER_MESSAGE_LOG 1
#define DLT_USER_MESSAGE_REGISTER_APPLICATION 2
#define DLT_USER_MESSAGE_UNREGISTER_APPLICATION 3
#define DLT_USER_MESSAGE_REGISTER_CONTEXT 4
#define DLT_USER_MESSAGE_UNREGISTER_CONTEXT 5
#define DLT_USER_MESSAGE_LOG_LEVEL 6
#define DLT_USER_MESSAGE_INJECTION 7
#define DLT_USER_MESSAGE_OVERFLOW 8
#define DLT_USER_MESSAGE_APP_LL_TS 9
#define DLT_USER_MESSAGE_LOG_SHM 10
#define DLT_USER_MESSAGE_LOG_MODE 11
#define DLT_USER_MESSAGE_LOG_STATE 12
#define DLT_USER_MESSAGE_MARKER 13
#define DLT_USER_MESSAGE_NOT_SUPPORTED 16

/* Internal defined values */

/* must be different from DltLogLevelType */
#define DLT_USER_LOG_LEVEL_NOT_SET    -2
/* must be different from DltTraceStatusType */
#define DLT_USER_TRACE_STATUS_NOT_SET -2

#endif /* DLT_USER_SHARED_CFG_H */

