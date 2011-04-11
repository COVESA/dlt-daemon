/*
 * Dlt Daemon - Diagnostic Log and Trace
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
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

static char str[DLT_DAEMON_TEXTBUFSIZE];

sem_t dlt_daemon_mutex;

static int dlt_daemon_cmp_apid(const void *m1, const void *m2)
{
    DltDaemonApplication *mi1 = (DltDaemonApplication *) m1;
    DltDaemonApplication *mi2 = (DltDaemonApplication *) m2;

    return memcmp(mi1->apid, mi2->apid, DLT_ID_SIZE);
}

static int dlt_daemon_cmp_apid_ctid(const void *m1, const void *m2)
{

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

int dlt_daemon_init(DltDaemon *daemon,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    daemon->num_contexts = 0;
    daemon->contexts = 0;

    daemon->num_applications = 0;
    daemon->applications = 0;

    daemon->default_log_level = DLT_DAEMON_INITIAL_LOG_LEVEL ;
    daemon->default_trace_status = DLT_DAEMON_INITIAL_TRACE_STATUS ;

    daemon->message_buffer_overflow = DLT_MESSAGE_BUFFER_NO_OVERFLOW;

    daemon->runtime_context_cfg_loaded = 0;

    /* Check for runtime cfg, if it is loadable, load it! */
    if ((dlt_daemon_applications_load(daemon,DLT_RUNTIME_APPLICATION_CFG, verbose)==0) &&
            (dlt_daemon_contexts_load(daemon,DLT_RUNTIME_CONTEXT_CFG, verbose)==0))
    {
        daemon->runtime_context_cfg_loaded = 1;
    }

    daemon->sendserialheader = 0;
    daemon->timingpackets = 0;

    dlt_set_id(daemon->ecuid,"");

    /* initialize ring buffer for client connection */
    if (dlt_ringbuffer_init(&(daemon->client_ringbuffer), DLT_DAEMON_RINGBUFFER_SIZE)==-1)
    {
    	return -1;
    }

    return 0;
}

int dlt_daemon_free(DltDaemon *daemon,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    /* Free contexts */
    if (dlt_daemon_contexts_clear(daemon, verbose)==-1)
    {
		return -1;
    }

    /* Free applications */
    if (dlt_daemon_applications_clear(daemon, verbose)==-1)
	{
		return -1;
    }

    return 0;
}

int dlt_daemon_applications_clear(DltDaemon *daemon,int verbose)
{
    uint32_t i;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    for (i=0; i<daemon->num_applications; i++)
    {
        if (daemon->applications[i].application_description!=0)
        {
            free(daemon->applications[i].application_description);
            daemon->applications[i].application_description = 0;
        }
    }

    if (daemon->applications)
    {
        free(daemon->applications);
    }

    daemon->applications = 0;
    daemon->num_applications = 0;

    return 0;
}

DltDaemonApplication* dlt_daemon_application_add(DltDaemon *daemon,char *apid,pid_t pid,char *description, int verbose)
{
    DltDaemonApplication *application;
	DltDaemonApplication *old;
    int new_application;
    int dlt_user_handle;
	char filename[DLT_DAEMON_TEXTBUFSIZE];

    if ((daemon==0) || (apid==0) || (apid[0]=='\0'))
    {
        return (DltDaemonApplication*) 0;
    }

    if (daemon->applications == 0)
    {
        daemon->applications = (DltDaemonApplication*) malloc(sizeof(DltDaemonApplication)*DLT_DAEMON_APPL_ALLOC_SIZE);
        if (daemon->applications==0)
        {
        	return (DltDaemonApplication*) 0;
        }
    }

    new_application=0;

    /* Check if application [apid] is already available */
    application = dlt_daemon_application_find(daemon, apid, verbose);
    if (application==0)
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
				if (daemon->applications==0)
				{
					daemon->applications = old;
					return (DltDaemonApplication*) 0;
				}
                memcpy(daemon->applications,old,sizeof(DltDaemonApplication)*daemon->num_applications);
                free(old);
            }
        }

        application = &(daemon->applications[daemon->num_applications-1]);

        dlt_set_id(application->apid,apid);
        application->application_description = 0;
        application->num_contexts = 0;

        new_application = 1;
    }

    /* Store application description and pid of application */
    if (application->application_description)
    {
        free(application->application_description);
    }

    application->application_description=0;

    if (description)
    {
        application->application_description = malloc(strlen(description)+1);
        if (application->application_description)
        {
            strncpy(application->application_description,description,strlen(description)+1);
            application->application_description[strlen(description)]='\0';
        }
    }

    application->pid = pid;

    application->user_handle = -1;

    if (pid)
    {
        sprintf(filename,"%s/dlt%d",DLT_USER_DIR,application->pid);

        dlt_user_handle = open(filename, O_WRONLY|O_NONBLOCK);
        if (dlt_user_handle <0)
        {
            sprintf(str,"open() failed to %s, errno=%d (%s)!\n",filename,errno,strerror(errno)); /* errno 2: ENOENT - No such file or directory */
            dlt_log(LOG_ERR, str);
        } /* if */

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

    if ((daemon==0) || (application==0))
    {
        return -1;
    }

    if (daemon->num_applications>0)
    {
        /* Check if user handle is open; if yes, close it */
        if (application->user_handle!=-1)
        {
            close(application->user_handle);
            application->user_handle=-1;
        }

        /* Free description of application to be deleted */
        if (application->application_description)
        {
            free(application->application_description);
            application->application_description = 0;
        }

        pos = application-(daemon->applications);

        /* move all applications above pos to pos */
        memmove(&(daemon->applications[pos]),&(daemon->applications[pos+1]), sizeof(DltDaemonApplication)*((daemon->num_applications-1)-pos));

        /* Clear last application */
        memset(&(daemon->applications[daemon->num_applications-1]),0,sizeof(DltDaemonApplication));

        daemon->num_applications--;

    }

    return 0;
}

