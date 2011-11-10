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

	// Init parameters
	buf->shm = NULL;
	buf->shmid = 0;
	buf->semid = 0;
	buf->size = 0;
	buf->mem = 0;

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
    if ((buf->shm = shmat(buf->shmid, NULL, 0)) == (char *) -1) {
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
	
	// Init pointers
    ((int*)(buf->shm))[0] = 0;  // pointer to write memory  
    ((int*)(buf->shm))[1] = 0;  // pointer to read memory
    ((int*)(buf->shm))[2] = 0;  // number of packets
    buf->mem = (char*)(&(((int*)(buf->shm))[3]));
    buf->size = shm_buf.shm_segsz - (buf->mem - buf->shm);

	// clear memory
	memset(buf->mem,0,buf->size);
    
	snprintf(str,sizeof(str),"SHM: Size %d\n",buf->size);
	dlt_log(LOG_INFO, str);

	return 0; /* OK */
}

int dlt_shm_init_client(DltShm *buf,int key) {
	struct shmid_ds shm_buf;

	// init parameters
	buf->shm = NULL;
	buf->shmid = 0;
	buf->semid = 0;
	buf->size = 0;
	buf->mem = 0;

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
	if ((buf->shm = shmat(buf->shmid, NULL, 0)) == (char *) -1) {
		dlt_log(LOG_ERR,"shmat");
		return -1; /* ERROR */
	}
        	
	// Init semaphore
	if( (buf->semid = semget(DLT_SHM_SEM,0,0)) == -1 ) {
            dlt_log(LOG_ERR,"SHM: semget");
            return -1; /* ERROR */
	}

	// Init pointers
	buf->mem = (char*)(&(((int*)(buf->shm))[3]));
	buf->size = shm_buf.shm_segsz - (buf->mem - buf->shm);
    
	return 0; /* OK */
}

void dlt_shm_info(DltShm *buf)
{
	char str[256];

	snprintf(str,sizeof(str),"SHM: SHM id: %d\n",buf->shmid);
	dlt_log(LOG_INFO, str);
	snprintf(str,sizeof(str),"SHM: Available size: %d\n",buf->size);
	dlt_log(LOG_INFO, str);
	snprintf(str,sizeof(str),"SHM: SHM full start address: %lX\n",(unsigned long)buf->shm);
	dlt_log(LOG_INFO, str);
	snprintf(str,sizeof(str),"SHM: SHM start address: %lX\n",(unsigned long)buf->mem);
	dlt_log(LOG_INFO, str);

}

void dlt_shm_status(DltShm *buf)
{
	int write, read, count;
	char str[256];

	write = ((int*)(buf->shm))[0];
	read = ((int*)(buf->shm))[1];
	count = ((int*)(buf->shm))[2];

	snprintf(str,sizeof(str),"SHM: Write: %d\n",write);
	dlt_log(LOG_INFO, str);
	snprintf(str,sizeof(str),"SHM: Read: %d\n",read);
	dlt_log(LOG_INFO, str);
	snprintf(str,sizeof(str),"SHM: Count: %d\n",count);
	dlt_log(LOG_INFO, str);
}

int dlt_shm_get_total_size(DltShm *buf)
{
	return buf->size;
}

int dlt_shm_get_used_size(DltShm *buf)
{
	int write, read, count;

	DLT_SHM_SEM_GET(buf->semid);
	write = ((int*)(buf->shm))[0];
	read = ((int*)(buf->shm))[1];
	count = ((int*)(buf->shm))[2];
	DLT_SHM_SEM_FREE(buf->semid);

	if(count == 0)
		return 0;
		
	if(write>read)
		return (write - read);
	
	return (buf->size - read + write);
}

int dlt_shm_get_message_count(DltShm *buf)
{
	return ((int*)(buf->shm))[2];
}

void dlt_shm_write_block(DltShm *buf,int *write, const unsigned char *data,unsigned int size)
{
	if((*write+size) <= buf->size) {
		// write one block
		memcpy(buf->mem+*write,data,size);
		*write += size;
	}
	else {
		// write two blocks
		memcpy(buf->mem+*write, data, buf->size-*write);
		memcpy(buf->mem, data+buf->size-*write, size-buf->size+*write);
		*write += size-buf->size;
	}
}

