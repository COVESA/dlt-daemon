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
#include <stdlib.h>

#include "dlt-system.h"

#include <systemd/sd-journal.h>
#include <systemd/sd-id128.h>

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
	char match[9+32+1] = "_BOOT_ID=";
	sd_id128_t boot_id;
	
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-journal, in thread."));

	DltSystemConfiguration *conf = (DltSystemConfiguration *) v_conf;
	DLT_REGISTER_CONTEXT(journalContext, conf->Journal.ContextId, "Journal Adapter");

	r = sd_journal_open(&j,  SD_JOURNAL_LOCAL_ONLY/*SD_JOURNAL_LOCAL_ONLY|SD_JOURNAL_RUNTIME_ONLY*/);
	if (r < 0) {
			DLT_LOG(dltsystem, DLT_LOG_ERROR,
					DLT_STRING("dlt-system-journal, cannot open journal:"),DLT_STRING(strerror(-r)));
			return;
	}
			
	if(conf->Journal.CurrentBoot)
	{
		/* show only current boot entries */
		r = sd_id128_get_boot(&boot_id);
		if(r<0)
		{
				DLT_LOG(dltsystem, DLT_LOG_ERROR,
						DLT_STRING("dlt-system-journal failed to get boot id:"),DLT_STRING(strerror(-r)));
				sd_journal_close(j);
				return;
			
		}
		sd_id128_to_string(boot_id, match + 9);
		r = sd_journal_add_match(j,match,strlen(match));
		if(r<0)
		{
				DLT_LOG(dltsystem, DLT_LOG_ERROR,
						DLT_STRING("dlt-system-journal failed to get match:"),DLT_STRING(strerror(-r)));
				sd_journal_close(j);
				return;
			
		}
	}	

	if(conf->Journal.Follow)
	{
		/* show only last 10 entries and follow */
        r = sd_journal_seek_tail(j);
		if(r<0)
		{
				DLT_LOG(dltsystem, DLT_LOG_ERROR,
						DLT_STRING("dlt-system-journal failed to seek to tail:"),DLT_STRING(strerror(-r)));
				sd_journal_close(j);
				return;
			
		}
        r = sd_journal_previous_skip(j, 10);
		if(r<0)
		{
				DLT_LOG(dltsystem, DLT_LOG_ERROR,
						DLT_STRING("dlt-system-journal failed to seek back 10 entries:"),DLT_STRING(strerror(-r)));
				sd_journal_close(j);
				return;
			
		}
	
	}
	
	while(!threads.shutdown)
	{			
		const char *d2="",*d3="",*d4="",*d5="";
		size_t l;
		uint64_t time_usecs = 0;
		int ret;
		struct tm * timeinfo;
		char buffer[256],buffer2[256];
		int loglevel,systemd_loglevel;

		// Jun 28 13:34:05 alexvbox systemd[1]: Started GENIVI DLT system.
		ret = sd_journal_next(j);
		if(ret>0)
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
			
			time_usecs /=1000000;
			timeinfo = localtime ((const time_t*)(&(time_usecs)));
            strftime (buffer,sizeof(buffer),"%Y/%m/%d %H:%M:%S",timeinfo);

			sprintf(buffer2,"%s[%s]:",d2,d3);

			loglevel = DLT_LOG_INFO;
			if(conf->Journal.MapLogLevels)
			{
				/* Map log levels from journal to DLT */
				systemd_loglevel = atoi(d4);
				switch(systemd_loglevel)
				{
					case 0: /* Emergency */
					case 1: /* Alert */
					case 2: /* Critical */
						loglevel = DLT_LOG_FATAL;
						break;
					case 3: /* Error */
						loglevel = DLT_LOG_ERROR;
						break;
					case 4: /* Warning */
					case 5: /* Notice */
						loglevel = DLT_LOG_WARN;
						break;
					case 6: /* Informational */
						loglevel = DLT_LOG_INFO;
						break;
					case 7: /* Debug */
						loglevel = DLT_LOG_DEBUG;
						break;
					default:
						loglevel = DLT_LOG_INFO;
						break;
				}
			}
			
			DLT_LOG(journalContext, loglevel,
						DLT_STRING(buffer),DLT_STRING(buffer2),DLT_STRING(d4),DLT_STRING(d5));

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
