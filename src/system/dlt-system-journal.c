/**
 * @licence app begin@
 * Copyright (C) 2013  BMW AG
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
 * \author Alexander Wenzel <Alexander.AW.Wenzel@bmw.de> BMW 2013
 *
 * \file dlt-system-journal.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-journal.c                                          **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
#if defined(DLT_SYSTEMD_JOURNAL_ENABLE)

#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>

#include "dlt-system.h"

#include <systemd/sd-journal.h>

extern DltSystemThreads threads;

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(journalContext)

int journal_checkUserBufferForFreeSpace()
{
	int total_size, used_size;

	dlt_user_check_buffer(&total_size, &used_size);

	if((total_size - used_size) < (total_size/2))
	{
		return -1;
	}
	return 1;
}

void journal_thread(void *v_conf)
{
	int r,r2,r3,r4,r5;
	sd_journal *j;

	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-journal, in thread."));

	DltSystemConfiguration *conf = (DltSystemConfiguration *) v_conf;
	DLT_REGISTER_CONTEXT(journalContext, conf->Journal.ContextId, "Journal Adapter");

	r = sd_journal_open(&j,  SD_JOURNAL_LOCAL_ONLY|SD_JOURNAL_RUNTIME_ONLY);
	if (r < 0) {
			DLT_LOG(dltsystem, DLT_LOG_ERROR,
					DLT_STRING("dlt-system-journal, cannot open journal."));
			return;
	}
	
	// Uncomment if only new jornal entries should be shown
	//sd_journal_seek_tail(j);
	
	/* possible mapping of log levels */
	/* journal log levels
	      0       Emergency		DLT_LOG_CRITICAL
          1       Alert			DLT_LOG_CRITICAL
          2       Critical		DLT_LOG_CRITICAL
          3       Error			DLT_LOG_ERROR
          4       Warning		DLT_LOG_WARNING
          5       Notice		DLT_LOG_WARNING
          6       Informational DLT_LOG_INFO
          7       Debug 		DLT_LOG_DEBUG
	*/
	
	while(!threads.shutdown)
	{			
		const char *d2="",*d3="",*d4="",*d5="";
		size_t l;
		uint64_t time_usecs = 0;

		// Jun 28 13:34:05 alexvbox systemd[1]: Started GENIVI DLT system.
		if(sd_journal_next(j)>0)
		{
			sd_journal_get_realtime_usec(j, &time_usecs);
			r2 = sd_journal_get_data(j, "_COMM",(const void **) &d2, &l);
			r3 = sd_journal_get_data(j, "_PID",(const void **) &d3, &l);
			r4 = sd_journal_get_data(j, "PRIORITY",(const void **) &d4, &l);
			r5 = sd_journal_get_data(j, "MESSAGE",(const void **) &d5, &l);
			if(r2>=0 && strlen(d2)>6) 	d2 +=6;
			if(r3>=0 && strlen(d3)>5) 	d3 +=5;
			if(r4>=0 && strlen(d4)>9) 	d4 +=9;
			if(r5>=0 && strlen(d5)>8) 	d5 +=8;
			
			if(r5>=0)
			{
				DLT_LOG(journalContext, DLT_LOG_INFO,
						DLT_UINT64(time_usecs),DLT_STRING(d2),DLT_STRING(d3),DLT_STRING(d4),DLT_STRING(d5));
			}

		}
		else
		{
			sd_journal_wait(j,1000000);			
		}

		if(journal_checkUserBufferForFreeSpace()==-1)
		{
			// buffer is nearly full
			// wait 500ms for writing next entry
			struct timespec t;
			t.tv_sec = 0;
			t.tv_nsec = 1000000ul*500;
			nanosleep(&t, NULL);
		}


	}

    sd_journal_close(j);

	DLT_UNREGISTER_CONTEXT(journalContext);

}

void start_systemd_journal(DltSystemConfiguration *conf)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-journal, start journal"));
	static pthread_attr_t t_attr;
	static pthread_t pt;
	pthread_create(&pt, &t_attr, (void *)journal_thread, conf);
	threads.threads[threads.count++] = pt;
}

#endif /* DLT_SYSTEMD_JOURNAL_ENABLE */
