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

#define DLT_SYSTEM_JOURNAL_BUFFER_SIZE 256
#define DLT_SYSTEM_JOURNAL_BUFFER_SIZE_BIG 2048

#define DLT_SYSTEM_JOURNAL_ASCII_FIRST_VISIBLE_CHARACTER 31
#define DLT_SYSTEM_JOURNAL_BOOT_ID_MAX_LENGTH 9+32+1


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

int dlt_system_journal_get(sd_journal* j,char *target,const char *field,size_t max_size)
{
	char *data;
	size_t length;
	int error_code;
	size_t field_size;

	// pre check parameters
	if(max_size<1 || target == 0 || field == 0 || j == 0)
		return -1;

	// intialise empty target
	target[0]=0;

	// get data from journal
	error_code = sd_journal_get_data(j, field,(const void **) &data, &length);

	// check if an error
	if(error_code)
		return error_code;

	// calculate field size
	field_size = strlen(field)+1;

	//check length
	if(length<field_size)
	   return -1;

    // copy string
	if(max_size<=(length-field_size))
	{
		// truncate
		strncpy(target,data+field_size,max_size-1);
		target[max_size]=0;
	}
	else
	{
		// full copy
		strncpy(target,data+field_size,length-field_size);
		target[length-field_size]=0;

	}

	// debug messages
	//printf("%s = %s\n",field,target);

	// Success
	return 0;
}

void journal_thread(void *v_conf)
{
	int r;
	sd_journal *j;
	char match[DLT_SYSTEM_JOURNAL_BOOT_ID_MAX_LENGTH] = "_BOOT_ID=";
	sd_id128_t boot_id;
	uint64_t time_usecs = 0;
	struct tm * timeinfo;
	char buffer_time[DLT_SYSTEM_JOURNAL_BUFFER_SIZE],
	     buffer_process[DLT_SYSTEM_JOURNAL_BUFFER_SIZE],
	     buffer_priority[DLT_SYSTEM_JOURNAL_BUFFER_SIZE],
	     buffer_pid[DLT_SYSTEM_JOURNAL_BUFFER_SIZE],
	     buffer_comm[DLT_SYSTEM_JOURNAL_BUFFER_SIZE],
	     buffer_message[DLT_SYSTEM_JOURNAL_BUFFER_SIZE_BIG],
    	 buffer_transport[DLT_SYSTEM_JOURNAL_BUFFER_SIZE];
	int loglevel,systemd_loglevel;
	char* systemd_log_levels[] = { "Emergency","Alert","Critical","Error","Warning","Notice","Informational","Debug" };
	
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-journal, in thread."));

	DltSystemConfiguration *conf = (DltSystemConfiguration *) v_conf;
	DLT_REGISTER_CONTEXT(journalContext, conf->Journal.ContextId, "Journal Adapter");

	r = sd_journal_open(&j,  SD_JOURNAL_LOCAL_ONLY/*SD_JOURNAL_LOCAL_ONLY|SD_JOURNAL_RUNTIME_ONLY*/);
			printf("journal open return %d\n", r);	
	if (r < 0) {
			DLT_LOG(dltsystem, DLT_LOG_ERROR,
					DLT_STRING("dlt-system-journal, cannot open journal:"),DLT_STRING(strerror(-r)));
			printf("journal open failed: %s\n", strerror(-r));
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

		r = sd_journal_next(j);
		if(r<0)
		{
				DLT_LOG(dltsystem, DLT_LOG_ERROR,
						DLT_STRING("dlt-system-journal failed to get next entry:"),DLT_STRING(strerror(-r)));
				sd_journal_close(j);
				return;
			
		}
		else if(r>0)
		{
			/* get all data from current journal entry */
			r = sd_journal_get_realtime_usec(j, &time_usecs);
			if(r<0)
			{
					DLT_LOG(dltsystem, DLT_LOG_ERROR,
							DLT_STRING("dlt-system-journal failed to call sd_journal_get_realtime_usec(): "),DLT_STRING(strerror(-r)));
					sd_journal_close(j);
					return;
				
			}

			/* get data from journal entry, empty string if invalid fields */
			dlt_system_journal_get(j,buffer_comm,"_COMM",sizeof(buffer_comm));
			dlt_system_journal_get(j,buffer_pid,"_PID",sizeof(buffer_pid));
			dlt_system_journal_get(j,buffer_priority,"PRIORITY",sizeof(buffer_priority));
			dlt_system_journal_get(j,buffer_message,"MESSAGE",sizeof(buffer_message));
			dlt_system_journal_get(j,buffer_transport,"_TRANSPORT",sizeof(buffer_transport));

			/* prepare time string */
			time_usecs /=1000000;
			timeinfo = localtime ((const time_t*)(&(time_usecs)));
            strftime (buffer_time,sizeof(buffer_time),"%Y/%m/%d %H:%M:%S",timeinfo);

			/* prepare process string */
			if(strcmp(buffer_transport,"kernel")==0)
				snprintf(buffer_process,DLT_SYSTEM_JOURNAL_BUFFER_SIZE,"kernel:");
			else
				snprintf(buffer_process,DLT_SYSTEM_JOURNAL_BUFFER_SIZE,"%s[%s]:",buffer_comm,buffer_pid);

			/* map log level on demand */
			loglevel = DLT_LOG_INFO;
			systemd_loglevel = atoi(buffer_priority);
			if(conf->Journal.MapLogLevels)
			{
				/* Map log levels from journal to DLT */
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
						loglevel = DLT_LOG_WARN;
						break;
					case 5: /* Notice */
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
			if(systemd_loglevel>=0 && systemd_loglevel<=7)
				snprintf(buffer_priority,DLT_SYSTEM_JOURNAL_BUFFER_SIZE,"%s:",systemd_log_levels[systemd_loglevel]);
			else
				snprintf(buffer_priority,DLT_SYSTEM_JOURNAL_BUFFER_SIZE,"prio_unknown:");
			
			/* write log entry */
			DLT_LOG(journalContext, loglevel,
						DLT_STRING(buffer_time),DLT_STRING(buffer_process),DLT_STRING(buffer_priority),DLT_STRING(buffer_message));

		}
		else
		{
			r = sd_journal_wait(j,1000000);			
			if(r<0)
			{
					DLT_LOG(dltsystem, DLT_LOG_ERROR,
							DLT_STRING("dlt-system-journal failed to call sd_journal_get_realtime_usec(): "),DLT_STRING(strerror(-r)));
					sd_journal_close(j);
					return;
				
			}
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
