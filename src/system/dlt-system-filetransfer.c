/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/*!
 * \author
 * Lassi Marttala <lassi.lm.marttala@partner.bmw.de>
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-system-filetransfer.c
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


#include <unistd.h>
#ifdef linux
#   include <sys/inotify.h>
#endif
#include <libgen.h>
#include <dirent.h>
#include <zlib.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <poll.h>

#include "dlt-system.h"
#include "dlt.h"
#include "dlt_filetransfer.h"

#ifdef linux
#   define INOTIFY_SZ (sizeof(struct inotify_event))
#   define INOTIFY_LEN (INOTIFY_SZ + NAME_MAX + 1)
#endif
#define Z_CHUNK_SZ 1024 * 128
#define COMPRESS_EXTENSION ".gz"
#define SUBDIR_COMPRESS ".tocompress"
#define SUBDIR_TOSEND ".tosend"


/* From dlt_filetransfer */
extern uint32_t getFileSerialNumber(const char *file, int *ok);

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(filetransferContext)

#ifdef linux
s_ft_inotify ino;
#endif


char *origin_name(char *src)
{
    if (strlen((char *)basename(src)) > 10) {
        return (char *)(basename(src) + 10);
    }
    else {
        DLT_LOG(dltsystem, DLT_LOG_ERROR,
                DLT_STRING("dlt-system-filetransfer, error in recreating origin name!"));
        return NULL;
    }
}

char *unique_name(char *src)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, creating unique temporary file name."));
    time_t t = time(NULL);
    int ok;
    uint32_t l = getFileSerialNumber(src, &ok) ^ t;

    if (!ok)
        return (char *)NULL;

    char *basename_f = basename(src);
    /* Length of ULONG_MAX + 1 */
    int len = 11 + strlen(basename_f);

    if (len > NAME_MAX) {
        DLT_LOG(dltsystem, DLT_LOG_WARN,
                DLT_STRING("dlt-system-filetransfer, unique name creation needs to shorten the filename:"),
                DLT_STRING(basename_f));
        len = NAME_MAX;
    }

    char *ret = malloc(len);

    MALLOC_ASSERT(ret);
    snprintf(ret, len, "%010" PRIu32 "%s", l, basename_f);
    return ret;
}

/**
 * Function which only calls the relevant part to transfer the payload
 */

void send_dumped_file(FiletransferOptions const *opts, char *dst_tosend)
{
    /* check if a client is connected to the deamon. If not, try again in a second */
    while (dlt_get_log_state() != 1)
        sleep(1);

    char *fn = origin_name(dst_tosend);
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, sending dumped file:"), DLT_STRING(fn));

    if (dlt_user_log_file_header_alias(&filetransferContext, dst_tosend, fn) == 0) {
        int pkgcount = dlt_user_log_file_packagesCount(&filetransferContext, dst_tosend);
        int lastpkg = 0;
        int success = 1;

        while (lastpkg < pkgcount) {
            int total = 2;
            int used = 2;
            dlt_user_check_buffer(&total, &used);

            while ((total - used) < (total / 2)) {
                struct timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 1000000ul * opts->TimeoutBetweenLogs;
                nanosleep(&t, NULL);
                dlt_user_log_resend_buffer();
                dlt_user_check_buffer(&total, &used);
            }

            lastpkg++;

            if (dlt_user_log_file_data(&filetransferContext, dst_tosend, lastpkg, opts->TimeoutBetweenLogs) < 0) {
                success = 0;
                break;
            }
        }

        if (success)
            dlt_user_log_file_end(&filetransferContext, dst_tosend, 1);
    }

    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, sent dumped file"));
}

/**
 * compress file, delete the source file
 * modification: compress into subdirectory
 * File whis is compress will be deleted afterwards
 *  @param src File to be sent
 *  @param dst destination where to compress the file
 * @param level of compression
 **/
int compress_file_to(char *src, char *dst, int level)
{
    DLT_LOG(dltsystem,
            DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, compressing file from:"),
            DLT_STRING(src),
            DLT_STRING("to:"),
            DLT_STRING(dst));
    char *buf;


    char dst_mode[8];
    snprintf(dst_mode, 8, "wb%d", level);

    gzFile dst_file;
    FILE *src_file;

    dst_file = gzopen(dst, dst_mode);

    if (dst_file == Z_NULL)

        return -1;

    src_file = fopen(src, "r");

    if (src_file == NULL) {
        gzclose(dst_file);

        return -1;
    }

    buf = malloc(Z_CHUNK_SZ);
    MALLOC_ASSERT(buf);

    while (!feof(src_file)) {
        int read = fread(buf, 1, Z_CHUNK_SZ, src_file);

        if (ferror(src_file)) {
            free(buf);

            gzclose(dst_file);
            fclose(src_file);
            return -1;
        }

        gzwrite(dst_file, buf, read);
    }

    if (remove(src) < 0)
        DLT_LOG(dltsystem, DLT_LOG_WARN, DLT_STRING("Could not remove file"), DLT_STRING(src));

    free(buf);
    fclose(src_file);
    gzclose(dst_file);

    return 0;
}

