#include <mqueue.h>
#include "sys/syscall.h"
#include "bits/syscall.h"

int mq_close(mqd_t mqd)
{
	return syscall(SYS_close, mqd);
}
