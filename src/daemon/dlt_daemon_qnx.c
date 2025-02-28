#ifdef __QNX__

#include "dlt_daemon_qnx.h"
#include <unistd.h>  // For close()
#include <pthread.h> // For pthread_t

int dlt_timer_pipes[DLT_TIMER_UNKNOWN][2] = {
    [DLT_TIMER_PACKET] = {DLT_FD_INIT, DLT_FD_INIT},
    [DLT_TIMER_ECU] = {DLT_FD_INIT, DLT_FD_INIT},
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = {DLT_FD_INIT, DLT_FD_INIT},
#endif
    [DLT_TIMER_GATEWAY] = {DLT_FD_INIT, DLT_FD_INIT}
};

pthread_t timer_threads[DLT_TIMER_UNKNOWN] = {
    [DLT_TIMER_PACKET] = 0,
    [DLT_TIMER_ECU] = 0,
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = 0,
#endif
    [DLT_TIMER_GATEWAY] = 0
};

DltDaemonPeriodicData *timer_data[DLT_TIMER_UNKNOWN] = {
    [DLT_TIMER_PACKET] = NULL,
    [DLT_TIMER_ECU] = NULL,
#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
    [DLT_TIMER_SYSTEMD] = NULL,
#endif
    [DLT_TIMER_GATEWAY] = NULL
};

void close_pipes(int fds[2])
{
    if (fds[0] > 0) {
        close(fds[0]);
        fds[0] = DLT_FD_INIT;
    }

    if (fds[1] > 0) {
        close(fds[1]);
        fds[1] = DLT_FD_INIT;
    }
}

void dlt_daemon_cleanup_timers()
{
    int i = 0;
    while (i < DLT_TIMER_UNKNOWN) {
        /* Remove FIFO of every timer and kill timer thread */
        if (0 != timer_threads[i]) {
            pthread_kill(timer_threads[i], SIGUSR1);
            pthread_join(timer_threads[i], NULL);
            timer_threads[i] = 0;

            close_pipes(dlt_timer_pipes[i]);

            /* Free data of every timer */
            if (NULL != timer_data[i]) {
                free(timer_data[i]);
                timer_data[i] = NULL;
            }
        }
        i++;
    }
}

static void *timer_thread(void *data)
{
    int pexit = 0;
    unsigned int sleep_ret = 0;

    DltDaemonPeriodicData* timer_thread_data = (DltDaemonPeriodicData*) data;

    /* Timer will start in starts_in sec*/
    if ((sleep_ret = sleep(timer_thread_data->starts_in))) {
        dlt_vlog(LOG_NOTICE, "Sleep remains [%u] for starting!"
                "Stop thread of timer [%d]\n",
                sleep_ret, timer_thread_data->timer_id);
         close_pipes(dlt_timer_pipes[timer_thread_data->timer_id]);
         return NULL;
    }

    while (1) {
        if ((dlt_timer_pipes[timer_thread_data->timer_id][1] > 0) &&
                (0 > write(dlt_timer_pipes[timer_thread_data->timer_id][1], "1", 1))) {
            dlt_vlog(LOG_ERR, "Failed to send notification for timer [%s]!\n",
                    dlt_timer_names[timer_thread_data->timer_id]);
            pexit = 1;
        }

        if (pexit || g_exit) {
            dlt_vlog(LOG_NOTICE, "Received signal!"
                    "Stop thread of timer [%d]\n",
                    timer_thread_data->timer_id);
            close_pipes(dlt_timer_pipes[timer_thread_data->timer_id]);
            return NULL;
        }

        if ((sleep_ret = sleep(timer_thread_data->period_sec))) {
            dlt_vlog(LOG_NOTICE, "Sleep remains [%u] for interval!"
                    "Stop thread of timer [%d]\n",
                    sleep_ret, timer_thread_data->timer_id);
             close_pipes(dlt_timer_pipes[timer_thread_data->timer_id]);
             return NULL;
        }
    }
}

dlt_daemon_cleanup_timers();
#endif // __QNX__
