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

install(FILES dlt.h dlt_user.h dlt_user_macros.h dlt_client.h dlt_protocol.h dlt_common.h dlt_types.h dlt_version.h dlt_shm.h dlt_offline_trace.h dlt_filetransfer.h dlt_common_api.h
        DESTINATION include/dlt
	COMPONENT devel)
