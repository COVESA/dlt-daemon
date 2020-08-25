#######
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
#######

set(dlt_example_user_SRCS dlt-example-user.c)
add_executable(dlt-example-user ${dlt_example_user_SRCS})
target_link_libraries(dlt-example-user dlt)
set_target_properties(dlt-example-user PROPERTIES LINKER_LANGUAGE C)

set(dlt_example_user_common_api_SRCS dlt-example-user-common-api.c)
add_executable(dlt-example-user-common-api ${dlt_example_user_common_api_SRCS})
target_link_libraries(dlt-example-user-common-api dlt)
set_target_properties(dlt-example-user-common-api PROPERTIES LINKER_LANGUAGE C)

set(dlt_example_user_func_SRCS dlt-example-user-func.c)
add_executable(dlt-example-user-func ${dlt_example_user_func_SRCS})
target_link_libraries(dlt-example-user-func dlt)
set_target_properties(dlt-example-user-func PROPERTIES LINKER_LANGUAGE C)

set(dlt_example_filetransfer_SRCS dlt-example-filetransfer.c)
add_executable(	dlt-example-filetransfer ${dlt_example_filetransfer_SRCS})
target_link_libraries(dlt-example-filetransfer dlt )
set_target_properties(dlt-example-filetransfer PROPERTIES LINKER_LANGUAGE C)

if(WITH_UDP_CONNECTION)
    set(dlt-example-multicast-clientmsg-view_SRCS dlt-example-multicast-clientmsg-view.c)
    add_executable(dlt-example-multicast-clientmsg-view ${dlt-example-multicast-clientmsg-view_SRCS} ${dlt_most_SRCS})
    target_link_libraries(dlt-example-multicast-clientmsg-view dlt )
    set_target_properties(dlt-example-multicast-clientmsg-view PROPERTIES LINKER_LANGUAGE C)

    set(dlt_example_multicast_client_SRCS dlt-example-multicast-client.c)
    add_executable(dlt-example-multicast-client ${dlt_example_multicast_client_SRCS})
    target_link_libraries(dlt-example-multicast-client dlt)
    set_target_properties(dlt-example-multicast-client PROPERTIES LINKER_LANGUAGE C)

    install(TARGETS dlt-example-multicast-clientmsg-view dlt-example-multicast-client
            RUNTIME DESTINATION bin
            COMPONENT base)

endif(WITH_UDP_CONNECTION)

install(TARGETS dlt-example-user dlt-example-user-func dlt-example-user-common-api dlt-example-filetransfer
	RUNTIME DESTINATION bin
	COMPONENT base)
