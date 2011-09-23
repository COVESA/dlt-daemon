
#define DLT_SHM_KEY  	11771
#define DLT_SHM_SIZE	(1024*4000)
#define DLT_SHM_SEM		22771

typedef struct
{
	int shmid;	// Id of shared memory
	int semid;	// Id of semaphore
	char* shm;	// pointer to beginning of shared memory
	int size; 	// size of data areay in shared memory
	char* mem;	// pointer to data area in shared memory
} DltShm;

#define DLT_SHM_SEM_GET(id) dlt_shm_pv(id,-1)
#define DLT_SHM_SEM_FREE(id) dlt_shm_pv(id,1)

extern int dlt_shm_init_client(DltShm *buf,int key);
extern int dlt_shm_init_server(DltShm *buf,int key,int size);

extern int dlt_shm_push(DltShm *buf,const char *data1, int size1,const char *data2, int size2,const char *data3, int size3);
extern int dlt_shm_pull(DltShm *buf,char *data, int size);

extern void dlt_shm_info(DltShm *buf);
extern void dlt_shm_status(DltShm *buf);

extern int dlt_shm_free_client(DltShm *buf);
extern int dlt_shm_free_server(DltShm *buf);