/*!Sends one file over DLT. */
/**
 * If configured in opts, compresses it, then sends it.
 * uses subdirecties for compressing and before sending, to avoid that those files get changed in the meanwhile
 *
 */
int send_one(char *src, FiletransferOptions const *opts, int which)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, sending a file."));

    /* Prepare all needed file names */
    char *fn = basename(src);

    if (fn == NULL) {
        DLT_LOG(dltsystem,
                DLT_LOG_ERROR,
                DLT_STRING("basename not valid"));
        return -1;
    }

    char *src_copy = strndup(src, PATH_MAX);
    MALLOC_ASSERT(src_copy);

    /*dirname overwrites its argument anyway, */
    /*but depending on argument returned address might change */
    char *fdir = dirname(src_copy);

    char *dst_tosend;/*file which is going to be sent */

    char *rn = unique_name(src);/*new unique filename based on inode */

    if (rn == NULL) {
        DLT_LOG(dltsystem,
                DLT_LOG_ERROR,
                DLT_STRING("file information not available, may be file got overwritten"));
        return -1;
    }

    /* Compress if needed */
    if (opts->Compression[which] > 0) {
        DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                DLT_STRING("dlt-system-filetransfer, Moving file to tmp directory for compressing it."));

        char *dst_tocompress;/*file which is going to be compressed, the compressed one is named dst_tosend */


        int len = strlen(fdir) + strlen(SUBDIR_COMPRESS) + strlen(rn) + 3;/*the filename in .tocompress +2 for 2*"/", +1 for \0 */
        dst_tocompress = malloc(len);
        MALLOC_ASSERT(dst_tocompress);

        snprintf(dst_tocompress, len, "%s/%s/%s", fdir, SUBDIR_COMPRESS, rn);

        /*moving in subdir, from where it can be compressed */
        if (rename(src, dst_tocompress) < 0) {
            DLT_LOG(dltsystem, DLT_LOG_ERROR,
                    DLT_STRING("Could not move file"),
                    DLT_STRING(src),
                    DLT_STRING(dst_tocompress));
            free(rn);
            free(dst_tocompress);
            free(src_copy);
            return -1;
        }

        len = strlen(fdir) + strlen(SUBDIR_TOSEND) + strlen(rn) + strlen(COMPRESS_EXTENSION) + 3;/*the resulting filename in .tosend +2 for 2*"/", +1 for \0 */
        dst_tosend = malloc(len);
        MALLOC_ASSERT(dst_tosend);
        snprintf(dst_tosend, len, "%s/%s/%s%s", fdir, SUBDIR_TOSEND, rn, COMPRESS_EXTENSION);

        if (compress_file_to(dst_tocompress, dst_tosend, opts->CompressionLevel[which]) != 0) {
            free(rn);
            free(dst_tosend);
            free(dst_tocompress);
            free(src_copy);
            return -1;
        }

        free(dst_tocompress);

    }
    else {
        /*move it directly into "tosend" */
        DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                DLT_STRING("dlt-system-filetransfer, Moving file to tmp directory."));
        int len = strlen(fdir) + strlen(SUBDIR_TOSEND) + strlen(rn) + 3;
        dst_tosend = malloc(len);/*the resulting filename in .tosend +2 for 2*"/", +1 for \0 */

        snprintf(dst_tosend, len, "%s/%s/%s", fdir, SUBDIR_TOSEND, rn);

        DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                DLT_STRING("dlt-system-filetransfer, Rename:"), DLT_STRING(src), DLT_STRING("to: "),
                DLT_STRING(dst_tosend));

        /*moving in subdir, from where it can be compressed */
        if (rename(src, dst_tosend) < 0) {
            DLT_LOG(dltsystem, DLT_LOG_ERROR,
                    DLT_STRING("Could not move file"),
                    DLT_STRING(src),
                    DLT_STRING(dst_tosend));
            free(rn);
            free(dst_tosend);
            free(src_copy);
            return -1;
        }
    }

    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, File ready to send"));

    send_dumped_file(opts, dst_tosend);


    free(rn);
    free(dst_tosend);
    free(src_copy);

    return 0;
}