DltDaemonApplication* dlt_daemon_application_find(DltDaemon *daemon,char *apid,int verbose)
{
    DltDaemonApplication application;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (apid==0) || (apid[0]=='\0') || (daemon->num_applications==0))
    {
        return (DltDaemonApplication*) 0;
    }

    /* Check, if apid is smaller than smallest apid or greater than greatest apid */
    if ((memcmp(apid,daemon->applications[0].apid,DLT_ID_SIZE)<0) ||
            (memcmp(apid,daemon->applications[daemon->num_applications-1].apid,DLT_ID_SIZE)>0))
    {
        return (DltDaemonApplication*) 0;
    }

    dlt_set_id(application.apid,apid);
    return (DltDaemonApplication*)bsearch(&application,daemon->applications,daemon->num_applications,sizeof(DltDaemonApplication),dlt_daemon_cmp_apid);
}

int dlt_daemon_applications_load(DltDaemon *daemon,const char *filename, int verbose)
{
    FILE *fd;
    ID4 apid;
    char buf[DLT_DAEMON_TEXTBUFSIZE];
    char *ret;
    char *pb;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (filename==0) || (filename[0]=='\0'))
    {
        return -1;
    }

    fd=fopen(filename, "r");

    if (fd==0)
    {
        return -1;
    }

    while (!feof(fd))
    {
        /* Clear buf */
        memset(buf, 0, sizeof(buf));

        /* Get line */
        ret=fgets(buf,sizeof(buf),fd);

        if (strcmp(buf,"")!=0)
        {
            /* Split line */
            pb=strtok(buf,":");
            dlt_set_id(apid,pb);
            pb=strtok(NULL,":");
            /* pb contains now the description */

            /* pid is unknown at loading time */
            if (dlt_daemon_application_add(daemon,apid,0,pb,verbose)==0)
            {
            	fclose(fd);
            	return -1;
            }
        }
    }
    fclose(fd);

    return 0;
}

int dlt_daemon_applications_save(DltDaemon *daemon,const char *filename, int verbose)
{
    FILE *fd;
    uint32_t i;

    char apid[DLT_ID_SIZE+1]; /* DLT_ID_SIZE+1, because the 0-termination is required here */

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (filename==0) || (filename[0]=='\0'))
    {
        return -1;
    }

    memset(apid,0, sizeof(apid));

    if ((daemon->applications) && (daemon->num_applications>0))
    {
        fd=fopen(filename, "w");
        if (fd!=0)
        {
            for (i=0; i<daemon->num_applications; i++)
            {
                dlt_set_id(apid,daemon->applications[i].apid);

                if ((daemon->applications[i].application_description) &&
                        (daemon->applications[i].application_description!='\0'))
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

DltDaemonContext* dlt_daemon_context_add(DltDaemon *daemon,char *apid,char *ctid,int8_t log_level,int8_t trace_status,int log_level_pos, int user_handle,char *description,int verbose)
{
    DltDaemonApplication *application;
    DltDaemonContext *context;
    DltDaemonContext *old;
    int new_context=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (apid==0) || (apid[0]=='\0') || (ctid==0) || (ctid[0]=='\0'))
    {
        return (DltDaemonContext*) 0;
    }

    if ((log_level<DLT_LOG_DEFAULT) || (log_level>DLT_LOG_VERBOSE))
    {
        return (DltDaemonContext*) 0;
    }

    if ((trace_status<DLT_TRACE_STATUS_DEFAULT) || (trace_status>DLT_TRACE_STATUS_ON))
    {
        return (DltDaemonContext*) 0;
    }

    if (daemon->contexts == 0)
    {
        daemon->contexts = (DltDaemonContext*) malloc(sizeof(DltDaemonContext)*DLT_DAEMON_CONTEXT_ALLOC_SIZE);
        if (daemon->contexts==0)
        {
			return (DltDaemonContext*) 0;
        }
    }

    /* Check if application [apid] is available */
    application = dlt_daemon_application_find(daemon, apid, verbose);
    if (application==0)
    {
        return (DltDaemonContext*) 0;
    }

    /* Check if context [apid, ctid] is already available */
    context = dlt_daemon_context_find(daemon, apid, ctid, verbose);
    if (context==0)
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
				if (daemon->contexts==0)
				{
					daemon->contexts = old;
					return (DltDaemonContext*) 0;
				}
                memcpy(daemon->contexts,old,sizeof(DltDaemonContext)*daemon->num_contexts);
                free(old);
            }
        }

        context = &(daemon->contexts[daemon->num_contexts-1]);

        dlt_set_id(context->apid,apid);
        dlt_set_id(context->ctid,ctid);
        context->context_description = 0;

        application->num_contexts++;
        new_context =1;
    }

    /* Set context description */
    if (context->context_description)
    {
        free(context->context_description);
    }

    context->context_description=0;

    if (description)
    {
        context->context_description = malloc(strlen(description)+1);

        if (context->context_description)
        {
            strncpy(context->context_description,description,strlen(description)+1);
            context->context_description[strlen(description)]='\0';
        }
    }

    /* Store log level and trace status,
       if this is a new context, or
       if this is an old context and the runtime cfg was not loaded */

    if ((new_context==1) ||
            ((new_context==0) && (daemon->runtime_context_cfg_loaded==0)))
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

    if ((daemon==0) || (context==0))
    {
        return -1;
    }

    if (daemon->num_contexts>0)
    {
        application = dlt_daemon_application_find(daemon, context->apid, verbose);

        /* Free description of context to be deleted */
        if (context->context_description)
        {
            free(context->context_description);
            context->context_description = 0;
        }

        pos = context-(daemon->contexts);

        /* move all contexts above pos to pos */
        memmove(&(daemon->contexts[pos]),&(daemon->contexts[pos+1]), sizeof(DltDaemonContext)*((daemon->num_contexts-1)-pos));

        /* Clear last context */
        memset(&(daemon->contexts[daemon->num_contexts-1]),0,sizeof(DltDaemonContext));

        daemon->num_contexts--;

        /* Check if application [apid] is available */
        if (application)
        {
            application->num_contexts--;
        }
    }

    return 0;
}

