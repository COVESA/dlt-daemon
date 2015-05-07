/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG"
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
 * \copyright Copyright © 2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_types.h
*/


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_types.h                                             **
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

#ifndef DLT_TYPES_H
#define DLT_TYPES_H

#ifdef _MSC_VER
   typedef __int64 int64_t;
   typedef __int32 int32_t;
   typedef __int16 int16_t;
   typedef __int8  int8_t;

   typedef unsigned __int64 uint64_t;
   typedef unsigned __int32 uint32_t;
   typedef unsigned __int16 uint16_t;
   typedef unsigned __int8  uint8_t;

   typedef int pid_t;
   typedef unsigned int speed_t;

   #define UINT16_MAX 0xFFFF

   #include <varargs.h>
#else
#include <stdint.h>
#endif

typedef float  float32_t;
typedef double float64_t;

#endif  /* DLT_TYPES_H */
