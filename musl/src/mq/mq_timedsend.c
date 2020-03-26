#include <mqueue.h>
#include "sys/syscall.h"
#include "bits/syscall.h"

int mq_timedsend(mqd_t mqd, const char *msg, size_t len, unsigned prio, const struct timespec *at)
{
	return syscall(SYS_mq_timedsend, mqd, msg, len, prio, at);
}