int flush_dir_send(FiletransferOptions const *opts, const char *compress_dir, const char *send_dir)
{
    struct dirent *dp;
    DIR *dir;
    dir = opendir(send_dir);

    if (dir != NULL) {
        while ((dp = readdir(dir)) != NULL) {
            if (dp->d_type != DT_REG)
                continue;

            char *fn;
            DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                    DLT_STRING("dlt-system-filetransfer, old compressed file found in send directory:"),
                    DLT_STRING(dp->d_name));
            int len = strlen(send_dir) + strlen(dp->d_name) + 2;
            fn = malloc(len);
            MALLOC_ASSERT(fn);
            snprintf(fn, len, "%s/%s", send_dir, dp->d_name);

            /*if we have a file here and in the to_compress dir, we delete the to_send file: we can not be sure, that it has been properly compressed! */
            if (!strncmp(dp->d_name + strlen(dp->d_name) - strlen(COMPRESS_EXTENSION), COMPRESS_EXTENSION,
                         strlen(COMPRESS_EXTENSION))) {

                /*ends with ".gz" */
                /*old file name (not: path) would have been: */
                char tmp[strlen(dp->d_name) - strlen(COMPRESS_EXTENSION) + 1];
                strncpy(tmp, dp->d_name, strlen(dp->d_name) - strlen(COMPRESS_EXTENSION));
                tmp[strlen(dp->d_name) - strlen(COMPRESS_EXTENSION)] = '\0';

                int len = strlen(tmp) + strlen(compress_dir) + 1 + 1;/*2 sizes + 1*"/" + \0 */
                char *path_uncompressed = malloc(len);
                MALLOC_ASSERT(path_uncompressed);
                snprintf(path_uncompressed, len, "%s/%s", compress_dir, tmp);

                struct stat sb;

                if (stat(path_uncompressed, &sb) == -1) {
                    /*uncompressed equivalent does not exist. We can send it out. */
                    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                            DLT_STRING("dlt-system-filetransfer, sending file."));

                    send_dumped_file(opts, fn);
                }
                else {
                    /*There is an uncompressed file. Compression seems to have been interrupted -> delete the compressed file instead of sending it! */
                    DLT_LOG(dltsystem,
                            DLT_LOG_DEBUG,
                            DLT_STRING(
                                "dlt-system-filetransfer, uncompressed version exists. Deleting partially compressed version."));

                    if (sb.st_mode & S_IFREG) {

                        if (remove(fn) != 0)
                            /*"Error deleting file". Continue? If we would cancel, maybe the dump is never sent! Deletion would again be tried in next LC. */
                            DLT_LOG(dltsystem, DLT_LOG_ERROR,
                                    DLT_STRING("Error deleting file:"), DLT_STRING(fn));
                    }
                    else {
                        /*"Oldfile is a not reg file. Is this possible? Can we compress a directory?: %s\n",path_uncompressed); */
                        DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                                DLT_STRING(
                                    "dlt-system-filetransfer, Oldfile is a not regular file! Do we have a problem?"),
                                DLT_STRING(fn));
                    }
                }

                free(path_uncompressed);/*it is no more used. It would be transferred in next step. */
            }/*it is a .gz file */
            else {
                /*uncompressed file. We can just resend it, the action to put it here was a move action. */
                DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                        DLT_STRING("dlt-system-filetransfer, Sending uncompressed file from previous LC."),
                        DLT_STRING(fn));
                send_dumped_file(opts, fn);
            }

            free(fn);
        }
    }
    else {
        DLT_LOG(dltsystem, DLT_LOG_ERROR,
                DLT_STRING("Could not open directory"),
                DLT_STRING(send_dir));
        return -1;
    }

    closedir(dir);/*end: send_dir */
    return 0;
}


