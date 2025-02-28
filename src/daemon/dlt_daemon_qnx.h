#ifndef DLT_DAEMON_QNX_H
#define DLT_DAEMON_QNX_H

#ifdef __QNX__
#include "dlt_daemon_common.h"  // Include necessary shared definitions

int dlt_timer_pipes[DLT_TIMER_UNKNOWN][2];
pthread_t timer_threads[DLT_TIMER_UNKNOWN];
DltDaemonPeriodicData *timer_data[DLT_TIMER_UNKNOWN];

void dlt_daemon_cleanup_timers();
void close_pipes(int fds[2]);

#endif // __QNX__
#endif // DLT_DAEMON_QNX_H
