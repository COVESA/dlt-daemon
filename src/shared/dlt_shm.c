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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_shm.c
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_shm.c                                                     **
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

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
*******************************************************************************/

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if !defined(_MSC_VER)
#include <unistd.h>
#include <syslog.h>
#endif

#include <dlt_shm.h>
#include <dlt_common.h>

void dlt_shm_print_hex(char *ptr, int size)
{
    int num;

    for (num = 0; num < size; num++) {
        if ((num % 16) == 15)
            printf("%.2x\n", ((unsigned char *)ptr)[num]);
        else
            printf("%.2x ", ((unsigned char *)ptr)[num]);
    }

    printf("\n");
}

DltReturnValue dlt_shm_init_server(DltShm *buf, const char *name, int size)
{
    unsigned char *ptr;

    /* Check if buffer and name available */
    if (buf == NULL || name == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* Init parameters */
    buf->shmfd = 0;
    buf->sem = NULL;

    /**
     * Create the shared memory segment.
     * @name Name of the new shared memory object.
     * (shm_open will create a file under /dev/shm/<name>)
     * @oflag O_CREAT | O_EXCL | O_RDWR
     * O_CREAT | O_EXCL: The shm_open() fails if the shared memory exists.
     * O_RDWR: Open the object for read-write access.
     * @mode 0666
     * The shared memory object's permission.
     */
    buf->shmfd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (buf->shmfd == -1)
    {
        dlt_vlog(LOG_ERR, "%s: shm_open() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /* Set the size of shm */
    if (ftruncate(buf->shmfd, size) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: ftruncate() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /* Now we attach the segment to our data space. */
    ptr = (unsigned char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                                buf->shmfd, 0);
    if (ptr == MAP_FAILED)
    {
        dlt_vlog(LOG_ERR, "%s: mmap() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /**
     * Create the semaphore with input name
     * @name name
     * Name of the new semaphore
     * (sem_open will create a file under /dev/shm/sem.<name>)
     * @oflag O_CREAT | O_EXCEL
     * The sem_open() fails if the semaphore name exists.
     * @mode 0777
     * The permissions to be placed on the new semaphore.
     * @value 1
     * Initial value for the new semaphore.
     */
    buf->sem = sem_open(name, O_CREAT | O_EXCL, 0666, 1);
    if (buf->sem == SEM_FAILED)
    {
        dlt_vlog(LOG_ERR, "%s: sem_open() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /* Init buffer */
    dlt_buffer_init_static_server(&(buf->buffer), ptr, size);

    /* The 'buf->shmfd' is no longer needed */
    if (close(buf->shmfd) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: Failed to close shared memory"
                    " file descriptor: %s\n", __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    return DLT_RETURN_OK; /* OK */
}

DltReturnValue dlt_shm_init_client(DltShm *buf, const char *name)
{
    struct stat shm_buf;
    unsigned char *ptr;

    /* Check if buffer and name available */
    if (buf == NULL || name == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* Init parameters */
    buf->shmfd = 0;
    buf->sem = NULL;

    /**
     * Open the existing shared memory segment created by the server.
     * @name Name of the existing shared memory object.
     * (shm_open will open the file under /dev/shm/<name>)
     * @oflag O_RDWR
     * Open the object for read-write access.
     * @mode 0
     * We are not creating a new object, this argument should be specified as 0.
     */
    buf->shmfd = shm_open(name, O_RDWR, 0);
    if (buf->shmfd == -1)
    {
        dlt_vlog(LOG_ERR, "%s: shm_open() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /* Get the size of shm */
    if (fstat(buf->shmfd, &shm_buf) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: fstat() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /* Now we attach the segment to our data space. */
    ptr = (unsigned char *)mmap(NULL, shm_buf.st_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, buf->shmfd, 0);
    if (ptr == MAP_FAILED)
    {
        dlt_vlog(LOG_ERR, "%s: mmap() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /**
     * Open the existing semaphore with name created by the server
     * The call requires only two arguments.
     * @name name
     * Name of the existing semaphore
     * (sem_open will open a file under /dev/shm/sem.<name>)
     * @oflag 0
     * We are accessing an existing semaphore, oflag should be specified as 0.
     */
    buf->sem = sem_open(name, 0);
    if (buf->sem == SEM_FAILED)
    {
        dlt_vlog(LOG_ERR, "%s: sem_open() failed: %s\n",
                 __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    /* Init buffer */
    dlt_buffer_init_static_client(&(buf->buffer), ptr, shm_buf.st_size);

    /* The 'buf->shmfd' is no longer needed */
    if (close(buf->shmfd) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: Failed to close shared memory"
                    " file descriptor: %s\n", __func__, strerror(errno));
        return DLT_RETURN_ERROR; /* ERROR */
    }

    return DLT_RETURN_OK; /* OK */
}

void dlt_shm_info(DltShm *buf)
{
    /* Check if buffer available */
    if (buf == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return;
    }

    dlt_buffer_info(&(buf->buffer));
}

void dlt_shm_status(DltShm *buf)
{
    /* Check if buffer available */
    if (buf == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return;
    }

    dlt_buffer_status(&(buf->buffer));
}

int dlt_shm_get_total_size(DltShm *buf)
{
    /* Check if buffer available */
    if (buf == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }

    return dlt_buffer_get_total_size(&(buf->buffer));
}

int dlt_shm_get_used_size(DltShm *buf)
{
    int ret;

    /* Check if buffer available */
    if ((buf == NULL) || (buf->buffer.mem == NULL))
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }

    DLT_SHM_SEM_GET(buf->sem);
    ret =  dlt_buffer_get_used_size(&(buf->buffer));
    DLT_SHM_SEM_FREE(buf->sem);

    return ret;
}

int dlt_shm_get_message_count(DltShm *buf)
{
    /* Check if buffer available */
    if (buf == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }
    return dlt_buffer_get_message_count(&(buf->buffer));
}

int dlt_shm_push(DltShm *buf,
                 const unsigned char *data1,
                 unsigned int size1,
                 const unsigned char *data2,
                 unsigned int size2,
                 const unsigned char *data3,
                 unsigned int size3)
{
    int ret;

    /* Check if buffer available */
    if ((buf == NULL) || (buf->buffer.mem == NULL))
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }

    DLT_SHM_SEM_GET(buf->sem);
    ret =  dlt_buffer_push3(&(buf->buffer),
                            data1, size1, data2, size2, data3, size3);
    DLT_SHM_SEM_FREE(buf->sem);

    return ret;
}

int dlt_shm_pull(DltShm *buf, unsigned char *data, int max_size)
{
    int ret;

    /* Check if buffer available */
    if ((buf == NULL) || (buf->buffer.mem == NULL))
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }

    DLT_SHM_SEM_GET(buf->sem);
    ret = dlt_buffer_pull(&(buf->buffer), data, max_size);
    DLT_SHM_SEM_FREE(buf->sem);

    return ret;
}

int dlt_shm_copy(DltShm *buf, unsigned char *data, int max_size)
{
    int ret;

    /* Check if buffer available */
    if ((buf == NULL) || (buf->buffer.mem == NULL))
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }

    DLT_SHM_SEM_GET(buf->sem);
    ret = dlt_buffer_copy(&(buf->buffer), data, max_size);
    DLT_SHM_SEM_FREE(buf->sem);

    return ret;
}

int dlt_shm_remove(DltShm *buf)
{
    int ret;

    /* Check if buffer available */
    if ((buf == NULL) || (buf->buffer.mem == NULL))
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return -1;
    }

    DLT_SHM_SEM_GET(buf->sem);
    ret = dlt_buffer_remove(&(buf->buffer));
    DLT_SHM_SEM_FREE(buf->sem);

    return ret;
}

DltReturnValue dlt_shm_free_server(DltShm *buf, const char *name)
{
    if ((buf == NULL) || (buf->buffer.shm == NULL) || name == NULL)
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (munmap(buf->buffer.shm, buf->buffer.min_size) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: munmap() failed: %s\n",
                 __func__, strerror(errno));
    }

    if (shm_unlink(name) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: shm_unlink() failed: %s\n",
                 __func__, strerror(errno));
    }

    if (sem_close(buf->sem) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: sem_close() failed: %s\n",
                 __func__, strerror(errno));
    }

    if (sem_unlink(name) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: sem_unlink() failed: %s\n",
                 __func__, strerror(errno));
    }

    /* Reset parameters */
    buf->shmfd = 0;
    buf->sem = NULL;

    return dlt_buffer_free_static(&(buf->buffer));
}

DltReturnValue dlt_shm_free_client(DltShm *buf)
{
    if ((buf == NULL) || (buf->buffer.shm == NULL))
    {
        dlt_vlog(LOG_ERR, "%s: Wrong parameter: Null pointer\n", __func__);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (munmap(buf->buffer.shm, buf->buffer.min_size) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: munmap() failed: %s\n",
                 __func__, strerror(errno));
    }

    if (sem_close(buf->sem) == -1)
    {
        dlt_vlog(LOG_ERR, "%s: sem_close() failed: %s\n",
                 __func__, strerror(errno));
    }

    /* Reset parameters */
    buf->shmfd = 0;
    buf->sem = NULL;

    return dlt_buffer_free_static(&(buf->buffer));
}