int flush_dir_compress(FiletransferOptions const *opts, int which, const char *compress_dir, const char *send_dir)
{

    /*check for files in compress_dir. Assumption: a file which lies here, should have been compressed, but that action was interrupted. */
    /*As it can arrive here only by a rename, it is most likely to be a complete file */
    struct dirent *dp;
    DIR *dir;
    dir = opendir(compress_dir);

    if (dir != NULL) {
        while ((dp = readdir(dir)) != NULL) {
            if (dp->d_type != DT_REG)
                continue;

            DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                    DLT_STRING("dlt-system-filetransfer, old file found in compress-directory."));


            /*compress file into to_send dir */
            int len = strlen(compress_dir) + strlen(dp->d_name) + 2;
            char *cd_filename = malloc(len);
            MALLOC_ASSERT(cd_filename);
            snprintf(cd_filename, len, "%s/%s", compress_dir, dp->d_name);


            len = strlen(send_dir) + strlen(dp->d_name) + strlen(COMPRESS_EXTENSION) + 2;
            char *dst_tosend = malloc(len);/*the resulting filename in .tosend +2 for 1*"/", +1 for \0 + .gz */
            MALLOC_ASSERT(dst_tosend);
            snprintf(dst_tosend, len, "%s/%s%s", send_dir, dp->d_name, COMPRESS_EXTENSION);

            if (compress_file_to(cd_filename, dst_tosend, opts->CompressionLevel[which]) != 0) {
                free(dst_tosend);
                free(cd_filename);
                closedir(dir);
                return -1;
            }

            /*send file */
            send_dumped_file(opts, dst_tosend);
            free(dst_tosend);
            free(cd_filename);
        }
    }
    else {
        DLT_LOG(dltsystem, DLT_LOG_ERROR,
                DLT_STRING("Could not open directory"),
                DLT_STRING(compress_dir));
        return -1;
    }

    closedir(dir);/*end: compress_dir */

    return 0;
}

int flush_dir_original(FiletransferOptions const *opts, int which)
{
    struct dirent *dp;
    DIR *dir;
    const char *sdir = opts->Directory[which];
    dir = opendir(sdir);

    if (dir != NULL) {
        while ((dp = readdir(dir)) != NULL) {
            if (dp->d_type != DT_REG)
                /*we don't send directories */
                continue;

            DLT_LOG(dltsystem, DLT_LOG_DEBUG,
                    DLT_STRING("dlt-system-filetransfer, old file found in directory."));
            int len = strlen(sdir) + strlen(dp->d_name) + 2;
            char *fn = malloc(len);
            MALLOC_ASSERT(fn);
            snprintf(fn, len, "%s/%s", sdir, dp->d_name);

            if (send_one(fn, opts, which) < 0) {
                closedir(dir);
                free(fn);
                return -1;
            }

            free(fn);
        }
    }
    else {
        DLT_LOG(dltsystem, DLT_LOG_ERROR,
                DLT_STRING("Could not open directory"),
                DLT_STRING(sdir));
        return -1;
    }

    closedir(dir);
    return 0;
}

/*!Cleans the surveyed directories and subdirectories. Sends residing files into trace */
/**
 * @param opts FiletransferOptions
 * @param which which directory is affected -> position in list of opts->Directory
 * @return Returns 0 if everything was okay. If there was a failure a value < 0 will be returned.
 */
int flush_dir(FiletransferOptions const *opts, int which)
{
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, flush directory of old files."));

    char *compress_dir;
    char *send_dir;
    int len = strlen(opts->Directory[which]) + strlen(SUBDIR_COMPRESS) + 2;
    compress_dir = malloc (len);
    MALLOC_ASSERT(compress_dir);
    snprintf(compress_dir, len, "%s/%s", opts->Directory[which], SUBDIR_COMPRESS);

    len = strlen(opts->Directory[which]) + strlen(SUBDIR_TOSEND) + 2;
    send_dir = malloc (len);
    MALLOC_ASSERT(send_dir);
    snprintf(send_dir, len, "%s/%s", opts->Directory[which], SUBDIR_TOSEND);

    /*1st: scan the tosend directory. */
    if (0 != flush_dir_send(opts, compress_dir, send_dir)) {
        free(send_dir);
        free(compress_dir);
        return -1;
    }

    /*1nd: scan the tocompress directory. */
    if (0 != flush_dir_compress(opts, which, compress_dir, send_dir)) {
        free(send_dir);
        free(compress_dir);
        return -1;
    }

    free(send_dir);/*no more used */
    free(compress_dir);

    /*last step: scan the original directory - we can reuse the send_one function */
    if (0 != flush_dir_original(opts, which))
        return -1;

    return 0;
}

/*!Initializes the surveyed directories */
/**On startup, the inotifiy handlers are created, and existing files shall be sent into DLT stream
 * @param config DltSystemConfiguration
 * @return Returns 0 if everything was okay. If there was a failure a value < 0 will be returned.
 */
