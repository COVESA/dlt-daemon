/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_daemon_common.c
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_common.c                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
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
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>  /* send() */
#include <sys/socket.h> /* send() */

#include "dlt_types.h"
#include "dlt_daemon_common.h"
#include "dlt_daemon_common_cfg.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt-daemon.h"

#include "dlt_daemon_socket.h"
#include "dlt_daemon_serial.h"

static char str[DLT_DAEMON_COMMON_TEXTBUFSIZE];

sem_t dlt_daemon_mutex;

static int dlt_daemon_cmp_apid(const void *m1, const void *m2)
{
    if(m1 == NULL || m2 == NULL)
        return -1;

    DltDaemonApplication *mi1 = (DltDaemonApplication *) m1;
    DltDaemonApplication *mi2 = (DltDaemonApplication *) m2;

    return memcmp(mi1->apid, mi2->apid, DLT_ID_SIZE);
}

static int dlt_daemon_cmp_apid_ctid(const void *m1, const void *m2)
{
    if(m1 == NULL || m2 == NULL)
        return -1;

    int ret, cmp;
    DltDaemonContext *mi1 = (DltDaemonContext *) m1;
    DltDaemonContext *mi2 = (DltDaemonContext *) m2;

    cmp=memcmp(mi1->apid, mi2->apid, DLT_ID_SIZE);
    if (cmp<0)
    {
        ret=-1;
    }
    else if (cmp==0)
    {
        ret=memcmp(mi1->ctid, mi2->ctid, DLT_ID_SIZE);
    }
    else
    {
        ret=1;
    }

    return ret;
}

int dlt_daemon_init(DltDaemon *daemon,unsigned long RingbufferMinSize,unsigned long RingbufferMaxSize,unsigned long RingbufferStepSize,const char *runtime_directory,int InitialContextLogLevel, int InitialContextTraceStatus, int ForceLLTS, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL || runtime_directory == NULL)
        return -1;

    int append_length = 0;
    daemon->num_contexts = 0;
    daemon->contexts = NULL;

    daemon->num_applications = 0;
    daemon->applications = NULL;

    daemon->default_log_level = InitialContextLogLevel;
    daemon->default_trace_status = InitialContextTraceStatus;
    daemon->force_ll_ts = ForceLLTS;

    daemon->overflow_counter = 0;

    daemon->runtime_context_cfg_loaded = 0;

    daemon->mode = DLT_USER_MODE_EXTERNAL;

    daemon->connectionState = 0; /* no logger connected */

    daemon->state = DLT_DAEMON_STATE_INIT; /* initial logging state */

    /* prepare filenames for configuration */

    append_length = PATH_MAX - sizeof(DLT_RUNTIME_APPLICATION_CFG);
    if(runtime_directory[0])
    {
        strncpy(daemon->runtime_application_cfg,runtime_directory,append_length);
        daemon->runtime_application_cfg[append_length]=0;
    }
    else
    {
        strncpy(daemon->runtime_application_cfg,DLT_RUNTIME_DEFAULT_DIRECTORY,append_length);
        daemon->runtime_application_cfg[append_length]=0;
    }
    strcat(daemon->runtime_application_cfg,DLT_RUNTIME_APPLICATION_CFG); /* strcat uncritical here, because max length already checked */

    append_length = PATH_MAX - sizeof(DLT_RUNTIME_CONTEXT_CFG);
    if(runtime_directory[0])
    {
        strncpy(daemon->runtime_context_cfg,runtime_directory,append_length);
        daemon->runtime_context_cfg[append_length]=0;
    }
    else
    {
        strncpy(daemon->runtime_context_cfg,DLT_RUNTIME_DEFAULT_DIRECTORY,append_length);
        daemon->runtime_context_cfg[append_length]=0;
    }
    strcat(daemon->runtime_context_cfg,DLT_RUNTIME_CONTEXT_CFG); /* strcat uncritical here, because max length already checked */

    append_length = PATH_MAX - sizeof(DLT_RUNTIME_CONFIGURATION);
    if(runtime_directory[0])
    {
        strncpy(daemon->runtime_configuration,runtime_directory,append_length);
        daemon->runtime_configuration[append_length]=0;
    }
    else
    {
        strncpy(daemon->runtime_configuration,DLT_RUNTIME_DEFAULT_DIRECTORY,append_length);
        daemon->runtime_configuration[append_length]=0;
    }
    strcat(daemon->runtime_configuration,DLT_RUNTIME_CONFIGURATION); /* strcat uncritical here, because max length already checked */

    /* Check for runtime cfg, if it is loadable, load it! */
    if ((dlt_daemon_applications_load(daemon,daemon->runtime_application_cfg, verbose)==0) &&
            (dlt_daemon_contexts_load(daemon,daemon->runtime_context_cfg, verbose)==0))
    {
        daemon->runtime_context_cfg_loaded = 1;
    }

    /* load configuration if available */
    dlt_daemon_configuration_load(daemon,daemon->runtime_configuration, verbose);

    daemon->sendserialheader = 0;
    daemon->timingpackets = 0;

    dlt_set_id(daemon->ecuid,"");

    /* initialize ring buffer for client connection */
    snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE,"Ringbuffer configuration: %lu/%lu/%lu\n", RingbufferMinSize,RingbufferMaxSize,RingbufferStepSize );
    dlt_log(LOG_INFO, str);
    if (dlt_buffer_init_dynamic(&(daemon->client_ringbuffer), RingbufferMinSize,RingbufferMaxSize,RingbufferStepSize) == DLT_RETURN_ERROR)
    {
        return -1;
    }

    daemon->storage_handle = NULL;
    return 0;
}

