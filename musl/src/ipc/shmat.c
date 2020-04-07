#include <sys/shm.h>
#include "sys/syscall.h"
#include "ipc.h"
#include "bits/syscall.h"

#ifdef SYS_shmat
void *shmat(int id, const void *addr, int flag)
{
	return (void *)syscall(SYS_shmat, id, addr, flag);
}
#else
void *shmat(int id, const void *addr, int flag)
{
	unsigned long ret;
	ret = syscall(SYS_ipc, IPCOP_shmat, id, flag, &addr, addr);
	return (ret > -(unsigned long)SHMLBA) ? (void *)ret : (void *)addr;
}
#endif
