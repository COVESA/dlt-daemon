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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#if !defined(_MSC_VER)
#include <unistd.h>
#include <syslog.h>
#endif

#include <dlt_shm.h>
#include <dlt_common.h>

void dlt_shm_print_hex(char *ptr,int size)
{
    int num;

    for (num=0;num<size;num++)
    {
		if((num%16)==15)
			printf("%.2x\n",((unsigned char*)ptr)[num]);
		else
			printf("%.2x ",((unsigned char*)ptr)[num]);
    }
    printf("\n");
}

void dlt_shm_pv(int id,int operation)
{
	static struct sembuf semaphor;
	
	semaphor.sem_op = operation;
	semaphor.sem_flg = SEM_UNDO;
	
	if(semop(id, &semaphor,1) == -1) {
		dlt_log(LOG_ERR,"SHM: semop");
	}
}

int dlt_shm_init_server(DltShm *buf,int key,int size) {
	struct shmid_ds shm_buf;
	char str[256];
	unsigned char *ptr;

	// Init parameters
	buf->shmid = 0;
	buf->semid = 0;

    // Create the segment.
    if ((buf->shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
        dlt_log(LOG_ERR,"SHM: shmget");
        return -1; /* ERROR */
    }

	// get the size of shm
	if (shmctl(buf->shmid,  IPC_STAT, &shm_buf))
	{
		dlt_log(LOG_ERR,"SHM: shmctl");
        return -1; /* ERROR */
	}	

    // Now we attach the segment to our data space.
    if ((ptr = shmat(buf->shmid, NULL, 0)) == (char *) -1) {
        dlt_log(LOG_ERR,"SHM: shmat");
        return -1; /* ERROR */
    }
	
	// Init semaphore
	if( (buf->semid = semget(DLT_SHM_SEM,1,S_IRWXU|S_IRWXG|S_IRWXO|IPC_CREAT|IPC_EXCL)) == -1 ) {
		if( (buf->semid = semget(DLT_SHM_SEM,1,S_IRWXU|S_IRWXG|S_IRWXO|IPC_EXCL)) == -1 ) {
			dlt_log(LOG_ERR,"SHM: semget");
			return -1; /* ERROR */
		}
	}
	if( semctl(buf->semid,0,SETVAL,(int)1) == -1 ) {
        dlt_log(LOG_ERR,"SHM: semctl");
        return -1; /* ERROR */
	}
	
	// init buffer
	dlt_buffer_init_static_server(&(buf->buffer),ptr,shm_buf.shm_segsz);

	return 0; /* OK */
}

int dlt_shm_init_client(DltShm *buf,int key) {
	struct shmid_ds shm_buf;
	unsigned char *ptr;

	// init parameters
	buf->shmid = 0;
	buf->semid = 0;

    // Create the segment.
    if ((buf->shmid = shmget(key, 0, 0666)) < 0) {
        dlt_log(LOG_ERR,"SHM: shmget");
        return -1; /* ERROR */
    }

	// get the size of shm
	if (shmctl(buf->shmid,  IPC_STAT, &shm_buf))
	{
	    dlt_log(LOG_ERR,"SHM: shmctl");
            return -1; /* ERROR */
	}	

	// Now we attach the segment to our data space.
	if ((ptr = shmat(buf->shmid, NULL, 0)) == (char *) -1) {
		dlt_log(LOG_ERR,"shmat");
		return -1; /* ERROR */
	}
        	
	// Init semaphore
	if( (buf->semid = semget(DLT_SHM_SEM,0,0)) == -1 ) {
            dlt_log(LOG_ERR,"SHM: semget");
            return -1; /* ERROR */
	}

	// init buffer
	dlt_buffer_init_static_client(&(buf->buffer),ptr,shm_buf.shm_segsz);
    
	return 0; /* OK */
}

void dlt_shm_info(DltShm *buf)
{
	dlt_buffer_info(&(buf->buffer));
}

void dlt_shm_status(DltShm *buf)
{
	dlt_buffer_status(&(buf->buffer));
}

int dlt_shm_get_total_size(DltShm *buf)
{
	return dlt_buffer_get_total_size(&(buf->buffer));
}

int dlt_shm_get_used_size(DltShm *buf)
{
	int ret;

	DLT_SHM_SEM_GET(buf->semid);
	ret =  dlt_buffer_get_used_size(&(buf->buffer));
	DLT_SHM_SEM_FREE(buf->semid);

	return ret;
}

int dlt_shm_get_message_count(DltShm *buf)
{
	return dlt_buffer_get_message_count(&(buf->buffer));
}

int dlt_shm_push(DltShm *buf,const unsigned char *data1,unsigned int size1,const unsigned char *data2,unsigned int size2,const unsigned char *data3,unsigned int size3)
{
	int ret;

	DLT_SHM_SEM_GET(buf->semid);
	ret =  dlt_buffer_push3(&(buf->buffer),data1,size1,data2,size2,data3,size3);
	DLT_SHM_SEM_FREE(buf->semid);

	return ret;
}

int dlt_shm_pull(DltShm *buf,unsigned char *data, int max_size)
{
	int ret;

	DLT_SHM_SEM_GET(buf->semid);
	ret =  dlt_buffer_pull(&(buf->buffer),data,max_size);
	DLT_SHM_SEM_FREE(buf->semid);

	return ret;
}

int dlt_shm_copy(DltShm *buf,unsigned char *data, int max_size)
{
	int ret;

	DLT_SHM_SEM_GET(buf->semid);
	ret =  dlt_buffer_copy(&(buf->buffer),data,max_size);
	DLT_SHM_SEM_FREE(buf->semid);

	return ret;
}

int dlt_shm_remove(DltShm *buf)
{
	int ret;

	DLT_SHM_SEM_GET(buf->semid);
	ret =  dlt_buffer_remove(&(buf->buffer));
	DLT_SHM_SEM_FREE(buf->semid);

	return ret;
}

int dlt_shm_free_server(DltShm *buf) {
		
	if(shmdt(buf->buffer.shm)) {
        dlt_log(LOG_ERR,"SHM: shmdt");
        return -1; /* ERROR */
    }

	if(shmctl(buf->shmid,IPC_RMID,NULL) == -1) {
        dlt_log(LOG_ERR,"SHM: shmdt");
        return -1; /* ERROR */
	}

	if(semctl(buf->semid,0,IPC_RMID,(int)0) == -1) {
        dlt_log(LOG_ERR,"SHM: shmdt");
        return -1; /* ERROR */
	}

	// Reset parameters
	buf->shmid = 0;
	buf->semid = 0;

	return dlt_buffer_free_static(&(buf->buffer));
		
}

int dlt_shm_free_client(DltShm *buf) {

	if(shmdt(buf->buffer.shm)) {
        dlt_log(LOG_ERR,"SHM: shmdt");
        return -1; /* ERROR */
    }

	// Reset parameters
	buf->shmid = 0;
	buf->semid = 0;
		
	return dlt_buffer_free_static(&(buf->buffer));
}