int dlt_daemon_free(DltDaemon *daemon, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return -1;

    /* Free contexts */
    if (dlt_daemon_contexts_clear(daemon, verbose)==-1)
        return -1;

    /* Free applications */
    if (dlt_daemon_applications_clear(daemon, verbose)==-1)
        return -1;

    /* free ringbuffer */
    dlt_buffer_free_dynamic(&(daemon->client_ringbuffer));

    return 0;
}

int dlt_daemon_applications_invalidate_fd(DltDaemon *daemon, int fd, int verbose)
{
    int i;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return -1;

    for (i=0; i<daemon->num_applications; i++)
    {
        if (daemon->applications[i].user_handle==fd)
        {
            daemon->applications[i].user_handle = DLT_FD_INIT;
        }
    }

    return 0;
}

int dlt_daemon_applications_clear(DltDaemon *daemon, int verbose)
{
    int i;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return -1;

    for (i=0; i<daemon->num_applications; i++)
    {
        if (daemon->applications[i].application_description != NULL)
        {
            free(daemon->applications[i].application_description);
            daemon->applications[i].application_description = NULL;
        }
    }

    if (daemon->applications != NULL)
    {
        free(daemon->applications);
    }

    daemon->applications = NULL;
    daemon->num_applications = 0;

    return 0;
}

DltDaemonApplication* dlt_daemon_application_add(DltDaemon *daemon, char *apid, pid_t pid, char *description, int verbose)
{
    DltDaemonApplication *application;
    DltDaemonApplication *old;
    int new_application;
    int dlt_user_handle;
    char filename[DLT_DAEMON_COMMON_TEXTBUFSIZE];

    if ((daemon == NULL) || (apid == NULL) || (apid[0]=='\0'))
    {
        return (DltDaemonApplication*) NULL;
    }

    if (daemon->applications == NULL)
    {
        daemon->applications = (DltDaemonApplication*) malloc(sizeof(DltDaemonApplication)*DLT_DAEMON_APPL_ALLOC_SIZE);
        if (daemon->applications == NULL)
        {
            return (DltDaemonApplication*) NULL;
        }
    }

    new_application=0;

    /* Check if application [apid] is already available */
    application = dlt_daemon_application_find(daemon, apid, verbose);
    if (application == NULL)
    {
        daemon->num_applications += 1;

        if (daemon->num_applications!=0)
        {
            if ((daemon->num_applications%DLT_DAEMON_APPL_ALLOC_SIZE)==0)
            {
                /* allocate memory in steps of DLT_DAEMON_APPL_ALLOC_SIZE, e.g. 100 */
                old = daemon->applications;
                daemon->applications = (DltDaemonApplication*) malloc(sizeof(DltDaemonApplication)*
                                       ((daemon->num_applications/DLT_DAEMON_APPL_ALLOC_SIZE)+1)*DLT_DAEMON_APPL_ALLOC_SIZE);
                if (daemon->applications == NULL)
                {
                    daemon->applications = old;
                    daemon->num_applications -= 1;
                    return (DltDaemonApplication*) NULL;
                }
                memcpy(daemon->applications,old,sizeof(DltDaemonApplication)*daemon->num_applications);
                free(old);
            }
        }

        application = &(daemon->applications[daemon->num_applications-1]);

        dlt_set_id(application->apid,apid);
        application->pid = 0;
        application->application_description = NULL;
        application->num_contexts = 0;
        application->user_handle = DLT_FD_INIT;

        new_application = 1;

    } else if (pid != application->pid) {

        snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "Duplicate registration of ApplicationID: '%.4s'; registering from PID %d, existing from PID %d\n",apid, pid, application->pid);
        dlt_log(LOG_WARNING, str);

    }

    /* Store application description and pid of application */
    if (application->application_description)
    {
        free(application->application_description);
        application->application_description = NULL;
    }

    if (description != NULL)
    {
        application->application_description = malloc(strlen(description)+1);
        if (application->application_description)
        {
            strncpy(application->application_description,description,strlen(description));
            application->application_description[strlen(description)]='\0';
        }
    }

    if( application->user_handle != DLT_FD_INIT )
    {
        if( application->pid != pid )
        {
    		if ( close(application->user_handle) < 0 )
    		{
    			snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "close() failed to %s/dltpipes/dlt%d, errno=%d (%s)!\n",dltFifoBaseDir,pid,errno,strerror(errno)); /* errno 2: ENOENT - No such file or directory */
    		    dlt_log(LOG_WARNING, str);
    		}

    		application->user_handle = DLT_FD_INIT;
    		application->pid = 0;
        }
    }

    /* open user pipe only if it is not yet opened */
    if (application->user_handle == DLT_FD_INIT && pid != 0)
    {
        snprintf(filename,DLT_DAEMON_COMMON_TEXTBUFSIZE,"%s/dltpipes/dlt%d",dltFifoBaseDir,pid);

        dlt_user_handle = open(filename, O_WRONLY|O_NONBLOCK);
        if ( dlt_user_handle < 0 )
        {
            snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "open() failed to %s, errno=%d (%s)!\n",filename,errno,strerror(errno)); /* errno 2: ENOENT - No such file or directory */
            dlt_log(LOG_WARNING, str);
        } /* if */

        /* check if file file descriptor was already used, and make it invalid if it is reused */
        /* This prevents sending messages to wrong file descriptor */
        dlt_daemon_applications_invalidate_fd(daemon,dlt_user_handle,verbose);
        dlt_daemon_contexts_invalidate_fd(daemon,dlt_user_handle,verbose);

        application->pid = pid;
        application->user_handle = dlt_user_handle;
    }

    /* Sort */
    if (new_application)
    {
        qsort(daemon->applications,daemon->num_applications,sizeof(DltDaemonApplication),dlt_daemon_cmp_apid);

        /* Find new position of application with apid*/
        application = dlt_daemon_application_find(daemon, apid, verbose);
    }

    return application;
}

