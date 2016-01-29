#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2015, ADIT GmbH
#
# This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
# @licence end@
#######
add_definitions( -Werror )

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV AND WITH_DLT_LOGSTORAGE_CTRL_PROP)
 set(dlt_logstorage_ctrl_SRCS dlt-logstorage-ctrl.c dlt-logstorage-common.c dlt-logstorage-udev.c dlt-logstorage-list.c)
elseif(WITH_DLT_LOGSTORAGE_CTRL_PROP)
 set(dlt_logstorage_ctrl_SRCS dlt-logstorage-ctrl.c dlt-logstorage-common.c dlt-logstorage-list.c)
elseif(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
 set(dlt_logstorage_ctrl_SRCS dlt-logstorage-ctrl.c dlt-logstorage-common.c dlt-logstorage-udev.c dlt-logstorage-list.c)
else(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
 set(dlt_logstorage_ctrl_SRCS dlt-logstorage-ctrl.c dlt-logstorage-common.c dlt-logstorage-list.c)
endif(WITH_DLT_LOGSTORAGE_CTRL_UDEV AND WITH_DLT_LOGSTORAGE_CTRL_PROP)

add_executable(dlt-logstorage-ctrl ${dlt_logstorage_ctrl_SRCS} ${dlt_control_common_SRCS} ${dlt_most_SRCS} ${CMAKE_SOURCE_DIR}/systemd/3rdparty/sd-daemon.c )

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
 target_link_libraries(dlt-logstorage-ctrl dlt udev ${EXPAT_LIBRARIES})
else(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
 target_link_libraries(dlt-logstorage-ctrl dlt ${EXPAT_LIBRARIES})
endif(WITH_DLT_LOGSTORAGE_CTRL_UDEV)

set_target_properties(dlt-logstorage-ctrl PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-logstorage-ctrl
        RUNTIME DESTINATION bin
        COMPONENT base)
