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

set(dlt_logstorage_ctrl_SRCS dlt-logstorage-ctrl.c dlt-logstorage-common.c dlt-logstorage-list.c)

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
 set(dlt_logstorage_ctrl_SRCS ${dlt_logstorage_ctrl_SRCS} dlt-logstorage-udev.c)
endif(WITH_DLT_LOGSTORAGE_CTRL_UDEV)

if(WITH_SYSTEMD)
 set(dlt_logstorage_ctrl_SRCS ${dlt_logstorage_ctrl_SRCS} ${PROJECT_SOURCE_DIR}/systemd/3rdparty/sd-daemon.c)
endif(WITH_SYSTEMD)

add_executable(dlt-logstorage-ctrl ${dlt_logstorage_ctrl_SRCS} ${dlt_control_common_SRCS} ${dlt_most_SRCS})

set(DLT_LOGSTORAGE_LIBRARIES dlt-logstorage-ctrl dlt ${EXPAT_LIBRARIES})

if(WITH_DLT_LOGSTORAGE_CTRL_UDEV)
 set(DLT_LOGSTORAGE_LIBRARIES ${DLT_LOGSTORAGE_LIBRARIES} udev)
endif(WITH_DLT_LOGSTORAGE_CTRL_UDEV)

target_link_libraries(${DLT_LOGSTORAGE_LIBRARIES})

set_target_properties(dlt-logstorage-ctrl PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-logstorage-ctrl
        RUNTIME DESTINATION bin
        COMPONENT base)
