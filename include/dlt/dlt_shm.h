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
 * \file dlt_shm.h
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_shm.h                                                     **
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

#ifndef DLT_SHM_H
#define DLT_SHM_H

#include <semaphore.h>
#include "dlt_common.h"

/**
 * Default size of shared memory.
 * size is extended during creation to fit segment size.
 * client retrieves real size from file descriptor of shared memory.
 */
#define DLT_SHM_SIZE   100000

typedef struct
{
    int shmfd;         /* file descriptor of shared memory */
    sem_t *sem;        /* pointer to semaphore */
    DltBuffer buffer;
} DltShm;

typedef struct
{
    char head[4];
    unsigned char status;
    int size;
} DltShmBlockHead;

#define DLT_SHM_SEM_GET(id) sem_wait(id)
#define DLT_SHM_SEM_FREE(id) sem_post(id)

/**
 * Initialise the shared memory on the client side.
 * This function must be called before using further shm functions.
 * @param buf pointer to shm structure
 * @param name the name of the shm, must be the same for server and client
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_shm_init_client(DltShm *buf, const char *name);

/**
 * Initialise the shared memory on the server side.
 * This function must be called before using further shm functions.
 * @param buf pointer to shm structure
 * @param name the name of the shm, must be the same for server and client
 * @param size the requested size of the shm
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_shm_init_server(DltShm *buf, const char *name, int size);

/**
 * Push data from client onto the shm.
 * @param buf pointer to shm structure
 * @param data1 pointer to first data block to be written, null if not used
 * @param size1 size in bytes of first data block to be written, 0 if not used
 * @param data2 pointer to second data block to be written, null if not used
 * @param size2 size in bytes of second data block to be written, 0 if not used
 * @param data3 pointer to third data block to be written, null if not used
 * @param size3 size in bytes of third data block to be written, 0 if not used
 * @return negative value if there was an error
 */
extern int dlt_shm_push(DltShm *buf,
                        const unsigned char *data1,
                        unsigned int size1,
                        const unsigned char *data2,
                        unsigned int size2,
                        const unsigned char *data3,
                        unsigned int size3);

/**
 * Pull data from shm.
 * This function should be called from client.
 * Data is deleted from shm after this call.
 * @param buf pointer to shm structure
 * @param data pointer to buffer where data is to be written
 * @param size maximum size to be written into buffer
 * @return negative value if there was an error
 */
extern int dlt_shm_pull(DltShm *buf, unsigned char *data, int size);

/**
 * Copy message from shm.
 * This function should be called from server.
 * Data is not deleted from shm after this call.
 * @param buf pointer to shm structure
 * @param data pointer to buffer where data is to be written
 * @param size maximum size to be written into buffer
 * @return negative value if there was an error
 */
extern int dlt_shm_copy(DltShm *buf, unsigned char *data, int size);

/**
 * Delete message from shm.
 * This function should be called from server.
 * This function should be called after each succesful copy.
 * @param buf pointer to shm structure
 * @return negative value if there was an error
 */
extern int dlt_shm_remove(DltShm *buf);

/**
 * Print information about shm.
 * @param buf pointer to shm structure
 */
extern void dlt_shm_info(DltShm *buf);

/**
 * Print status about shm.
 * @param buf pointer to shm structure
 */
extern void dlt_shm_status(DltShm *buf);

/**
 * Deinitialise the shared memory on the client side.
 * @param buf pointer to shm structure
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_shm_free_client(DltShm *buf);

/**
 * Returns the total size of the shm.
 * @param buf pointer to shm structure
 * @return size of the shared memory.
 */
extern int dlt_shm_get_total_size(DltShm *buf);

/**
 * Returns the used size in the shm.
 * @param buf pointer to shm structure
 * @return size of the shared memory.
 */
extern int dlt_shm_get_used_size(DltShm *buf);

/**
 * Returns the number of messages in the shm.
 * @param buf pointer to shm structure
 * @return size of the shared memory.
 */
extern int dlt_shm_get_message_count(DltShm *buf);

/**
 * Reset pointers and counters when shm corrupted.
 * @param buf pointer to shm structure
 * @return size of the shared memory.
 */
extern int dlt_shm_reset(DltShm *buf);

/**
 * Recover to find next valid message.
 * @param buf pointer to shm structure
 * @return size of the shared memory.
 */
extern int dlt_shm_recover(DltShm *buf);

/**
 * Deinitialise the shared memory on the server side.
 * @param buf pointer to shm structure
 * @param name name of the shared memory
 * @return negative value if there was an error
 */
extern DltReturnValue dlt_shm_free_server(DltShm *buf, const char *name);

#endif /* DLT_SHM_H */