int dlt_daemon_application_del(DltDaemon *daemon, DltDaemonApplication *application, int verbose)
{
    int pos;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (application == NULL))
    {
        return -1;
    }

    if (daemon->num_applications > 0)
    {
        /* Check if user handle is open; if yes, close it */
        if (application->user_handle >= DLT_FD_MINIMUM)
        {
            close(application->user_handle);
            application->user_handle = DLT_FD_INIT;
        }

        /* Free description of application to be deleted */
        if (application->application_description)
        {
            free(application->application_description);
            application->application_description = NULL;
        }

        pos = application-(daemon->applications);

        /* move all applications above pos to pos */
        memmove(&(daemon->applications[pos]),&(daemon->applications[pos+1]), sizeof(DltDaemonApplication)*((daemon->num_applications-1)-pos));

        /* Clear last application */
        memset(&(daemon->applications[daemon->num_applications-1]), 0, sizeof(DltDaemonApplication));

        daemon->num_applications--;

    }

    return 0;
}

DltDaemonApplication* dlt_daemon_application_find(DltDaemon *daemon, char *apid, int verbose)
{
    DltDaemonApplication application;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (apid == NULL) || (apid[0]=='\0') || (daemon->num_applications==0))
    {
        return (DltDaemonApplication*) NULL;
    }

    /* Check, if apid is smaller than smallest apid or greater than greatest apid */
    if ((memcmp(apid,daemon->applications[0].apid, DLT_ID_SIZE) < 0) ||
            (memcmp(apid,daemon->applications[daemon->num_applications-1].apid, DLT_ID_SIZE) > 0))
    {
        return (DltDaemonApplication*) NULL;
    }

    dlt_set_id(application.apid, apid);
    return (DltDaemonApplication*)bsearch(&application, daemon->applications, daemon->num_applications, sizeof(DltDaemonApplication), dlt_daemon_cmp_apid);
}

