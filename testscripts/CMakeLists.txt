#######
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
#######

if(WITH_TESTSCRIPTS)
    configure_file(${PROJECT_SOURCE_DIR}/testscripts/Meego/dlt-daemon.cmake ${PROJECT_BINARY_DIR}/testscripts/Meego/dlt-daemon @ONLY)
    configure_file(${PROJECT_SOURCE_DIR}/testscripts/Ubuntu/dlt-daemon.cmake ${PROJECT_BINARY_DIR}/testscripts/Ubuntu/dlt-daemon @ONLY)
endif(WITH_TESTSCRIPTS)
