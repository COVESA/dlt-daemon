/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.wenzel@bmw.de>
 *
 * \file dlt-dbus.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "dlt-dbus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <sys/time.h>
#include <dlt.h>

#include <time.h>

#ifdef USE_EAVESDROP
    #define EAVESDROPPING_RULE "eavesdrop=true,"
#else
    #define EAVESDROPPING_RULE ""
#endif

DLT_DECLARE_CONTEXT(dbusLog);
DLT_DECLARE_CONTEXT(dbusContext);

static char dbus_message_buffer[DBUS_MAXIMUM_MESSAGE_LENGTH];

static DBusHandlerResult
filter_func (DBusConnection *con,
             DBusMessage *message,
             void *data)
{
  char **buf;
  int len_p;
  UNUSED(con);
  UNUSED(data);

  buf = (char**)&dbus_message_buffer;
  if (!dbus_message_marshal(message,
                            buf,
                            &len_p))
  {
    fprintf (stderr, "Failed to serialize DBus message!\n");
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  DLT_TRACE_NETWORK_SEGMENTED(dbusContext,DLT_NW_TRACE_IPC,0,0,len_p,(void *)*buf);

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
  {
    DLT_UNREGISTER_CONTEXT (dbusContext);
    DLT_UNREGISTER_APP ();
    exit (0);
  }


  /* Conceptually we want this to be
   * DBUS_HANDLER_RESULT_NOT_YET_HANDLED, but this raises
   * some problems.  See bug 1719.
   */
  return DBUS_HANDLER_RESULT_HANDLED;
}

int main (int argc, char *argv[])
{
	DltDBusCliOptions options;
	DltDBusConfiguration config;

	DBusConnection *connection;
	DBusError error;
	DBusBusType type;

	int num;

	if(read_command_line(&options, argc, argv) < 0)
	{
		fprintf(stderr, "Failed to read command line!\n");
		return -1;
	}

	if(read_configuration_file(&config, options.ConfigurationFileName) < 0)
	{
		fprintf(stderr, "Failed to read configuration file!\n");
		return -1;
	}

	// register application
	if(options.ApplicationId)
		DLT_REGISTER_APP (options.ApplicationId, "DBus Logging");
	else
		DLT_REGISTER_APP (config.ApplicationId, "DBus Logging");

	// register context
	DLT_REGISTER_CONTEXT(dbusContext, config.DBus.ContextId, "DBus Context for Logging");
	DLT_REGISTER_CONTEXT(dbusLog, "Log", "DBus Context for Logging Generic information");

	// initialise error handler
	dbus_error_init (&error);

	// set DBus bus type
	if(options.BusType)
		type = (DBusBusType) atoi(options.BusType);
	else
		type = (DBusBusType) atoi(config.DBus.BusType);

	// get connection
	connection = dbus_bus_get (type, &error);

	if(type==0)
		DLT_LOG(dbusLog,DLT_LOG_INFO,DLT_STRING("BusType"),DLT_STRING("Session Bus"));
	else if(type==1)
		DLT_LOG(dbusLog,DLT_LOG_INFO,DLT_STRING("BusType"),DLT_STRING("System Bus"));
	else
		DLT_LOG(dbusLog,DLT_LOG_INFO,DLT_STRING("BusType"),DLT_INT(type));

	if (NULL == connection)
	{
	fprintf (stderr, "Failed to open connection to %d: %s\n",
			 DBUS_BUS_SYSTEM,
			 error.message);
			 dbus_error_free (&error);
			 exit (1);
	}

	for(num=0;num<config.DBus.FilterCount;num++)
	{
		dbus_bus_add_match (connection,
						  config.DBus.FilterMatch[num],
						  &error);
		printf("Added FilterMatch: %s\n",config.DBus.FilterMatch[num]);
		DLT_LOG(dbusLog,DLT_LOG_INFO,DLT_STRING("FilterMatch"),DLT_UINT(num+1),DLT_STRING(config.DBus.FilterMatch[num]));
		if (dbus_error_is_set (&error))
		goto fail;
	}

	if (!dbus_connection_add_filter (connection, filter_func, NULL, NULL)) {
	fprintf (stderr, "Couldn't add filter!\n");
	exit (1);
	}

	while (dbus_connection_read_write_dispatch(connection, -1))
	;

	DLT_UNREGISTER_CONTEXT (dbusContext);
	DLT_UNREGISTER_CONTEXT (dbusLog);
	DLT_UNREGISTER_APP ();
	exit(1);

fail:

	/* fail */
	fprintf (stderr, "Error: %s\n", error.message);
	DLT_UNREGISTER_CONTEXT (dbusContext);
	DLT_UNREGISTER_CONTEXT (dbusLog);
	DLT_UNREGISTER_APP ();
	exit(1);

}