int dlt_daemon_applications_load(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    ID4 apid;
    char buf[DLT_DAEMON_COMMON_TEXTBUFSIZE];
    char *ret;
    char *pb;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0]=='\0'))
        return -1;

    fd=fopen(filename, "r");

    if (fd == NULL)
    {
        snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "DLT runtime-application load, cannot open file %s: %s\n", filename, strerror(errno));
        dlt_log(LOG_WARNING, str);

        return -1;
    }

    while (!feof(fd))
    {
        /* Clear buf */
        memset(buf, 0, sizeof(buf));

        /* Get line */
        ret=fgets(buf,sizeof(buf),fd);
        if (NULL == ret)
        {
            /* fgets always null pointer if the last byte of the file is a new line
             * We need to check here if there was an error or was it feof.*/
            if(ferror(fd))
            {
                snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "dlt_daemon_applications_load fgets(buf,sizeof(buf),fd) returned NULL. %s\n",
                         strerror(errno));
                dlt_log(LOG_WARNING, str);
                fclose(fd);
                return -1;
            }
            else if(feof(fd))
            {
                fclose(fd);
                return 0;
            }
            else {
                snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "dlt_daemon_applications_load fgets(buf,sizeof(buf),fd) returned NULL. Unknown error.\n");
                dlt_log(LOG_WARNING, str);
                fclose(fd);
                return -1;
            }
        }

        if (strcmp(buf,"") != 0)
        {
            /* Split line */
            pb=strtok(buf,":");
            if(pb != NULL)
            {
                dlt_set_id(apid,pb);
                pb=strtok(NULL,":");
                if(pb != NULL)
                {
                    /* pb contains now the description */
                    /* pid is unknown at loading time */
                    if (dlt_daemon_application_add(daemon, apid, 0, pb, verbose) == 0)
                    {
                        dlt_log(LOG_WARNING, "dlt_daemon_applications_load dlt_daemon_application_add failed\n");
                        fclose(fd);
                        return -1;
                    }
                }
            }
        }
    }
    fclose(fd);

    return 0;
}

int dlt_daemon_applications_save(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    int i;

    char apid[DLT_ID_SIZE+1]; /* DLT_ID_SIZE+1, because the 0-termination is required here */

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0]=='\0'))
        return -1;

    memset(apid, 0, sizeof(apid));

    if ((daemon->applications != NULL) && (daemon->num_applications > 0))
    {
        fd=fopen(filename, "w");
        if (fd != NULL)
        {
            for (i=0; i<daemon->num_applications; i++)
            {
                dlt_set_id(apid,daemon->applications[i].apid);

                if ((daemon->applications[i].application_description) &&
                        (daemon->applications[i].application_description[0] != '\0'))
                {
                    fprintf(fd,"%s:%s:\n",apid, daemon->applications[i].application_description);
                }
                else
                {
                    fprintf(fd,"%s::\n",apid);
                }
            }
            fclose(fd);
        }
    }

    return 0;
}

DltDaemonContext* dlt_daemon_context_add(DltDaemon *daemon, char *apid, char *ctid,int8_t log_level, int8_t trace_status, int log_level_pos, int user_handle, char *description, int verbose)
{
    DltDaemonApplication *application;
    DltDaemonContext *context;
    DltDaemonContext *old;
    int new_context=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (apid == NULL) || (apid[0]=='\0') || (ctid == NULL) || (ctid[0]=='\0'))
    {
        return (DltDaemonContext*) NULL;
    }

    if ((log_level<DLT_LOG_DEFAULT) || (log_level>DLT_LOG_VERBOSE))
    {
        return (DltDaemonContext*) NULL;
    }

    if ((trace_status<DLT_TRACE_STATUS_DEFAULT) || (trace_status>DLT_TRACE_STATUS_ON))
    {
        return (DltDaemonContext*) NULL;
    }

    if (daemon->contexts == NULL)
    {
        daemon->contexts = (DltDaemonContext*) malloc(sizeof(DltDaemonContext)*DLT_DAEMON_CONTEXT_ALLOC_SIZE);
        if (daemon->contexts == NULL)
        {
            return (DltDaemonContext*) NULL;
        }
    }

    /* Check if application [apid] is available */
    application = dlt_daemon_application_find(daemon, apid, verbose);
    if (application == NULL)
    {
        return (DltDaemonContext*) NULL;
    }

    /* Check if context [apid, ctid] is already available */
    context = dlt_daemon_context_find(daemon, apid, ctid, verbose);
    if (context == NULL)
    {
        daemon->num_contexts += 1;

        if (daemon->num_contexts!=0)
        {
            if ((daemon->num_contexts%DLT_DAEMON_CONTEXT_ALLOC_SIZE)==0)
            {
                /* allocate memory for context in steps of DLT_DAEMON_CONTEXT_ALLOC_SIZE, e.g 100 */
                old = daemon->contexts;
                daemon->contexts = (DltDaemonContext*) malloc(sizeof(DltDaemonContext)*
                                   ((daemon->num_contexts/DLT_DAEMON_CONTEXT_ALLOC_SIZE)+1)*DLT_DAEMON_CONTEXT_ALLOC_SIZE);
                if (daemon->contexts == NULL)
                {
                    daemon->contexts = old;
                    daemon->num_contexts -= 1;
                    return (DltDaemonContext*) NULL;
                }
                memcpy(daemon->contexts,old,sizeof(DltDaemonContext)*daemon->num_contexts);
                free(old);
            }
        }

        context = &(daemon->contexts[daemon->num_contexts-1]);

        dlt_set_id(context->apid,apid);
        dlt_set_id(context->ctid,ctid);
        context->context_description = NULL;

        application->num_contexts++;
        new_context =1;
    }

    /* Set context description */
    if (context->context_description)
    {
        free(context->context_description);
        context->context_description = NULL;
    }

    if (description != NULL)
    {
        context->context_description = malloc(strlen(description)+1);

        if (context->context_description)
        {
            strncpy(context->context_description,description,strlen(description));
            context->context_description[strlen(description)]='\0';
        }
    }

    if (daemon->force_ll_ts)
    {
        if (log_level > daemon->default_log_level)
        {
            log_level = daemon->default_log_level;
        }
        if (trace_status > daemon->default_trace_status)
        {
            trace_status = daemon->default_trace_status;
        }
        snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "Adapting ll_ts for context: %.4s:%.4s with %i %i\n", apid, ctid, log_level, trace_status);
        dlt_log(LOG_NOTICE, str);
    }

    /* Store log level and trace status,
       if this is a new context, or
       if this is an old context and the runtime cfg was not loaded */

    if ((new_context == 1) ||
            ((new_context == 0) && (daemon->runtime_context_cfg_loaded == 0)))
    {
        context->log_level = log_level;
        context->trace_status = trace_status;
    }

    context->log_level_pos = log_level_pos;
    context->user_handle = user_handle;

    /* Sort */
    if (new_context)
    {
        qsort(daemon->contexts,daemon->num_contexts, sizeof(DltDaemonContext),dlt_daemon_cmp_apid_ctid);

        /* Find new position of context with apid, ctid */
        context = dlt_daemon_context_find(daemon, apid, ctid, verbose);
    }

    return context;
}

