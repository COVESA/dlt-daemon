/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, V2 - Volvo Group
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
 * \author Shivam Goel <shivam.goel@volvo.com>
 *
 * \copyright Copyright © 2011-2015 V2 - Volvo Group. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-cpp-extension-v2.cpp
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-cpp-extension-v2.cpp                                 **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Shivam Goel <shivam.goel@volvo.com>                           **
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
**  sg          Shivam Goel                V2 - Volvo Group                   **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision:  $
 * $LastChangedDate: 2025-11-12 15:12:06 +0200 (We, 12. Nov 2025) $
 * $LastChangedBy$
 * Initials    Date         Comment
 * sg          12.11.2025   initial
 */

#include "dlt_cpp_extension.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dlt_user_macros.h"

struct MyStruct
{
    int64_t uuid;
    int32_t interfaceId;
    int32_t registrationState;
};

template<>
inline int logToDlt(DltContextData &log, MyStruct const &value)
{
    int result = 0;

    result += dlt_user_log_write_string(&log, "(");
    result += logToDlt(log, value.uuid);
    result += dlt_user_log_write_string(&log, ",");
    result += logToDlt(log, value.interfaceId);
    result += dlt_user_log_write_string(&log, ",");
    result += logToDlt(log, value.registrationState);
    result += dlt_user_log_write_string(&log, ")");

    if (result != 0)
        result = -1;

    return result;
}

/**
 * Sample code to show usage of the cpp-extension
 * mainly the variadic templates
 */
int main()
{
    if (dlt_register_app_v2("TCPP", "Test cpp extension") < 0) {
        printf("Failed to register application\n");
        return -1;
    }

    DltContext ctx;

    if (dlt_register_context_ll_ts_v2(&ctx, "TCPP", "Test cpp extension", DLT_LOG_INFO, DLT_TRACE_STATUS_OFF) < 0) {
        printf("Failed to register context\n");
        return -1;
    }

    dlt_enable_local_print();
    dlt_verbose_mode();

    DLT_LOG_V2(ctx, DLT_LOG_WARN, DLT_STRING("a message")); /* the classic way to go */

    int an_int = 42;
    float a_float = 22.7;
    DLT_LOG_FCN_CXX(ctx, DLT_LOG_WARN, "Testing DLT_LOG_CXX_FCN", an_int, a_float);
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, 1.0, 65);

    /* Example for logging user-defined types */
    MyStruct myData = { 1u, 2u, 3u };
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, "MyStruct myData", myData);

    char *non_const_string = (char *)malloc(17);
    memcpy(non_const_string, "non_const_string", 16);
    non_const_string[16] = 0;
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, "char *", non_const_string);

    std::string aString = "std::string";
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, "std::string", aString);

    std::vector<int> intVector;
    intVector.push_back(0);
    intVector.push_back(1);
    intVector.push_back(2);
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, "vector", intVector);

    std::vector<double> doubleList;
    doubleList.push_back(10.);
    doubleList.push_back(11.);
    doubleList.push_back(12.);
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, "list", doubleList);

    std::map<const char *, int> testMap;
    testMap["apple"] = 100;
    testMap["plum"] = 200;
    testMap["orange"] = 300;
    DLT_LOG_CXX(ctx, DLT_LOG_WARN, "map", testMap);

    dlt_unregister_context_v2(&ctx);
    dlt_unregister_app_v2();

    return 0;
}
