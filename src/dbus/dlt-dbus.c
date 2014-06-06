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

DLT_DECLARE_CONTEXT(dbusContext);

static char dbus_message_buffer[DBUS_MAXIMUM_MESSAGE_LENGTH];

static const char*
type_to_name (int message_type)
{
  switch (message_type)
    {
    case DBUS_MESSAGE_TYPE_SIGNAL:
      return "signal";
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
      return "method call";
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
      return "method return";
    case DBUS_MESSAGE_TYPE_ERROR:
      return "error";
    default:
      return "(unknown message type)";
    }
}

static DBusHandlerResult
filter_func (DBusConnection *con,
             DBusMessage *message,
             void *data)
{
  DBusMessageIter iter;
  const char *sender;
  const char *destination;
  char **buf;
  int message_type;
  char *log_message;
  int log_type;
  int len_p;

  buf = (char**)&dbus_message_buffer;
  if (!dbus_message_marshal(message,
                            buf,
                            &len_p))
  {
    fprintf (stderr, "Failed to serialize DBus message!\n");
    return DBUS_HANDLER_RESULT_HANDLED;
  }
  //DLT_LOG (dbusContext, DLT_LOG_INFO, DLT_STRING("dbus message"), DLT_RAW ((void *)*buf, len_p));
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
  /* DLT initialisation */
  DLT_REGISTER_APP ("IPC0", "DBus Logging");
  DLT_REGISTER_CONTEXT(dbusContext, "ALL", "DBus Context for Logging");

  DBusConnection *connection;
  DBusError error;
  DBusBusType type = DBUS_BUS_SESSION;
  //DBusBusType type = DBUS_BUS_SYSTEM;

  dbus_error_init (&error);

  connection = dbus_bus_get (type, &error);
  if (NULL == connection)
  {
    fprintf (stderr, "Failed to open connection to %s: %s\n",
             DBUS_BUS_SYSTEM,
             error.message);
             dbus_error_free (&error);
             exit (1);
  }

  dbus_bus_add_match (connection,
                      EAVESDROPPING_RULE "type='signal'",
                      &error);
  if (dbus_error_is_set (&error))
    goto fail;
  dbus_bus_add_match (connection,
                      EAVESDROPPING_RULE "type='method_call'",
                      &error);
  if (dbus_error_is_set (&error))
    goto fail;
  dbus_bus_add_match (connection,
                      EAVESDROPPING_RULE "type='method_return'",
                      &error);
  if (dbus_error_is_set (&error))
    goto fail;
  dbus_bus_add_match (connection,
                      EAVESDROPPING_RULE "type='error'",
                      &error);
  if (dbus_error_is_set (&error))
    goto fail;

  if (!dbus_connection_add_filter (connection, filter_func, NULL, NULL)) {
    fprintf (stderr, "Couldn't add filter!\n");
    exit (1);
  }

  while (dbus_connection_read_write_dispatch(connection, -1))
    ;

  DLT_UNREGISTER_CONTEXT (dbusContext);
  DLT_UNREGISTER_APP ();
  exit(1);

fail:

  /* fail */
  fprintf (stderr, "Error: %s\n", error.message);
  DLT_UNREGISTER_CONTEXT (dbusContext);
  DLT_UNREGISTER_APP ();
  exit(1);

}