int dlt_daemon_context_del(DltDaemon *daemon, DltDaemonContext* context, int verbose)
{
    int pos;
    DltDaemonApplication *application;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (context == NULL))
        return -1;

    if (daemon->num_contexts>0)
    {
        application = dlt_daemon_application_find(daemon, context->apid, verbose);

        /* Free description of context to be deleted */
        if (context->context_description)
        {
            free(context->context_description);
            context->context_description = NULL;
        }

        pos = context-(daemon->contexts);

        /* move all contexts above pos to pos */
        memmove(&(daemon->contexts[pos]), &(daemon->contexts[pos+1]), sizeof(DltDaemonContext)*((daemon->num_contexts-1)-pos));

        /* Clear last context */
        memset(&(daemon->contexts[daemon->num_contexts-1]), 0, sizeof(DltDaemonContext));

        daemon->num_contexts--;

        /* Check if application [apid] is available */
        if (application != NULL)
        {
            application->num_contexts--;
        }
    }

    return 0;
}

DltDaemonContext* dlt_daemon_context_find(DltDaemon *daemon, char *apid, char *ctid, int verbose)
{
    DltDaemonContext context;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (apid == NULL) || (apid[0]=='\0') || (ctid == NULL) || (ctid[0]=='\0') || (daemon->num_contexts==0))
    {
        return (DltDaemonContext*) NULL;
    }

    /* Check, if apid is smaller than smallest apid or greater than greatest apid */
    if ((memcmp(apid,daemon->contexts[0].apid,DLT_ID_SIZE)<0) ||
            (memcmp(apid,daemon->contexts[daemon->num_contexts-1].apid,DLT_ID_SIZE)>0))
    {
        return (DltDaemonContext*) NULL;
    }

    dlt_set_id(context.apid,apid);
    dlt_set_id(context.ctid,ctid);

    return (DltDaemonContext*)bsearch(&context,daemon->contexts,daemon->num_contexts,sizeof(DltDaemonContext),dlt_daemon_cmp_apid_ctid);
}

int dlt_daemon_contexts_invalidate_fd(DltDaemon *daemon, int fd, int verbose)
{
    int i;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return -1;

    for (i=0; i<daemon->num_contexts; i++)
    {
        if (daemon->contexts[i].user_handle == fd)
        {
            daemon->contexts[i].user_handle = DLT_FD_INIT;
        }
    }

    return 0;
}