void dlt_shm_read_block(DltShm *buf,int *read,unsigned char *data,unsigned int size)
{
	if((*read+size) <= buf->size) {
		// read one block
		memcpy(data,buf->mem+*read,size);
		*read += size;
	}
	else {
		// read two blocks
		memcpy(data, buf->mem+*read, buf->size-*read);
		memcpy(data+buf->size-*read, buf->mem, size-buf->size+*read);
		*read += size-buf->size;
	}
}
int dlt_shm_push(DltShm *buf,const unsigned char *data1,unsigned int size1,const unsigned char *data2,unsigned int size2,const unsigned char *data3,unsigned int size3)
{
	int free_size;	
	int write, read;
	DltShmBlockHead head;
	
	if(!buf->mem) {
		// shm not initialised
		dlt_log(LOG_ERR,"SHM: SHM not initialised\n");
		return -1; /* ERROR */
	}

	// get semaphore
	DLT_SHM_SEM_GET(buf->semid);

	// get current write pointer
	write = ((int*)(buf->shm))[0];
	read = ((int*)(buf->shm))[1];

	// check pointers
	if((read>buf->size) || (write>buf->size))
	{
		dlt_log(LOG_ERR,"SHM: Pointer out of range\n");
		dlt_shm_reset(buf);
		return -1; // ERROR		
	}

	// calculate free size
	if(read>write)
		free_size = read - write;
	else
		free_size = buf->size - write + read;
	
	// check size
	if(free_size < (sizeof(DltShmBlockHead)+size1+size2+size3)) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: SHM is full\n");
		return -1; // ERROR
	}

	// set header
	strcpy(head.head,DLT_SHM_HEAD);
	head.status = 2;
	head.size = size1+size2+size3;

	// write data
	dlt_shm_write_block(buf,&write,(unsigned char*)&head,sizeof(DltShmBlockHead));
	if(size1) dlt_shm_write_block(buf,&write,data1,size1);
	if(size2) dlt_shm_write_block(buf,&write,data2,size2);
	if(size3) dlt_shm_write_block(buf,&write,data3,size3);

	// update global shm pointers
	((int*)(buf->shm))[0] = write; // set new write pointer 	
	((int*)(buf->shm))[2] += 1; // increase counter

	// free semaphore
	DLT_SHM_SEM_FREE(buf->semid);

	return 0; // OK
}

