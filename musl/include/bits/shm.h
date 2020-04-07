#define SHMLBA 4096

struct shmid_ds
{
	struct ipc_perm shm_perm;
	size_t shm_segsz;
	time_t shm_atime;
	int __unused1;
	time_t shm_dtime;
	int __unused2;
	time_t shm_ctime;
	int __unused3;
	pid_t shm_cpid;
	pid_t shm_lpid;
	unsigned long shm_nattch;
	unsigned long __pad1;
	unsigned long __pad2;
};

struct shminfo {
	unsigned long shmmax, shmmin, shmmni, shmseg, shmall, __unused_[4];
};

struct shm_info {
	int __used_ids;
	unsigned long shm_tot, shm_rss, shm_swp;
	unsigned long __swap_attempts, __swap_successes;
};