int dlt_daemon_contexts_clear(DltDaemon *daemon, int verbose)
{
    int i;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
        return -1;

    for (i=0; i<daemon->num_contexts; i++)
    {
        if (daemon->contexts[i].context_description != NULL)
        {
            free(daemon->contexts[i].context_description);
            daemon->contexts[i].context_description = NULL;
        }
    }

    if (daemon->contexts)
    {
        free(daemon->contexts);
    }

    daemon->contexts = NULL;

    for (i=0; i<daemon->num_applications; i++)
    {
        daemon->applications[i].num_contexts = 0;
    }

    daemon->num_contexts = 0;

    return 0;
}

int dlt_daemon_contexts_load(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;
    ID4 apid, ctid;
    char buf[DLT_DAEMON_COMMON_TEXTBUFSIZE];
    char *ret;
    char *pb;
    int ll, ts;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) ||( filename[0]=='\0'))
        return -1;

    fd=fopen(filename, "r");

    if (fd == NULL)
    {
        snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "DLT runtime-context load, cannot open file %s: %s\n", filename, strerror(errno));
        dlt_log(LOG_WARNING, str);

        return -1;
    }

    while (!feof(fd))
    {
        /* Clear buf */
        memset(buf, 0, sizeof(buf));

        /* Get line */
        ret=fgets(buf,sizeof(buf),fd);
        if (NULL == ret)
        {
            /* fgets always returns null pointer if the last byte of the file is a new line.
             * We need to check here if there was an error or was it feof.*/
            if(ferror(fd))
            {
                snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "dlt_daemon_contexts_load fgets(buf,sizeof(buf),fd) returned NULL. %s\n",
                         strerror(errno));
                dlt_log(LOG_WARNING, str);
                fclose(fd);
                return -1;
            }
            else if(feof(fd))
            {
                fclose(fd);
                return 0;
            }
            else {
                snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE, "dlt_daemon_contexts_load fgets(buf,sizeof(buf),fd) returned NULL. Unknown error.\n");
                dlt_log(LOG_WARNING, str);
                fclose(fd);
                return -1;
            }
        }

        if (strcmp(buf,"")!=0)
        {
            /* Split line */
            pb=strtok(buf,":");
            if(pb != NULL)
            {
                dlt_set_id(apid,pb);
                pb=strtok(NULL,":");
                if(pb != NULL)
                {
                    dlt_set_id(ctid,pb);
                    pb=strtok(NULL,":");
                    if(pb != NULL)
                    {
                        sscanf(pb,"%d",&ll);
                        pb=strtok(NULL,":");
                        if(pb != NULL)
                        {
                            sscanf(pb,"%d",&ts);
                            pb=strtok(NULL,":");
                            if(pb != NULL)
                            {
                                /* pb contains now the description */

                                /* log_level_pos, and user_handle are unknown at loading time */
                                if (dlt_daemon_context_add(daemon, apid, ctid, (int8_t)ll, (int8_t)ts, 0, 0, pb, verbose) == NULL)
                                {
                                    dlt_log(LOG_WARNING, "dlt_daemon_contexts_load dlt_daemon_context_add failed\n");
                                    fclose(fd);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    fclose(fd);

    return 0;
}

int dlt_daemon_contexts_save(DltDaemon *daemon,const char *filename, int verbose)
{
    FILE *fd;
    int i;

    char apid[DLT_ID_SIZE+1], ctid[DLT_ID_SIZE+1]; /* DLT_ID_SIZE+1, because the 0-termination is required here */

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0]=='\0'))
        return -1;

    memset(apid, 0, sizeof(apid));
    memset(ctid, 0, sizeof(ctid));

    if ((daemon->contexts) && (daemon->num_contexts > 0))
    {
        fd=fopen(filename, "w");
        if (fd != NULL)
        {
            for (i=0; i<daemon->num_contexts; i++)
            {
                dlt_set_id(apid,daemon->contexts[i].apid);
                dlt_set_id(ctid,daemon->contexts[i].ctid);

                if ((daemon->contexts[i].context_description) &&
                        (daemon->contexts[i].context_description[0]!='\0'))
                {
                    fprintf(fd,"%s:%s:%d:%d:%s:\n",apid,ctid,
                            (int)(daemon->contexts[i].log_level),
                            (int)(daemon->contexts[i].trace_status),
                            daemon->contexts[i].context_description);
                }
                else
                {
                    fprintf(fd,"%s:%s:%d:%d::\n",apid,ctid,
                            (int)(daemon->contexts[i].log_level),
                            (int)(daemon->contexts[i].trace_status));
                }
            }
            fclose(fd);
        }
    }

    return 0;
}

int dlt_daemon_configuration_save(DltDaemon *daemon, const char *filename, int verbose)
{
    FILE *fd;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename[0] == '\0'))
        return -1;

    fd=fopen(filename, "w");
    if (fd != NULL)
    {
        fprintf(fd,"# 0 = off, 1 = external, 2 = internal, 3 = both\n");
        fprintf(fd,"LoggingMode = %d\n",daemon->mode);

        fclose(fd);
    }

    return 0;
}

int dlt_daemon_configuration_load(DltDaemon *daemon, const char *filename, int verbose)
{
    if(daemon == NULL || filename == NULL)
        return -1;

    FILE * pFile;
    char line[1024];
    char token[1024];
    char value[1024];
    char *pch;

    PRINT_FUNCTION_VERBOSE(verbose);

    pFile = fopen (filename,"r");

    if (pFile != NULL)
    {
        while(1)
        {
            /* fetch line from configuration file */
            if ( fgets (line , 1024 , pFile) != NULL )
            {
                  pch = strtok (line," =\r\n");
                  token[0]=0;
                  value[0]=0;

                  while (pch != NULL)
                  {
                    if(strcmp(pch,"#")==0)
                        break;

                    if(token[0]==0)
                    {
                        strncpy(token,pch,sizeof(token)-1);
                        token[sizeof(token)-1]=0;
                    }
                    else
                    {
                        strncpy(value,pch,sizeof(value)-1);
                        value[sizeof(value)-1]=0;
                        break;
                    }

                    pch = strtok (NULL, " =\r\n");
                  }

                  if(token[0] && value[0])
                  {
                        /* parse arguments here */
                        if(strcmp(token, "LoggingMode") == 0)
                        {
                            daemon->mode = atoi(value);
                            snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE,"Runtime Option: %s=%d\n",token,daemon->mode);
                            dlt_log(LOG_INFO, str);
                        }
                        else
                        {
                            snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE,"Unknown option: %s=%s\n",token,value);
                            dlt_log(LOG_WARNING, str);
                        }
                    }
            }
            else
            {
                break;
            }
        }
        fclose (pFile);
    }
    else
    {
        snprintf(str,DLT_DAEMON_COMMON_TEXTBUFSIZE,"Cannot open configuration file: %s\n",filename);
        dlt_log(LOG_WARNING, str);
    }

    return 0;
}