int init_filetransfer_dirs(DltSystemConfiguration *config)
{
    FiletransferOptions const *opts = &(config->Filetransfer);
    DLT_REGISTER_CONTEXT(filetransferContext, config->Filetransfer.ContextId, "Filetransfer Adapter");
    DLT_LOG(dltsystem, DLT_LOG_DEBUG,
            DLT_STRING("dlt-system-filetransfer, initializing inotify on directories."));
    int i;
#ifdef linux
    ino.handle = inotify_init();

    if (ino.handle < 0) {
        DLT_LOG(filetransferContext, DLT_LOG_FATAL,
                DLT_STRING("Failed to initialize inotify in dlt-system file transfer."));
        return -1;
    }

#endif

    for (i = 0; i < opts->Count; i++) {
        /*create subdirectories for processing the files */

        char *subdirpath;
        int len = strlen(opts->Directory[i]) + strlen(SUBDIR_COMPRESS) + 2;
        subdirpath = malloc (len);
        MALLOC_ASSERT(subdirpath);
        snprintf(subdirpath, len, "%s/%s", opts->Directory[i], SUBDIR_COMPRESS);
        int ret = mkdir(subdirpath, 0777);

        if ((0 != ret) && (EEXIST != errno)) {
            DLT_LOG(dltsystem,
                    DLT_LOG_ERROR,
                    DLT_STRING("dlt-system-filetransfer, error creating subdirectory: "),
                    DLT_STRING(subdirpath),
                    DLT_STRING(" Errorcode: "),
                    DLT_INT(errno));
            free (subdirpath);
            return -1;
        }

        free(subdirpath);

        len = strlen(opts->Directory[i]) + strlen(SUBDIR_TOSEND) + 2;
        subdirpath = malloc (len);
        MALLOC_ASSERT(subdirpath);
        snprintf(subdirpath, len, "%s/%s", opts->Directory[i], SUBDIR_TOSEND);
        ret = mkdir(subdirpath, 0777);

        if ((0 != ret) && (EEXIST != errno)) {
            DLT_LOG(dltsystem,
                    DLT_LOG_ERROR,
                    DLT_STRING("dlt-system-filetransfer, error creating subdirectory: "),
                    DLT_STRING(subdirpath),
                    DLT_STRING(" Errorcode: "),
                    DLT_INT(errno));
            free (subdirpath);
            return -1;
        }

        free(subdirpath);

#ifdef linux
        ino.fd[i] = inotify_add_watch(ino.handle, opts->Directory[i],
                                      IN_CLOSE_WRITE | IN_MOVED_TO);

        if (ino.fd[i] < 0) {
            char buf[1024];
            snprintf(buf, 1024, "Failed to add inotify watch to directory %s in dlt-system file transfer.",
                     opts->Directory[i]);
            DLT_LOG(filetransferContext, DLT_LOG_FATAL,
                    DLT_STRING(buf));
            return -1;
        }

#endif

        flush_dir(opts, i);

    }
    return 0;
}

int process_files(FiletransferOptions const *opts)
{
#ifdef linux
    DLT_LOG(dltsystem, DLT_LOG_DEBUG, DLT_STRING("dlt-system-filetransfer, processing files."));
    static char buf[INOTIFY_LEN];
    ssize_t len = read(ino.handle, buf, INOTIFY_LEN);

    if (len < 0) {
        DLT_LOG(filetransferContext, DLT_LOG_ERROR,
                DLT_STRING("Error while reading events for files in dlt-system file transfer."));
        return -1;
    }

    unsigned int i = 0;

    while (i < (len - INOTIFY_SZ)) {
        struct inotify_event *ie = (struct inotify_event *)&buf[i];

        if (ie->len > 0) {
            if ((ie->mask & IN_CLOSE_WRITE) || (ie->mask & IN_MOVED_TO)) {
                int j;

                for (j = 0; j < opts->Count; j++)
                    if (ie->wd == ino.fd[j]) {
                        DLT_LOG(dltsystem,
                                DLT_LOG_DEBUG,
                                DLT_STRING("dlt-system-filetransfer, found new file."),
                                DLT_STRING(ie->name));
                        int length = strlen(opts->Directory[j]) + ie->len + 1;

                        if (length > PATH_MAX) {
                            DLT_LOG(filetransferContext,
                                    DLT_LOG_ERROR,
                                    DLT_STRING(
                                        "dlt-system-filetransfer: Very long path for file transfer. Cancelling transfer! Length is: "),
                                    DLT_INT(length));
                            return -1;
                        }

                        char *tosend = malloc(length);
                        snprintf(tosend, length, "%s/%s", opts->Directory[j], ie->name);
                        send_one(tosend, opts, j);
                        free(tosend);
                    }
            }
        }

        i += INOTIFY_SZ + ie->len;
    }

#endif
    return 0;
}

void filetransfer_fd_handler(DltSystemConfiguration *config)
{
    process_files(&(config->Filetransfer));
}