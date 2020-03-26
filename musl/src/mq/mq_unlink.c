#include <mqueue.h>
#include <errno.h>
#include "sys/syscall.h"
#include "bits/syscall.h"

int mq_unlink(const char *name)
{
	int ret;
	if (*name == '/') name++;
	ret = syscall(SYS_mq_unlink, name);
	if (ret < 0) {
		if (ret == -EPERM) ret = -EACCES;
		errno = -ret;
		return -1;
	}
	return ret;
}
