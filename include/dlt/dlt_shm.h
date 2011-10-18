/*
* Dlt- Diagnostic Log and Trace user library
* @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
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

/* shared memory key */
/* must be the same for server and cleint */
#define DLT_SHM_KEY  	11771

/* default size of shared memory */
/* size is extended during creation to fit segment size */
/* client retreives real size from shm buffer */
#define DLT_SHM_SIZE	100000

/* Id of the used semaphore */
/* used for synchronisation of write and read access of multiple clients and server */
/* must be the same for server and client */
#define DLT_SHM_SEM		22771

#define DLT_SHM_HEAD 	"SHM"

typedef struct
{
	int shmid;	/* Id of shared memory */
	int semid;	/* Id of semaphore */
	char* shm;	/* pointer to beginning of shared memory */
	int size; 	/* size of data area in shared memory */
	char* mem;	/* pointer to data area in shared memory */
} DltShm;

#define DLT_SHM_SEM_GET(id) dlt_shm_pv(id,-1)
#define DLT_SHM_SEM_FREE(id) dlt_shm_pv(id,1)

/**
 * Initialise the shared memory on the client side.
 * This function must be called before using further shm functions.
 * @param buf pointer to shm structure
 * @param key the identifier of the shm, must be the same for server and client
 * @return negative value if there was an error
 */
extern int dlt_shm_init_client(DltShm *buf,int key);

/**
 * Initialise the shared memory on the server side.
 * This function must be called before using further shm functions.
 * @param buf pointer to shm structure
 * @param key the identifier of the shm, must be the same for server and client
 * @param size the requested size of the shm
 * @return negative value if there was an error
 */
extern int dlt_shm_init_server(DltShm *buf,int key,int size);

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
extern int dlt_shm_push(DltShm *buf,const unsigned char *data1,unsigned int size1,const unsigned char *data2,unsigned int size2,const unsigned char *data3,unsigned int size3);

/**
 * Pull data from shm.
 * This function should be called from client.
 * Data is deleted from shm after this call.
 * @param buf pointer to shm structure
 * @param data pointer to buffer where data is to be written
 * @param size maximum size to be written into buffer
 * @return negative value if there was an error
 */
extern int dlt_shm_pull(DltShm *buf,unsigned char *data, int size);

/**
 * Copy message from shm.
 * This function should be called from server.
 * Data is not deleted from shm after this call.
 * @param buf pointer to shm structure
 * @param data pointer to buffer where data is to be written
 * @param size maximum size to be written into buffer
 * @return negative value if there was an error
 */
extern int dlt_shm_copy(DltShm *buf,unsigned char *data, int size);

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
extern int dlt_shm_free_client(DltShm *buf);

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
 * @return negative value if there was an error
 */
extern int dlt_shm_free_server(DltShm *buf);

#endif /* DLT_SHM_H */
