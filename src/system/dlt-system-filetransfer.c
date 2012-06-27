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
 * \file dlt-system-filetransfer.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-filetransfer.c                                                  **
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


#include <pthread.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <dirent.h>
#include <zlib.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "dlt-system.h"
#include "dlt.h"
#include "dlt_filetransfer.h"

#define INOTIFY_SZ (sizeof(struct inotify_event))
#define INOTIFY_LEN (INOTIFY_SZ + 256)
#define Z_CHUNK_SZ 1024*128

extern DltSystemThreads threads;
// From dlt_filetransfer
extern unsigned long getFileSerialNumber(const char* file);

DLT_IMPORT_CONTEXT(dltsystem);
DLT_DECLARE_CONTEXT(filetransferContext)

typedef struct {
	int handle;
	int fd[DLT_SYSTEM_LOG_DIRS_MAX];
} s_ft_inotify;

s_ft_inotify ino;

char *unique_name(const char *src)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-filetransfer, creating unique temporary file name."));
	time_t t = time(NULL);
	unsigned long l = getFileSerialNumber(src) ^ t;
	// Length of ULONG_MAX + 1
	char *ret = malloc(11);
	MALLOC_ASSERT(ret);
	snprintf(ret, 11, "%lu", l);
	return ret;
}

char *compress_file(char *src, int level)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-filetransfer, compressing file."));
	char *buf;
	char *dst = malloc(strlen(src)+4);
	MALLOC_ASSERT(dst);
	char dst_mode[8];
	sprintf(dst, "%s.gz", src);
	sprintf(dst_mode, "wb%d", level);

	gzFile dst_file;
	FILE *src_file;

	dst_file = gzopen(dst, dst_mode);
	if(dst_file == Z_NULL)
	{
		free(dst);
		return NULL;
	}

	src_file = fopen(src, "r");

	if(src_file == NULL)
	{
		gzclose(dst_file);
		free(dst);
		return NULL;
	}

	buf = malloc(Z_CHUNK_SZ);
	MALLOC_ASSERT(buf);

	while(!feof(src_file))
	{
		int read = fread(buf, 1, Z_CHUNK_SZ, src_file);
		if(ferror(src_file))
		{
			free(buf);
			free(dst);
			gzclose(dst_file);
			return NULL;
		}
		gzwrite(dst_file, buf, read);
	}

	if(remove(src) < 0)
		DLT_LOG(dltsystem, DLT_LOG_WARN, DLT_STRING("Could not remove file"), DLT_STRING(src));
	free(buf);
	fclose(src_file);
	gzclose(dst_file);
	return dst;
}

int send_one(char *src, FiletransferOptions opts, int which)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-filetransfer, sending a file."));
	sleep(opts.TimeDelay);

	// Prepare all needed file names
	char *fn = basename(src);
	char *rn = unique_name(src);
	char *dst = malloc(strlen(opts.TempDir)+strlen(rn)+2);
	MALLOC_ASSERT(fn);
	MALLOC_ASSERT(rn);
	MALLOC_ASSERT(dst);

	sprintf(dst, "%s/%s", opts.TempDir, rn);
	if(rename(src, dst) < 0)
	{
		DLT_LOG(dltsystem, DLT_LOG_ERROR,
				DLT_STRING("Could not move file"),
				DLT_STRING(src),
				DLT_STRING(dst));
		free(rn);
		free(dst);
		return -1;
	}

	// Compress if needed
	if(opts.Compression[which] > 0)
	{
		dst = compress_file(dst, opts.CompressionLevel[which]);
		char *old_fn = fn;
		fn = malloc(strlen(old_fn)+4);
		MALLOC_ASSERT(fn);
		sprintf(fn, "%s.gz", old_fn);
	}

	if(dlt_user_log_file_header_alias(&filetransferContext, dst, fn) == 0)
	{
		int pkgcount = dlt_user_log_file_packagesCount(&filetransferContext, dst);
		int lastpkg = 0;
		while(lastpkg < pkgcount)
		{
			int total = 2;
			int used = 2;
            dlt_user_check_buffer(&total, &used);
            while((total-used) < (total/2))
			{
				struct timespec t;
				t.tv_sec = 0;
				t.tv_nsec = 1000000ul*opts.TimeoutBetweenLogs;
				nanosleep(&t, NULL);
				dlt_user_check_buffer(&total, &used);
			}
			lastpkg++;
			if(dlt_user_log_file_data(&filetransferContext, dst, lastpkg, opts.TimeoutBetweenLogs) < 0)
				break;
		}
		dlt_user_log_file_end(&filetransferContext, dst, 1);
	}

	if(opts.Compression[which] > 0)
		free(fn);
	free(rn);
	free(dst);
	return 0;
}

