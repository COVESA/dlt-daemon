#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
#
# Copyright (C) 2011-2012, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# @licence end@
########

if(WITH_TESTSCRIPTS)
    configure_file(${CMAKE_SOURCE_DIR}/testscripts/Meego/dlt-daemon.cmake ${PROJECT_BINARY_DIR}/testscripts/Meego/dlt-daemon @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/testscripts/Ubuntu/dlt-daemon.cmake ${PROJECT_BINARY_DIR}/testscripts/Ubuntu/dlt-daemon @ONLY)
endif(WITH_TESTSCRIPTS)
