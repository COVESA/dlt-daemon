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

install(FILES dlt.h dlt_user.h dlt_user_macros.h dlt_client.h dlt_protocol.h dlt_common.h dlt_types.h dlt_version.h dlt_shm.h dlt_offline_trace.h dlt_filetransfer.h dlt_common_api.h
        DESTINATION include/dlt
        COMPONENT devel)


if(${WITH_DLT_CXX11_EXT})
  install(FILES dlt_cpp_extension.hpp
          DESTINATION include/dlt
          COMPONENT devel)
endif()

if(${WITH_DLT_CXX_WRAPPER})
  install(FILES CDltWrapper.h
          DESTINATION include/dlt
          COMPONENT devel)
endif()

