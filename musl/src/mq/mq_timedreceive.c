#include <mqueue.h>
#include "sys/syscall.h"
#include "bits/syscall.h"

ssize_t mq_timedreceive(mqd_t mqd, char * msg, size_t len, unsigned * prio, const struct timespec * at)
{
	return syscall(SYS_mq_timedreceive, mqd, msg, len, prio, at);
}