DltDaemonContext* dlt_daemon_context_find(DltDaemon *daemon,char *apid,char *ctid,int verbose)
{
    DltDaemonContext context;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (apid==0) || (apid[0]=='\0') || (ctid==0) || (ctid[0]=='\0') || (daemon->num_contexts==0))
    {
        return (DltDaemonContext*) 0;
    }

    /* Check, if apid is smaller than smallest apid or greater than greatest apid */
    if ((memcmp(apid,daemon->contexts[0].apid,DLT_ID_SIZE)<0) ||
            (memcmp(apid,daemon->contexts[daemon->num_contexts-1].apid,DLT_ID_SIZE)>0))
    {
        return (DltDaemonContext*) 0;
    }

    dlt_set_id(context.apid,apid);
    dlt_set_id(context.ctid,ctid);

    return (DltDaemonContext*)bsearch(&context,daemon->contexts,daemon->num_contexts,sizeof(DltDaemonContext),dlt_daemon_cmp_apid_ctid);
}

int dlt_daemon_contexts_clear(DltDaemon *daemon,int verbose)
{
    int i;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return -1;
    }

    for (i=0; i<daemon->num_contexts; i++)
    {
        if (daemon->contexts[i].context_description!=0)
        {
            free(daemon->contexts[i].context_description);
            daemon->contexts[i].context_description = 0;
        }
    }

    if (daemon->contexts)
    {
        free(daemon->contexts);
    }

    daemon->contexts = 0;

    for (i=0; i<daemon->num_applications; i++)
    {
        daemon->applications[i].num_contexts = 0;
    }

    daemon->num_contexts = 0;

    return 0;
}

int dlt_daemon_contexts_load(DltDaemon *daemon,const char *filename, int verbose)
{
    FILE *fd;
    ID4 apid, ctid;
    char buf[DLT_DAEMON_TEXTBUFSIZE];
    char *ret;
    char *pb;
    int ll, ts;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (filename==0) ||( filename[0]=='\0'))
    {
        return -1;
    }

    fd=fopen(filename, "r");

    if (fd==0)
    {
        return -1;
    }

    while (!feof(fd))
    {
        /* Clear buf */
        memset(buf, 0, sizeof(buf));

        /* Get line */
        ret=fgets(buf,sizeof(buf),fd);

        if (strcmp(buf,"")!=0)
        {
            /* Split line */
            pb=strtok(buf,":");
            dlt_set_id(apid,pb);
            pb=strtok(NULL,":");
            dlt_set_id(ctid,pb);
            pb=strtok(NULL,":");
            sscanf(pb,"%d",&ll);
            pb=strtok(NULL,":");
            sscanf(pb,"%d",&ts);
            pb=strtok(NULL,":");
            /* pb contains now the description */

            /* log_level_pos, and user_handle are unknown at loading time */
            if (dlt_daemon_context_add(daemon,apid,ctid,(int8_t)ll,(int8_t)ts,0,0,pb,verbose)==0)
            {
				fclose(fd);
				return -1;
            }
        }
    }
    fclose(fd);

    return 0;
}

int dlt_daemon_contexts_save(DltDaemon *daemon,const char *filename, int verbose)
{
    FILE *fd;
    uint32_t i;

    char apid[DLT_ID_SIZE+1], ctid[DLT_ID_SIZE+1]; /* DLT_ID_SIZE+1, because the 0-termination is required here */

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (filename==0) ||( filename[0]=='\0'))
    {
        return -1;
    }

    memset(apid,0, sizeof(apid));
    memset(ctid,0, sizeof(ctid));

    if ((daemon->contexts) && (daemon->num_contexts>0))
    {
        fd=fopen(filename, "w");
        if (fd!=0)
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

int dlt_daemon_user_send_log_level(DltDaemon *daemon,DltDaemonContext *context,int verbose)
{
    DltUserHeader userheader;
    DltUserControlMsgLogLevel usercontext;
    DltReturnValue ret;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (context==0))
    {
        return -1;
    }

    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_LEVEL)==-1)
    {
    	return -1;
    }

    usercontext.log_level = ((context->log_level == DLT_LOG_DEFAULT)?daemon->default_log_level:context->log_level);
    usercontext.trace_status = ((context->trace_status == DLT_TRACE_STATUS_DEFAULT)?daemon->default_trace_status:context->trace_status);

    usercontext.log_level_pos = context->log_level_pos;

    /* log to FIFO */
    ret = dlt_user_log_out2(context->user_handle, &(userheader), sizeof(DltUserHeader),  &(usercontext), sizeof(DltUserControlMsgLogLevel));

    if (ret!=DLT_RETURN_OK)
    {
        if (errno==EPIPE)
        {
            /* Close connection */
            close(context->user_handle);
            context->user_handle=0;
        }
    }

    return ((ret==DLT_RETURN_OK)?0:-1);
}

