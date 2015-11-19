#######
# @licence make begin@
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2011-2015, BMW AG
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

set (dlt_procfs_SRCS dlt-procfs.c dlt-procfs-options.c dlt-procfs-process.c dlt-procfs-process-list.c dlt-procfs-common.c dlt-procfs-interrupt.c)
add_executable (dlt-procfs ${dlt_procfs_SRCS})
target_link_libraries (dlt-procfs dlt)
set_target_properties(dlt-procfs PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-procfs
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-procfs.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)
