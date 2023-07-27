/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2015  Intel Corporation
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
 * \author Stefan Vacek <stefan.vacek@intel.com> Intel Corporation
 *
 * \copyright Copyright © 2015 Intel Corporation. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-cpp-extension.cpp
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
    if (dlt_register_app("TCPP", "Test cpp extension") < 0) {
        printf("Failed to register application\n");
        return -1;
    }

    DltContext ctx;

    if (dlt_register_context_ll_ts(&ctx, "TCPP", "Test cpp extension", DLT_LOG_INFO, DLT_TRACE_STATUS_OFF) < 0) {
        printf("Failed to register context\n");
        return -1;
    }

    dlt_enable_local_print();
    dlt_verbose_mode();

    DLT_LOG(ctx, DLT_LOG_WARN, DLT_STRING("a message")); /* the classic way to go */

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

    dlt_unregister_context(&ctx);
    dlt_unregister_app();

    return 0;
}
