#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2018 Advanced Driver Information Technology.
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
# @licence end@
#######

if(NOT WITH_DLT_CXX11_EXT)
    message(FATAL_ERROR "DLT_QNX_SYSTEM needs the C++ features of dlt, please enable the WITH_DLT_CXX11_EXT option.")
endif()

set(dlt-qnx-system_SRCS dlt-qnx-system.c dlt-qnx-slogger2-adapter.cpp)
add_executable(dlt-qnx-system ${dlt-qnx-system_SRCS})

target_link_libraries(dlt-qnx-system dlt slog2parse json)

install(TARGETS dlt-qnx-system
    RUNTIME DESTINATION bin
    COMPONENT base)

install(FILES dlt-qnx-system.conf dlt-slog2ctxt.json
        DESTINATION ${CONFIGURATION_FILES_DIR}
        COMPONENT base)