int dlt_daemon_user_send_log_level(DltDaemon *daemon, DltDaemonContext *context, int verbose)
{
    DltUserHeader userheader;
    DltUserControlMsgLogLevel usercontext;
    DltReturnValue ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (context == NULL))
    {
        dlt_vlog(LOG_ERR, "NULL parameter in %s", __func__);
        return -1;
    }

    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_LEVEL) < DLT_RETURN_OK)
    {
        dlt_vlog(LOG_ERR, "Failed to set userheader in %s", __func__);
        return -1;
    }

    if(context->storage_log_level != DLT_LOG_DEFAULT)
    {
        usercontext.log_level = context->log_level > context->storage_log_level ? context->log_level:context->storage_log_level;
    }
    else /* Storage log level is not updated (is DEFAULT) then  no device is yet connected so ignore */
    {
        usercontext.log_level = ((context->log_level == DLT_LOG_DEFAULT)?daemon->default_log_level:context->log_level);
    }

    usercontext.trace_status = ((context->trace_status == DLT_TRACE_STATUS_DEFAULT)?daemon->default_trace_status:context->trace_status);

    usercontext.log_level_pos = context->log_level_pos;

    dlt_vlog(LOG_NOTICE, "Send log-level to context: %.4s:%.4s [%i -> %i] [%i -> %i]\n",
             context->apid,
             context->ctid,
             context->log_level,
             usercontext.log_level,
             context->trace_status,
             usercontext.trace_status);

    /* log to FIFO */
    errno = 0;
    ret = dlt_user_log_out2(context->user_handle,
                    &(userheader), sizeof(DltUserHeader),
                    &(usercontext), sizeof(DltUserControlMsgLogLevel));

    if (ret < DLT_RETURN_OK)
    {
        dlt_vlog(LOG_ERR, "Failed to send data to application in %s: %s",
                 __func__,
                 errno != 0 ? strerror(errno) : "Unknown error");

        if (errno == EPIPE)
        {
            /* Close connection */
            close(context->user_handle);
            context->user_handle = DLT_FD_INIT;
        }
    }

    return ((ret == DLT_RETURN_OK) ? DLT_RETURN_OK : DLT_RETURN_ERROR);
}

int dlt_daemon_user_send_log_state(DltDaemon *daemon, DltDaemonApplication *app, int verbose)
{
    DltUserHeader userheader;
    DltUserControlMsgLogState logstate;
    DltReturnValue ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (app == NULL))
        return -1;

    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_STATE) < DLT_RETURN_OK)
        return -1;

    logstate.log_state = daemon->connectionState;

    /* log to FIFO */
    ret = dlt_user_log_out2(app->user_handle,
                    &(userheader), sizeof(DltUserHeader),
                    &(logstate), sizeof(DltUserControlMsgLogState));

    if (ret < DLT_RETURN_OK)
    {
        if (errno==EPIPE)
        {
            /* Close connection */
            close(app->user_handle);
            app->user_handle=DLT_FD_INIT;
        }
    }

    return ((ret == DLT_RETURN_OK) ? DLT_RETURN_OK : DLT_RETURN_ERROR);
}

