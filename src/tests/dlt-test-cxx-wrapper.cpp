/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2017, GENIVI Alliance
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
 * \author Emneg Zeerd <emneg.zeerd@gmail.com>
 *
 * \copyright Copyright © 2017 GENIVI Alliance . \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-test-user.c
 */

#include "CDltWrapper.h"

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
       (void)argc;
       (void)argv;

       CDltWrapper::instanctiateOnce("CDLT", "testing for CDltWrapper");

       logVerbose("[Verbose]CDltWrapperTest : ", "OK");
       logDebug  ("[Debug  ]CDltWrapperTest : ", "OK");
       logInfo   ("[Info   ]CDltWrapperTest : ", "OK");
       logWarning("[Warning]CDltWrapperTest : ", "OK");
       logError  ("[Error  ]CDltWrapperTest : ", "OK");

       CDltWrapper::instance()->init(DLT_LOG_VERBOSE);
       CDltWrapper::instance()->append("[Verbose]CDltWrapperTest : Verbose temporary enable .");
       CDltWrapper::instance()->send();

       logVerbose("[Verbose]CDltWrapperTest : ", "OK");

       return 0;
}