int flush_dir(FiletransferOptions opts, int which)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-filetransfer, flush directory of old files."));
	const char *sdir = opts.Directory[which];
	char *fn;
	struct dirent *dp;
	DIR *dir;

	dir = opendir(sdir);
	if(dir != NULL)
	{
		while((dp = readdir(dir)) != NULL)
		{
			if(dp->d_type != DT_REG)
				continue;
			DLT_LOG(dltsystem, DLT_LOG_DEBUG,
					DLT_STRING("dlt-system-filetransfer, old file found in directory."));
			fn = malloc(strlen(sdir)+dp->d_reclen+2);
			MALLOC_ASSERT(fn);
			sprintf(fn, "%s/%s", sdir, dp->d_name);
			if(send_one(fn, opts, which) < 0)
				return -1;
		}
	}
	else
	{
		DLT_LOG(dltsystem, DLT_LOG_ERROR,
				DLT_STRING("Could not open directory"),
				DLT_STRING(sdir));
		return -1;
	}
	closedir(dir);
	return 0;
}

int init_filetransfer_dirs(FiletransferOptions opts)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-filetransfer, initializing inotify on directories."));
	ino.handle = inotify_init();
	int i;

	if(ino.handle < 0)
	{
		DLT_LOG(filetransferContext, DLT_LOG_FATAL,
		DLT_STRING("Failed to initialize inotify in dlt-system file transfer."));
		return -1;
	}
	for(i = 0;i < opts.Count;i++)
	{
		ino.fd[i] = inotify_add_watch(ino.handle, opts.Directory[i],
				IN_CLOSE_WRITE|IN_MOVED_TO);
		if(ino.fd[i] < 0)
		{
			char buf[1024];
			snprintf(buf, 1024, "Failed to add inotify watch to directory %s in dlt-system file transfer.",
					opts.Directory[i]);
			DLT_LOG(filetransferContext, DLT_LOG_FATAL,
			DLT_STRING(buf));
			return -1;
		}
		flush_dir(opts, i);
	}
	return 0;
}

int wait_for_files(FiletransferOptions opts)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("dlt-system-filetransfer, waiting for files."));
	static char buf[INOTIFY_LEN];
	int len = read(ino.handle, buf, INOTIFY_LEN);
	if(len < 0)
	{
		DLT_LOG(filetransferContext, DLT_LOG_ERROR,
		DLT_STRING("Error while waiting for files in dlt-system file transfer."));
		return -1;
	}

	int i = 0;
	while(i<len)
	{
		struct inotify_event *ie = (struct inotify_event *)&buf[i];
		if(ie->len > 0)
		{
			if(ie->mask & IN_CLOSE_WRITE || ie->mask & IN_MOVED_TO)
			{
				int j;
				for(j = 0;j < opts.Count;j++)
				{
					if(ie->wd == ino.fd[j])
					{
						DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("dlt-system-filetransfer, found new file."));
						char *tosend = malloc(strlen(opts.Directory[j])+ie->len+1);
						sprintf(tosend, "%s/%s", opts.Directory[j], ie->name);
						send_one(tosend, opts, j);
						free(tosend);
					}
				}
			}
		}
		i += INOTIFY_SZ + ie->len;
	}
	return 0;
}

void filetransfer_thread(void *v_conf)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("dlt-system-filetransfer, in thread."));
	DltSystemConfiguration *conf = (DltSystemConfiguration *) v_conf;
	DLT_REGISTER_CONTEXT(filetransferContext, conf->Filetransfer.ContextId,
			"File transfer manager.");

	sleep(conf->Filetransfer.TimeStartup);

	if(init_filetransfer_dirs(conf->Filetransfer) < 0)
		return;

	while(!threads.shutdown)
	{
		if(wait_for_files(conf->Filetransfer) < 0)
		{
			DLT_LOG(dltsystem, DLT_LOG_ERROR, DLT_STRING("Error while waiting files. File transfer shutdown."));
			return;
		}
		sleep(conf->Filetransfer.TimeDelay);
	}
}

void start_filetransfer(DltSystemConfiguration *conf)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("dlt-system-filetransfer, start."));
	static pthread_attr_t t_attr;
	static pthread_t pt;
	pthread_create(&pt, &t_attr, (void *)filetransfer_thread, conf);
	threads.threads[threads.count++] = pt;
}