void dlt_daemon_control_reset_to_factory_default(DltDaemon *daemon,const char *filename, const char *filename1, int InitialContextLogLevel, int InitialContextTraceStatus, int InitialEnforceLlTsStatus, int verbose)
{
    FILE *fd;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon == NULL) || (filename == NULL) || (filename1 == NULL))
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    if ((filename[0] == '\0') || (filename1[0] == '\0'))
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Empty string\n");
        return;
    }

    /* Check for runtime cfg file and delete it, if available */
    fd=fopen(filename, "r");

    if (fd != NULL)
    {
        /* Close and delete file */
        fclose(fd);
        unlink(filename);
    }

    fd=fopen(filename1, "r");

    if (fd != NULL)
    {
        /* Close and delete file */
        fclose(fd);
        unlink(filename1);
    }

    daemon->default_log_level = InitialContextLogLevel;
    daemon->default_trace_status = InitialContextTraceStatus;
    daemon->force_ll_ts = InitialEnforceLlTsStatus;

    /* Reset all other things (log level, trace status, etc.
                               to default values             */

    /* Inform user libraries about changed default log level/trace status */
    dlt_daemon_user_send_default_update(daemon, verbose);
}

void dlt_daemon_user_send_default_update(DltDaemon *daemon, int verbose)
{
    int32_t count;
    DltDaemonContext *context;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    for (count=0; count<daemon->num_contexts; count ++)
    {
        context = &(daemon->contexts[count]);

        if (context != NULL)
        {
            if ((context->log_level == DLT_LOG_DEFAULT) ||
                    (context->trace_status == DLT_TRACE_STATUS_DEFAULT))
            {
                if (context->user_handle >= DLT_FD_MINIMUM)
                {
                    if (dlt_daemon_user_send_log_level(daemon, context, verbose)==-1)
                    {
                        return;
                    }
                }
            }
        }
    }
}

void dlt_daemon_user_send_all_update(DltDaemon *daemon, int8_t log_level, int verbose)
{
    int32_t count = 0;
    DltDaemonContext *context = NULL;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
    {
        return;
    }

    for (count = 0; count < daemon->num_contexts; count++)
    {
        context = &(daemon->contexts[count]);
        if (context)
        {
            if (context->user_handle >= DLT_FD_MINIMUM)
            {
                context->log_level = log_level;
                if (dlt_daemon_user_send_log_level(daemon, context, verbose) == -1)
                {
                    return;
                }
            }
        }
    }
}

void dlt_daemon_user_send_all_log_state(DltDaemon *daemon, int verbose)
{
    int32_t count;
    DltDaemonApplication *app;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon == NULL)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    for (count=0; count<daemon->num_applications; count ++)
    {
        app = &(daemon->applications[count]);

        if (app != NULL)
        {
            if (app->user_handle >= DLT_FD_MINIMUM)
            {
                if (dlt_daemon_user_send_log_state(daemon, app, verbose)==-1)
                {
                    return;
                }
            }
        }
    }
}

void dlt_daemon_change_state(DltDaemon *daemon, DltDaemonState newState)
{
    switch(newState)
    {
        case DLT_DAEMON_STATE_INIT:
            dlt_log(LOG_INFO,"Switched to init state.\n");
            daemon->state = DLT_DAEMON_STATE_INIT;
            break;
        case DLT_DAEMON_STATE_BUFFER:
            dlt_log(LOG_INFO,"Switched to buffer state for socket connections.\n");
            daemon->state = DLT_DAEMON_STATE_BUFFER;
            break;
        case DLT_DAEMON_STATE_BUFFER_FULL:
            dlt_log(LOG_INFO,"Switched to buffer full state.\n");
            daemon->state = DLT_DAEMON_STATE_BUFFER_FULL;
            break;
        case DLT_DAEMON_STATE_SEND_BUFFER:
            dlt_log(LOG_INFO,"Switched to send buffer state for socket connections.\n");
            daemon->state = DLT_DAEMON_STATE_SEND_BUFFER;
            break;
        case DLT_DAEMON_STATE_SEND_DIRECT:
            dlt_log(LOG_INFO,"Switched to send direct state.\n");
            daemon->state = DLT_DAEMON_STATE_SEND_DIRECT;
            break;
    }

}



