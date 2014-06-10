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

#ifndef DLT_DBUS_H_
#define DLT_DBUS_H_

// DLT related includes.
#include "dlt.h"
#include "dlt_common.h"

#define DEFAULT_CONF_FILE "/etc/dlt-dbus.conf"

#define DLT_DBUS_FILTER_MAX 32

// Macros
#define UNUSED(x) (void)(x)
#define MALLOC_ASSERT(x) if(x == NULL) {\
	fprintf(stderr, "Out of memory\n");\
	abort();}

#define MAX_LINE 1024

// Command line options
typedef struct {
	char 	*ConfigurationFileName;
	char 	*ApplicationId;
	char 	*BusType;
	int 	Daemonize;
} DltDBusCliOptions;

// Configuration dbus options
typedef struct {
	char 	*ContextId;
	char 	*BusType;
	int  FilterCount;
	char *FilterMatch[DLT_DBUS_FILTER_MAX];
} DBusOptions;

typedef struct {
	char *ApplicationId;
	DBusOptions			DBus;

} DltDBusConfiguration;

extern void init_cli_options(DltDBusCliOptions *options);
extern int read_command_line(DltDBusCliOptions *options, int argc, char *argv[]);
extern int read_configuration_file(DltDBusConfiguration *config, char *file_name);

#endif /* DLT_DBUS_H_ */
