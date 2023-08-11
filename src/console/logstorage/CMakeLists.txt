#######
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2015, ADIT GmbH
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
add_definitions(-Werror)

set(dlt-logstorage-ctrl_SRCS
    dlt-logstorage-ctrl.c
    dlt-logstorage-common.c
    dlt-logstorage-list.c)

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
    set(dlt-logstorage-ctrl_SRCS
        ${dlt-logstorage-ctrl_SRCS}
        dlt-logstorage-udev.c)
    set(LOGSTORAGE_LIBRARY udev)
endif()

if(WITH_SYSTEMD)
    set(dlt-logstorage-ctrl_SRCS
        ${dlt-logstorage-ctrl_SRCS}
        ${PROJECT_SOURCE_DIR}/systemd/3rdparty/sd-daemon.c)
endif()

add_executable(dlt-logstorage-ctrl
               ${dlt-logstorage-ctrl_SRCS}
               ${dlt_control_common_SRCS})

target_link_libraries(dlt-logstorage-ctrl dlt ${LOGSTORAGE_LIBRARY} dlt_control_common_lib)
set_target_properties(dlt-logstorage-ctrl PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-logstorage-ctrl
        RUNTIME DESTINATION bin
        COMPONENT base)