int dlt_shm_pull(DltShm *buf,unsigned char *data, int max_size)
{
	int used_size;	
	int write, read, count;
	char head_compare[] = DLT_SHM_HEAD;
	DltShmBlockHead head;
	
	if(!buf->mem) {
		// shm not initialised
		dlt_log(LOG_ERR,"SHM: SHM not initialised\n");
		return -1; /* ERROR */
	}

	// get current write pointer
	DLT_SHM_SEM_GET(buf->semid);
	write = ((int*)(buf->shm))[0];
	read = ((int*)(buf->shm))[1];
	count = ((int*)(buf->shm))[2];

	// check pointers
	if((read>buf->size) || (write>buf->size) || (count<0))
	{
		dlt_log(LOG_ERR,"SHM: Pointer out of range\n");
		dlt_shm_reset(buf);
		return -1; // ERROR		
	}

	// check if data is in there
	if(count==0) {
		DLT_SHM_SEM_FREE(buf->semid);
		if(write!=read)
		{
			dlt_log(LOG_ERR,"SHM: SHM should be empty, but is not\n");
			dlt_shm_reset(buf);
		}
		return -1; // ERROR		
	}

	// calculate used size
	if(write>read)
		used_size = write - read;
	else	
		used_size = buf->size - read + write;

	// first check size
	if(used_size < (sizeof(DltShmBlockHead))) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Size check 1 failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// read header
	dlt_shm_read_block(buf,&read,(unsigned char*)&head,sizeof(DltShmBlockHead));

	// check header
	if(memcmp((unsigned char*)(head.head),head_compare,sizeof(head_compare))!=0)
	{
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Header head check failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}
	if(head.status != 2)
	{
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Header status check failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// second check size
	if(used_size < (sizeof(DltShmBlockHead)+head.size)) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Size check 2 failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// third check size
	if(head.size > max_size) {
		dlt_log(LOG_ERR,"SHM: Size check 3 failed\n");
		// nothing to do but data does not fit provided buffer
	}

	// read data
	dlt_shm_read_block(buf,&read,data,head.size);

	// update buffer pointers
	((int*)(buf->shm))[1] = read; // set new read pointer 	
	((int*)(buf->shm))[2] -= 1; // decrease counter

	DLT_SHM_SEM_FREE(buf->semid);

	return head.size; // OK
}

int dlt_shm_copy(DltShm *buf,unsigned char *data, int max_size)
{
	int used_size;	
	int write, read, count;
	char head_compare[] = DLT_SHM_HEAD;
	DltShmBlockHead head;
	
	if(!buf->mem) {
		// shm not initialised
		dlt_log(LOG_ERR,"SHM: SHM not initialised\n");
		return -1; /* ERROR */
	}

	// get current write pointer
	DLT_SHM_SEM_GET(buf->semid);
	write = ((int*)(buf->shm))[0];
	read = ((int*)(buf->shm))[1];
	count = ((int*)(buf->shm))[2];

	// check pointers
	if((read>buf->size) || (write>buf->size) || (count<0))
	{
		dlt_log(LOG_ERR,"SHM: Pointer out of range\n");
		dlt_shm_reset(buf);
		return -1; // ERROR		
	}

	// check if data is in there
	if(count==0) {
		DLT_SHM_SEM_FREE(buf->semid);
		if(write!=read)
		{
			dlt_log(LOG_ERR,"SHM: SHM should be empty, but is not\n");
			dlt_shm_reset(buf);
		}
		return -1; // ERROR		
	}

	// calculate used size
	if(write>read)
		used_size = write - read;
	else	
		used_size = buf->size - read + write;

	// first check size
	if(used_size < (sizeof(DltShmBlockHead))) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Size check 1 failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// read header
	dlt_shm_read_block(buf,&read,(unsigned char*)&head,sizeof(DltShmBlockHead));

	// check header
	if(memcmp((unsigned char*)(head.head),head_compare,sizeof(head_compare))!=0)
	{
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Header head check failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}
	if(head.status != 2)
	{
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Header status check failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// second check size
	if(used_size < (sizeof(DltShmBlockHead)+head.size)) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Size check 2 failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// third check size
	if(head.size > max_size) {
		dlt_log(LOG_ERR,"SHM: Size check 3 failed\n");
		// nothing to do but data does not fit provided buffer
	}

	// read data
	dlt_shm_read_block(buf,&read,data,head.size);

	// do not update pointers

	DLT_SHM_SEM_FREE(buf->semid);

	return head.size; // OK
}

int dlt_shm_remove(DltShm *buf)
{
	int used_size;	
	int write, read, count;
	char head_compare[] = DLT_SHM_HEAD;
	DltShmBlockHead head;
	
	if(!buf->mem) {
		// shm not initialised
		dlt_log(LOG_ERR,"SHM: SHM not initialised\n");
		return -1; /* ERROR */
	}

	// get current write pointer
	DLT_SHM_SEM_GET(buf->semid);
	write = ((int*)(buf->shm))[0];
	read = ((int*)(buf->shm))[1];
	count = ((int*)(buf->shm))[2];

	// check pointers
	if((read>buf->size) || (write>buf->size) || (count<0))
	{
		dlt_log(LOG_ERR,"SHM: Pointer out of range\n");
		dlt_shm_reset(buf);
		return -1; // ERROR		
	}

	// check if data is in there
	if(count==0) {
		DLT_SHM_SEM_FREE(buf->semid);
		if(write!=read)
		{
			dlt_log(LOG_ERR,"SHM: SHM should be empty, but is not\n");
			dlt_shm_reset(buf);
		}
		return -1; // ERROR		
	}

	// calculate used size
	if(write>read)
		used_size = write - read;
	else	
		used_size = buf->size - read + write;

	// first check size
	if(used_size < (sizeof(DltShmBlockHead))) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Size check 1 failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// read header
	dlt_shm_read_block(buf,&read,(unsigned char*)&head,sizeof(DltShmBlockHead));

	// check header
	if(memcmp((unsigned char*)(head.head),head_compare,sizeof(head_compare))!=0)
	{
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Header head check failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}
	if(head.status != 2)
	{
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Header status check failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// second check size
	if(used_size < (sizeof(DltShmBlockHead)+head.size)) {
		DLT_SHM_SEM_FREE(buf->semid);
		dlt_log(LOG_ERR,"SHM: Size check 2 failed\n");
		dlt_shm_reset(buf);
		return -1; // ERROR
	}

	// do not copy data

	// update buffer pointers
	if( (read+head.size) <= buf->size)
		((int*)(buf->shm))[1] = read+head.size; // set new read pointer 	
	else
		((int*)(buf->shm))[1] = read+head.size-buf->size; // set new read pointer 		
	((int*)(buf->shm))[2] -= 1; // decrease counter

	DLT_SHM_SEM_FREE(buf->semid);

	return head.size; // OK	
}

int dlt_shm_reset(DltShm *buf) {
	
	dlt_log(LOG_ERR,"SHM: SHM reset triggered.\n");

	/* reset pointers and counters */	
	DLT_SHM_SEM_GET(buf->semid);
	((int*)(buf->shm))[0] = 0;  // pointer to write memory  
	((int*)(buf->shm))[1] = 0;  // pointer to read memory
	((int*)(buf->shm))[2] = 0;  // number of packets
	DLT_SHM_SEM_FREE(buf->semid);

	return 0; /* OK */
}

int dlt_shm_recover(DltShm *buf) {

	return -1; /* OK */
}

int dlt_shm_free_server(DltShm *buf) {

	if(!buf->shm) {
        dlt_log(LOG_ERR,"SHM: Shared memory segment not attached\n");
        return -1; /* ERROR */
    }
		
	if(shmdt(buf->shm)) {
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
	buf->shm = NULL;
	buf->shmid = 0;
	buf->semid = 0;
	buf->size = 0;
	buf->mem = 0;
		
	return 0; /* OK */
}

int dlt_shm_free_client(DltShm *buf) {

	if(!buf->shm) {
        dlt_log(LOG_ERR,"SHM: Shared memory segment not attached\n");
        return -1; /* ERROR */
    }
		
	if(shmdt(buf->shm)) {
        dlt_log(LOG_ERR,"SHM: shmdt");
        return -1; /* ERROR */
    }

	// Reset parameters
	buf->shm = NULL;
	buf->shmid = 0;
	buf->semid = 0;
	buf->size = 0;
	buf->mem = 0;
		
	return 0; /* OK */
}
