/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de> BMW 2012
 *
 * \file dlt-system.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

#include "dlt-system.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
#include "sd-daemon.h"
#endif

DLT_DECLARE_CONTEXT(dltsystem)

int main(int argc, char* argv[])
{
	DltSystemCliOptions options;
	DltSystemConfiguration config;

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
	int ret;
#endif

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
	DLT_REGISTER_APP(config.ApplicationId, "DLT System Manager");
	DLT_REGISTER_CONTEXT(dltsystem,"MGR", "Context of main dlt system manager");

#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE) || defined(DLT_SYSTEMD_ENABLE)
    ret = sd_booted();

    if(ret == 0){
    	DLT_LOG(dltsystem, DLT_LOG_INFO, DLT_STRING("system not booted with systemd!\n"));
    }
    else if(ret < 0)
    {
    	DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("sd_booted failed!\n"));
    	return -1;
    }
    else
    {
    	DLT_LOG(dltsystem, DLT_LOG_INFO, DLT_STRING("system booted with systemd\n"));
    }
#endif

	DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("Configuration loaded."));

	if(options.Daemonize > 0)
	{
		if(daemonize() < 0)
		{
			DLT_LOG(dltsystem, DLT_LOG_FATAL, DLT_STRING("Daemonization failed!"));
			return -1;
		}
		DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("dlt-system daemonized."));
	}

	DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("Setting signal handlers for abnormal exit"));
	signal(SIGTERM, dlt_system_signal_handler);
	signal(SIGHUP,  dlt_system_signal_handler);
	signal(SIGQUIT, dlt_system_signal_handler);
	signal(SIGINT,  dlt_system_signal_handler);

	DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("Launching threads."));


	start_threads(&config);
	join_threads();
	return 0;
}