int dlt_daemon_control_process_control(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    uint32_t id,id_tmp=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0))
    {
        return -1;
    }

    if (msg->datasize<sizeof(uint32_t))
    {
        return -1;
    }

    id_tmp = *((uint32_t*)(msg->databuffer));
    id=DLT_ENDIAN_GET_32(msg->standardheader->htyp ,id_tmp);

    if ((id > 0) && (id <= DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW))
    {
        /* Control message handling */
        switch (id)
        {
        case DLT_SERVICE_ID_SET_LOG_LEVEL:
        {
            dlt_daemon_control_set_log_level(sock, daemon, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_TRACE_STATUS:
        {
            dlt_daemon_control_set_trace_status(sock, daemon, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_LOG_INFO:
        {
            dlt_daemon_control_get_log_info(sock, daemon, msg, verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL:
        {
            dlt_daemon_control_get_default_log_level(sock, daemon,  verbose);
            break;
        }
        case DLT_SERVICE_ID_STORE_CONFIG:
        {
            if (dlt_daemon_applications_save(daemon, DLT_RUNTIME_APPLICATION_CFG, verbose)==0)
            {
				if (dlt_daemon_contexts_save(daemon, DLT_RUNTIME_CONTEXT_CFG, verbose)==0)
				{
					dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
				}
				else
				{
					/* Delete saved files */
					dlt_daemon_control_reset_to_factory_default(daemon, DLT_RUNTIME_APPLICATION_CFG, DLT_RUNTIME_CONTEXT_CFG, verbose);
					dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
				}
            }
            else
            {
            	dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
            }
            break;
        }
        case DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT:
        {
            dlt_daemon_control_reset_to_factory_default(daemon, DLT_RUNTIME_APPLICATION_CFG, DLT_RUNTIME_CONTEXT_CFG, verbose);
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_VERBOSE_MODE:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_MESSAGE_FILTERING:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_TIMING_PACKETS:
        {
            dlt_daemon_control_set_timing_packets(sock, daemon, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_LOCAL_TIME:
        {
            /* Send response with valid timestamp (TMSP) field */
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_ECU_ID:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_SESSION_ID:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_TIMESTAMP:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_USE_EXTENDED_HEADER:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL:
        {
            dlt_daemon_control_set_default_log_level(sock, daemon, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS:
        {
            dlt_daemon_control_set_default_trace_status(sock, daemon, msg,  verbose);
            break;
        }
        case DLT_SERVICE_ID_GET_SOFTWARE_VERSION:
        {
            dlt_daemon_control_get_software_version(sock, daemon,  verbose);
            break;
        }
        case DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW:
        {
            dlt_daemon_control_message_buffer_overflow(sock, daemon,  verbose);
            break;
        }
        default:
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
            break;
        }
        }
    }
    else
    {
        /* Injection handling */
        dlt_daemon_control_callsw_cinjection(sock, daemon, msg,  verbose);
    }

    return 0;
}

void dlt_daemon_control_callsw_cinjection(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];
    uint32_t id=0,id_tmp=0;
    uint8_t *ptr;
    DltDaemonContext *context;
	uint32_t data_length_inject=0,data_length_inject_tmp=0;

	int32_t datalength;

	DltUserHeader userheader;
	DltUserControlMsgInjection usercontext;
	uint8_t *userbuffer;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    datalength = msg->datasize;
    ptr = msg->databuffer;

    if (ptr==0)
    {
        return;
    }

    DLT_MSG_READ_VALUE(id_tmp,ptr,datalength,uint32_t); /* Get service id */
    id=DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

    if ((id>=DLT_DAEMON_INJECTION_MIN) && (id<=DLT_DAEMON_INJECTION_MAX))
    {
        /* This a a real SW-C injection call */
        data_length_inject=0;
        data_length_inject_tmp=0;

        DLT_MSG_READ_VALUE(data_length_inject_tmp,ptr,datalength,uint32_t); /* Get data length */
        data_length_inject=DLT_ENDIAN_GET_32(msg->standardheader->htyp, data_length_inject_tmp);

        /* Get context handle for apid, ctid (and seid) */
        /* Warning: seid is ignored in this implementation! */
        if (DLT_IS_HTYP_UEH(msg->standardheader->htyp))
        {
            dlt_set_id(apid, msg->extendedheader->apid);
            dlt_set_id(ctid, msg->extendedheader->ctid);
        }
        else
        {
            /* No extended header, and therefore no apid and ctid available */
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
            return;
        }

        /* At this point, apid and ctid is available */
        context=dlt_daemon_context_find(daemon, apid, ctid, verbose);

        if (context==0)
        {
            // dlt_log(LOG_INFO,"No context found!\n");
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
            return;
        }

        /* Send user message to handle, specified in context */
		if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_INJECTION)==-1)
		{
			dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
			return;
		}

		usercontext.log_level_pos = context->log_level_pos;

		userbuffer = malloc(data_length_inject);

		if (userbuffer==0)
		{
			dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
			return;
		}

		usercontext.data_length_inject = data_length_inject;
		usercontext.service_id = id;

		memcpy(userbuffer,ptr,data_length_inject);  /* Copy received injection to send buffer */

		/* write to FIFO */
		if (dlt_user_log_out3(context->user_handle, &(userheader), sizeof(DltUserHeader),
							  &(usercontext), sizeof(DltUserControlMsgInjection),
							  userbuffer, data_length_inject)!=DLT_RETURN_OK)
		{
			if (errno==EPIPE)
			{
				/* Close connection */
				close(context->user_handle);
				context->user_handle=0;
			}
			dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
		}
		else
		{
			dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
		}

		free(userbuffer);
		userbuffer=0;

    }
    else
    {
        /* Invalid ID */
        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_NOT_SUPPORTED,  verbose);
    }
}

void dlt_daemon_control_set_log_level(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];
    DltServiceSetLogLevel *req;
    DltDaemonContext *context;
    int32_t id=DLT_SERVICE_ID_SET_LOG_LEVEL;

	int8_t old_log_level;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetLogLevel*) (msg->databuffer);

    dlt_set_id(apid, req->apid);
    dlt_set_id(ctid, req->ctid);

    context=dlt_daemon_context_find(daemon, apid, ctid, verbose);

    /* Set log level */
    if (context!=0)
    {
        old_log_level = context->log_level;
        context->log_level = req->log_level; /* No endianess conversion necessary*/

        if ((context->user_handle!=0) &&
                (dlt_daemon_user_send_log_level(daemon, context, verbose)==0))
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
        }
        else
        {
            //dlt_log(LOG_ERR, "Log level could not be sent!\n");
            context->log_level = old_log_level;
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        }
    }
    else
    {
        //dlt_log(LOG_ERR, "Context not found!\n");
        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_trace_status(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];
    DltServiceSetLogLevel *req;             /* request uses same struct as set log level */
    DltDaemonContext *context;
    int32_t id=DLT_SERVICE_ID_SET_TRACE_STATUS;

	int8_t old_trace_status;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetLogLevel*) (msg->databuffer);

    dlt_set_id(apid, req->apid);
    dlt_set_id(ctid, req->ctid);

    context=dlt_daemon_context_find(daemon, apid, ctid, verbose);

    /* Set log level */
    if (context!=0)
    {
        old_trace_status = context->trace_status;
        context->trace_status = req->log_level;   /* No endianess conversion necessary */

        if ((context->user_handle!=0) &&
                (dlt_daemon_user_send_log_level(daemon, context, verbose)==0))
        {
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
        }
        else
        {
            //dlt_log(LOG_ERR, "Trace Status could not be sent!\n");
            context->trace_status = old_trace_status;
            dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        }
    }
    else
    {
        //dlt_log(LOG_ERR, "Context not found!\n");
        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_default_log_level(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetDefaultLogLevel *req;
    int32_t id=DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetDefaultLogLevel*) (msg->databuffer);

    /* No endianess conversion necessary */
    if ((req->log_level>=0) &&
            (req->log_level<=DLT_LOG_VERBOSE))
    {
        daemon->default_log_level = req->log_level; /* No endianess conversion necessary */

        /* Send Update to all contexts using the default log level */
        dlt_daemon_user_send_default_update(daemon, verbose);

        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_default_trace_status(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    /* Payload of request message */
    DltServiceSetDefaultLogLevel *req;
    int32_t id=DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetDefaultLogLevel*) (msg->databuffer);

    /* No endianess conversion necessary */
    if ((req->log_level==DLT_TRACE_STATUS_OFF) ||
            (req->log_level==DLT_TRACE_STATUS_ON))
    {
        daemon->default_trace_status = req->log_level; /* No endianess conversion necessary*/

        /* Send Update to all contexts using the default trace status */
        dlt_daemon_user_send_default_update(daemon, verbose);

        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_set_timing_packets(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    DltServiceSetVerboseMode *req;  /* request uses same struct as set verbose mode */
    int32_t id=DLT_SERVICE_ID_SET_TIMING_PACKETS;

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    req = (DltServiceSetVerboseMode*) (msg->databuffer);
    if ((req->new_status==0) || (req->new_status==1))
    {
        daemon->timingpackets = req->new_status;

        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_OK,  verbose);
    }
    else
    {
        dlt_daemon_control_service_response(sock, daemon, id, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    }
}

void dlt_daemon_control_get_software_version(int sock, DltDaemon *daemon, int verbose)
{
	char version[DLT_DAEMON_TEXTBUFSIZE];
    DltMessage msg;
    uint32_t len;
	DltServiceGetSoftwareVersionResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0)==-1)
    {
    	dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_SOFTWARE_VERSION, DLT_SERVICE_RESPONSE_ERROR,  verbose);
		return;
    }

    /* prepare payload of data */
    dlt_get_version(version);
    len = strlen(version);

    msg.datasize = sizeof(DltServiceGetSoftwareVersionResponse) + len;
    if (msg.databuffer)
    {
        free(msg.databuffer);
    }
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    if (msg.databuffer==0)
    {
        dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_SOFTWARE_VERSION, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    resp = (DltServiceGetSoftwareVersionResponse*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->length = len;
    memcpy(msg.databuffer+sizeof(DltServiceGetSoftwareVersionResponse),version,len);

    /* send message */
    dlt_daemon_control_send_control_message(sock, daemon, &msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

void dlt_daemon_control_get_default_log_level(int sock, DltDaemon *daemon, int verbose)
{
    DltMessage msg;
	DltServiceGetDefaultLogLevelResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0)==-1)
    {
    	dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    msg.datasize = sizeof(DltServiceGetDefaultLogLevelResponse);
    if (msg.databuffer)
    {
        free(msg.databuffer);
    }
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    if (msg.databuffer==0)
    {
        dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    resp = (DltServiceGetDefaultLogLevelResponse*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->log_level = daemon->default_log_level;

    /* send message */
    dlt_daemon_control_send_control_message(sock,daemon,&msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

void dlt_daemon_control_get_log_info(int sock, DltDaemon *daemon, DltMessage *msg, int verbose)
{
    DltServiceGetLogInfoRequest *req;
    DltMessage resp;
    DltDaemonContext *context=0;
    DltDaemonApplication *application=0;

    int num_applications=0, num_contexts=0;
    uint16_t count_app_ids=0, count_con_ids=0;

#if (DLT_DEBUG_GETLOGINFO==1)
    char buf[255];
#endif

    int32_t i,j,offset=0;
    char *apid=0;
    int8_t ll,ts;
    uint16_t len;
    int8_t value;
    int32_t sizecont=0;
    int offset_base;

    uint32_t sid;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0))
    {
        return;
    }

    /* prepare pointer to message request */
    req = (DltServiceGetLogInfoRequest*) (msg->databuffer);

    /* initialise new message */
    if (dlt_message_init(&resp,0)==-1)
    {
		dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_LOG_INFO, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    /* check request */
    if ((req->options < 3 ) || (req->options>7))
    {
        dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_LOG_INFO, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }

    if (req->apid[0]!='\0')
    {
        application = dlt_daemon_application_find(daemon, req->apid, verbose);
        if (application)
        {
            num_applications = 1;
            if (req->ctid[0]!='\0')
            {
                context = dlt_daemon_context_find(daemon, req->apid, req->ctid, verbose);

                num_contexts = ((context)?1:0);
            }
            else
            {
                num_contexts = application->num_contexts;
            }
        }
        else
        {
            num_applications = 0;
            num_contexts = 0;
        }
    }
    else
    {
        /* Request all applications and contexts */
        num_applications = daemon->num_applications;
        num_contexts = daemon->num_contexts;
    }

    /* prepare payload of data */

    /* Calculate maximum size for a response */
    resp.datasize = sizeof(uint32_t) /* SID */ + sizeof(int8_t) /* status*/ + sizeof(ID4) /* DLT_DAEMON_REMO_STRING */;

    sizecont = sizeof(uint32_t) /* context_id */;

    /* Add additional size for response of Mode 4, 6, 7 */
    if ((req->options==4) || (req->options==6) || (req->options==7))
    {
        sizecont += sizeof(int8_t); /* log level */
    }

    /* Add additional size for response of Mode 5, 6, 7 */
    if ((req->options==5) || (req->options==6) || (req->options==7))
    {
        sizecont+= sizeof(int8_t); /* trace status */
    }

    resp.datasize+= (num_applications * (sizeof(uint32_t) /* app_id */  + sizeof(uint16_t) /* count_con_ids */)) +
                    (num_contexts * sizecont);

    resp.datasize+= sizeof(uint16_t) /* count_app_ids */;

    /* Add additional size for response of Mode 7 */
    if (req->options==7)
    {
        if (req->apid[0]!='\0')
        {
            if (req->ctid[0]!='\0')
            {
                /* One application, one context */
                // context = dlt_daemon_context_find(daemon, req->apid, req->ctid, verbose);
                if (context)
                {
                    resp.datasize+=sizeof(uint16_t) /* len_context_description */;
                    if (context->context_description!=0)
                    {
                        resp.datasize+=strlen(context->context_description); /* context_description */
                    }
                }
            }
            else
            {
                /* One application, all contexts */
                if ((daemon->applications) && (application))
                {
                    /* Calculate start offset within contexts[] */
                    offset_base=0;
                    for (i=0; i<(application-(daemon->applications)); i++)
                    {
                        offset_base+=daemon->applications[i].num_contexts;
                    }

                    /* Iterate over all contexts belonging to this application */
                    for (j=0;j<application->num_contexts;j++)
                    {

                        context = &(daemon->contexts[offset_base+j]);
                        if (context)
                        {
                            resp.datasize+=sizeof(uint16_t) /* len_context_description */;
                            if (context->context_description!=0)
                            {
                                resp.datasize+=strlen(context->context_description); /* context_description */
                            }
                        }
                    }
                }
            }

            /* Space for application description */
            if (application)
            {
                resp.datasize+=sizeof(uint16_t) /* len_app_description */;
                if (application->application_description!=0)
                {
                    resp.datasize+=strlen(application->application_description); /* app_description */
                }
            }
        }
        else
        {
            /* All applications, all contexts */
            for (i=0;i<daemon->num_contexts;i++)
            {
                resp.datasize+=sizeof(uint16_t) /* len_context_description */;
                if (daemon->contexts[i].context_description!=0)
                {
                    resp.datasize+=strlen(daemon->contexts[i].context_description); /* context_description */
                }
            }

            for (i=0;i<daemon->num_applications;i++)
            {
                resp.datasize+=sizeof(uint16_t) /* len_app_description */;
                if (daemon->applications[i].application_description!=0)
                {
                    resp.datasize+=strlen(daemon->applications[i].application_description); /* app_description */
                }
            }
        }
    }

    if (verbose)
    {
        sprintf(str,"Allocate %d bytes for response msg databuffer\n", resp.datasize);
        dlt_log(LOG_INFO, str);
    }

    /* Allocate buffer for response message */
    resp.databuffer = (uint8_t *) malloc(resp.datasize);
    if (resp.databuffer==0)
    {
        dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_GET_LOG_INFO, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        return;
    }
    memset(resp.databuffer,0,resp.datasize);
    /* Preparation finished */

    /* Prepare response */
    sid = DLT_SERVICE_ID_GET_LOG_INFO;
    memcpy(resp.databuffer,&sid,sizeof(uint32_t));
    offset+=sizeof(uint32_t);

    value = (((num_applications!=0)&&(num_contexts!=0))?req->options:8); /* 8 = no matching context found */

    memcpy(resp.databuffer+offset,&value,sizeof(int8_t));
    offset+=sizeof(int8_t);

    count_app_ids = num_applications;

    if (count_app_ids!=0)
    {
        memcpy(resp.databuffer+offset,&count_app_ids,sizeof(uint16_t));
        offset+=sizeof(uint16_t);

#if (DLT_DEBUG_GETLOGINFO==1)
        sprintf(str,"#apid: %d \n", count_app_ids);
        dlt_log(LOG_DEBUG, str);
#endif

        for (i=0;i<count_app_ids;i++)
        {
            if (req->apid[0]!='\0')
            {
                apid = req->apid;
            }
            else
            {
                if (daemon->applications)
                {
                    apid = daemon->applications[i].apid;
                }
                else
                {
                    /* This should never occur! */
                    apid=0;
                }
            }

            application = dlt_daemon_application_find(daemon, apid, verbose);

            if (application)
            {
                /* Calculate start offset within contexts[] */
                offset_base=0;
                for (j=0; j<(application-(daemon->applications)); j++)
                {
                    offset_base+=daemon->applications[j].num_contexts;
                }

                dlt_set_id((char*)(resp.databuffer+offset),apid);
                offset+=sizeof(ID4);

#if (DLT_DEBUG_GETLOGINFO==1)
                dlt_print_id(buf, apid);
                sprintf(str,"apid: %s\n",buf);
                dlt_log(LOG_DEBUG, str);
#endif

                if (req->apid[0]!='\0')
                {
                    count_con_ids = num_contexts;
                }
                else
                {
                    count_con_ids = application->num_contexts;
                }

                memcpy(resp.databuffer+offset,&count_con_ids,sizeof(uint16_t));
                offset+=sizeof(uint16_t);

#if (DLT_DEBUG_GETLOGINFO==1)
                sprintf(str,"#ctid: %d \n", count_con_ids);
                dlt_log(LOG_DEBUG, str);
#endif

                for (j=0;j<count_con_ids;j++)
                {
#if (DLT_DEBUG_GETLOGINFO==1)
                    sprintf(str,"j: %d \n",j);
                    dlt_log(LOG_DEBUG, str);
#endif
                    if (!((count_con_ids==1) && (req->apid[0]!='\0') && (req->ctid[0]!='\0')))
                    {
                        context = &(daemon->contexts[offset_base+j]);
                    }
                    /* else: context was already searched and found
                             (one application (found) with one context (found))*/

                    if ((context) &&
                            ((req->ctid[0]=='\0') ||
                             ((req->ctid[0]!='\0') && (memcmp(context->ctid,req->ctid,DLT_ID_SIZE)==0)))
                       )
                    {
                        dlt_set_id((char*)(resp.databuffer+offset),context->ctid);
                        offset+=sizeof(ID4);

#if (DLT_DEBUG_GETLOGINFO==1)
                        dlt_print_id(buf, context->ctid);
                        sprintf(str,"ctid: %s \n",buf);
                        dlt_log(LOG_DEBUG, str);
#endif

                        /* Mode 4, 6, 7 */
                        if ((req->options==4) || (req->options==6) || (req->options==7))
                        {
                            ll=context->log_level;
                            memcpy(resp.databuffer+offset,&ll,sizeof(int8_t));
                            offset+=sizeof(int8_t);
                        }

                        /* Mode 5, 6, 7 */
                        if ((req->options==5) || (req->options==6) || (req->options==7))
                        {
                            ts=context->trace_status;
                            memcpy(resp.databuffer+offset,&ts,sizeof(int8_t));
                            offset+=sizeof(int8_t);
                        }

                        /* Mode 7 */
                        if (req->options==7)
                        {
                            if (context->context_description)
                            {
                                len = strlen(context->context_description);
                                memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                                offset+=sizeof(uint16_t);
                                memcpy(resp.databuffer+offset,context->context_description,strlen(context->context_description));
                                offset+=strlen(context->context_description);
                            }
                            else
                            {
                                len = 0;
                                memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                                offset+=sizeof(uint16_t);
                            }
                        }

#if (DLT_DEBUG_GETLOGINFO==1)
                        sprintf(str,"ll=%d ts=%d \n",(int32_t)ll,(int32_t)ts);
                        dlt_log(LOG_DEBUG, str);
#endif
                    }

#if (DLT_DEBUG_GETLOGINFO==1)
                    dlt_log(LOG_DEBUG,"\n");
#endif
                }

                /* Mode 7 */
                if (req->options==7)
                {
                    if (application->application_description)
                    {
                        len = strlen(application->application_description);
                        memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                        offset+=sizeof(uint16_t);
                        memcpy(resp.databuffer+offset,application->application_description,strlen(application->application_description));
                        offset+=strlen(application->application_description);
                    }
                    else
                    {
                        len = 0;
                        memcpy(resp.databuffer+offset,&len,sizeof(uint16_t));
                        offset+=sizeof(uint16_t);
                    }
                }
            } /* if (application) */
        } /* for (i=0;i<count_app_ids;i++) */
    } /* if (count_app_ids!=0) */

    dlt_set_id((char*)(resp.databuffer+offset),DLT_DAEMON_REMO_STRING);

    /* send message */
    dlt_daemon_control_send_control_message(sock,daemon,&resp,"","",  verbose);

    /* free message */
    dlt_message_free(&resp,0);
}

void dlt_daemon_control_message_buffer_overflow(int sock, DltDaemon *daemon, int verbose)
{
    DltMessage msg;
	DltServiceMessageBufferOverflowResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0)==-1)
    {
    	dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW, DLT_SERVICE_RESPONSE_ERROR,  verbose);
    	return;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceMessageBufferOverflowResponse);
    if (msg.databuffer)
    {
        free(msg.databuffer);
    }
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    if (msg.databuffer==0)
    {
        if (sock!=DLT_DAEMON_STORE_TO_BUFFER)
        {
            dlt_daemon_control_service_response(sock, daemon, DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW, DLT_SERVICE_RESPONSE_ERROR,  verbose);
        }
        return;
    }

    resp = (DltServiceMessageBufferOverflowResponse*) msg.databuffer;
    resp->service_id = DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW;
    resp->status = DLT_SERVICE_RESPONSE_OK;
    resp->overflow = daemon->message_buffer_overflow;

    /* send message */
    dlt_daemon_control_send_control_message(sock,daemon,&msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

void dlt_daemon_control_service_response( int sock, DltDaemon *daemon, uint32_t service_id, int8_t status , int verbose)
{
    DltMessage msg;
    DltServiceResponse *resp;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0)==-1)
    {
		return;
    }

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceResponse);
    if (msg.databuffer)
    {
        free(msg.databuffer);
    }
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    if (msg.databuffer==0)
    {
        return;
    }

    resp = (DltServiceResponse*) msg.databuffer;
    resp->service_id = service_id;
    resp->status = status;

    /* send message */
    dlt_daemon_control_send_control_message(sock,daemon,&msg,"","",  verbose);

    /* free message */
    dlt_message_free(&msg,0);
}

void dlt_daemon_control_send_control_message( int sock, DltDaemon *daemon, DltMessage *msg, char* appid, char* ctid, int verbose)
{
    ssize_t ret;
    int32_t len;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (msg==0) || (appid==0) || (ctid==0))
    {
        return;
    }

    /* prepare storage header */
    msg->storageheader = (DltStorageHeader*)msg->headerbuffer;

    if (dlt_set_storageheader(msg->storageheader,daemon->ecuid)==-1)
    {
		return;
    }

    /* prepare standard header */
    msg->standardheader = (DltStandardHeader*)(msg->headerbuffer + sizeof(DltStorageHeader));
    msg->standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg->standardheader->htyp = ( msg->standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg->standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg->headerextra.ecu,daemon->ecuid);

    //msg->headerextra.seid = 0;

    msg->headerextra.tmsp = dlt_uptime();

    dlt_message_set_extraparameters(msg, verbose);

    /* prepare extended header */
    msg->extendedheader = (DltExtendedHeader*)(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp));
    msg->extendedheader->msin = DLT_MSIN_CONTROL_RESPONSE;

    msg->extendedheader->noar = 1; /* number of arguments */
    if (strcmp(appid,"")==0)
    {
        dlt_set_id(msg->extendedheader->apid,DLT_DAEMON_CTRL_APID);       /* application id */
    }
    else
    {
        dlt_set_id(msg->extendedheader->apid, appid);
    }
    if (strcmp(ctid,"")==0)
    {
        dlt_set_id(msg->extendedheader->ctid,DLT_DAEMON_CTRL_CTID);       /* context id */
    }
    else
    {
        dlt_set_id(msg->extendedheader->ctid, ctid);
    }

    /* prepare length information */
    msg->headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp);

    len=msg->headersize - sizeof(DltStorageHeader) + msg->datasize;
    if (len>UINT16_MAX)
    {
        dlt_log(LOG_CRIT,"Huge control message discarded!\n");
        return;
    }

    msg->standardheader->len = DLT_HTOBE_16(((uint16_t)len));

    if (sock!=DLT_DAEMON_STORE_TO_BUFFER)
    {
        /* Send message */
        if (isatty(sock))
        {
            DLT_DAEMON_SEM_LOCK();

            /* Optional: Send serial header, if requested */
            if (daemon->sendserialheader)
            {
                ret=write(sock,dltSerialHeader,sizeof(dltSerialHeader));
            }

            /* Send data */
            ret=write(sock, msg->headerbuffer+sizeof(DltStorageHeader),msg->headersize-sizeof(DltStorageHeader));
            ret=write(sock, msg->databuffer,msg->datasize);

            DLT_DAEMON_SEM_FREE();
        }
        else
        {
            DLT_DAEMON_SEM_LOCK();

            /* Optional: Send serial header, if requested */
            if (daemon->sendserialheader)
            {
                send(sock, dltSerialHeader,sizeof(dltSerialHeader),0);
            }

            /* Send data */
            send(sock, msg->headerbuffer+sizeof(DltStorageHeader),msg->headersize-sizeof(DltStorageHeader),0);
            send(sock, msg->databuffer,msg->datasize,0);

            DLT_DAEMON_SEM_FREE();
        }
    }
    else
    {
        /* Store message in history buffer */
        if (dlt_ringbuffer_put3(&(daemon->client_ringbuffer),
                            msg->headerbuffer+sizeof(DltStorageHeader),msg->headersize-sizeof(DltStorageHeader),
                            msg->databuffer,msg->datasize,
                            0, 0
                           )<0)
		{
			dlt_log(LOG_ERR,"Storage of message in history buffer failed! Message discarded.\n");
			return;
		}
    }
}

void dlt_daemon_control_reset_to_factory_default(DltDaemon *daemon,const char *filename, const char *filename1, int verbose)
{
    FILE *fd;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((daemon==0) || (filename==0) || (filename1==0) || (filename[0]=='\0') || (filename1[0]=='\0'))
    {
        return;
    }

    /* Check for runtime cfg file and delete it, if available */
    fd=fopen(filename, "r");

    if (fd!=0)
    {
        /* Close and delete file */
        fclose(fd);
        unlink(filename);
    }

    fd=fopen(filename1, "r");

    if (fd!=0)
    {
        /* Close and delete file */
        fclose(fd);
        unlink(filename1);
    }

    daemon->default_log_level = DLT_DAEMON_INITIAL_LOG_LEVEL ;
    daemon->default_trace_status = DLT_DAEMON_INITIAL_TRACE_STATUS ;

    daemon->message_buffer_overflow = DLT_MESSAGE_BUFFER_NO_OVERFLOW;

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

    if (daemon==0)
    {
        return;
    }

    for (count=0;count<daemon->num_contexts; count ++)
    {
        context = &(daemon->contexts[count]);

        if (context)
        {
            if ((context->log_level == DLT_LOG_DEFAULT) ||
                    (context->trace_status == DLT_TRACE_STATUS_DEFAULT))
            {
                if (context->user_handle!=0)
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

void dlt_daemon_control_message_time(int sock, DltDaemon *daemon, int verbose)
{
    DltMessage msg;
    ssize_t ret;
    int32_t len;

    PRINT_FUNCTION_VERBOSE(verbose);

    if (daemon==0)
    {
        return;
    }

    if (sock==DLT_DAEMON_STORE_TO_BUFFER)
    {
        return;
    }

    /* initialise new message */
    if (dlt_message_init(&msg,0)==-1)
    {
        return;
    }

    /* prepare payload of data */
    msg.datasize = 0;
    if (msg.databuffer)
    {
        free(msg.databuffer);
    }
    msg.databuffer = 0;

    /* send message */

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader*)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader,daemon->ecuid);

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = ( msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu,daemon->ecuid);
    msg.headerextra.tmsp = dlt_uptime();

    dlt_message_set_extraparameters(&msg, verbose);

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp));
    msg.extendedheader->msin = DLT_MSIN_CONTROL_TIME;

    msg.extendedheader->noar = 0;                  /* number of arguments */
    dlt_set_id(msg.extendedheader->apid,"");       /* application id */
    dlt_set_id(msg.extendedheader->ctid,"");       /* context id */

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);

    len=msg.headersize - sizeof(DltStorageHeader) + msg.datasize;
    if (len>UINT16_MAX)
    {
        dlt_log(LOG_CRIT,"Huge control message discarded!\n");

        /* free message */
        dlt_message_free(&msg,0);

        return;
    }

    msg.standardheader->len = DLT_HTOBE_16(((uint16_t)len));

    /* Send message */
    if (isatty(sock))
    {
        DLT_DAEMON_SEM_LOCK();

        /* Optional: Send serial header, if requested */
        if (daemon->sendserialheader)
        {
            ret=write(sock,dltSerialHeader,sizeof(dltSerialHeader));
        }

        /* Send data */
        ret=write(sock, msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        ret=write(sock, msg.databuffer,msg.datasize);

        DLT_DAEMON_SEM_FREE();
    }
    else
    {
        DLT_DAEMON_SEM_LOCK();

        /* Optional: Send serial header, if requested */
        if (daemon->sendserialheader)
        {
            send(sock, dltSerialHeader,sizeof(dltSerialHeader),0);
        }

        /* Send data */
        send(sock, msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader),0);
        send(sock, msg.databuffer,msg.datasize,0);

        DLT_DAEMON_SEM_FREE();
    }

    /* free message */
    dlt_message_free(&msg,0);
}

