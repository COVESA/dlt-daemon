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

set (dlt_kpi_SRCS dlt-kpi.c dlt-kpi-options.c dlt-kpi-process.c dlt-kpi-process-list.c dlt-kpi-common.c dlt-kpi-interrupt.c)
add_executable (dlt-kpi ${dlt_kpi_SRCS})
target_link_libraries (dlt-kpi dlt)
set_target_properties(dlt-kpi PROPERTIES LINKER_LANGUAGE C)

install(TARGETS dlt-kpi
	RUNTIME DESTINATION bin
	COMPONENT base)

INSTALL(FILES dlt-kpi.conf
	DESTINATION ${CONFIGURATION_FILES_DIR}
	COMPONENT base)
