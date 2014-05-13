/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt_daemon_common.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_daemon_common.h                                           **
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
 aw          15.02.2010   initial
 */

#ifndef DLT_DAEMON_COMMON_H
#define DLT_DAEMON_COMMON_H

/**
  \defgroup daemonapi DLT Daemon API
  \addtogroup daemonapi
  \{
*/

#include <limits.h>
#include <semaphore.h>
#include "dlt_common.h"
#include "dlt_user.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DLT_DAEMON_RINGBUFFER_MIN_SIZE    500000 /**< Ring buffer size for storing log messages while no client is connected */
#define DLT_DAEMON_RINGBUFFER_MAX_SIZE  10000000 /**< Ring buffer size for storing log messages while no client is connected */
#define DLT_DAEMON_RINGBUFFER_STEP_SIZE   500000 /**< Ring buffer size for storing log messages while no client is connected */

#define DLT_DAEMON_SEND_TO_ALL     -3   /**< Constant value to identify the command "send to all" */
#define DLT_DAEMON_SEND_FORCE      -4   /**< Constant value to identify the command "send force to all" */

/* Use a semaphore or mutex from your OS to prevent concurrent access to the DLT buffer. */

#define DLT_DAEMON_SEM_LOCK() { sem_wait(&dlt_daemon_mutex); }
#define DLT_DAEMON_SEM_FREE() { sem_post(&dlt_daemon_mutex); }
extern sem_t dlt_daemon_mutex;

/**
 * Definitions of DLT daemon logging states
 */
typedef enum
{
	DLT_DAEMON_STATE_INIT =    		  	 0,  /**< Initial state */
	DLT_DAEMON_STATE_BUFFER =            1,  /**< logging is buffered until external logger is connected or internal logging is activated */
	DLT_DAEMON_STATE_BUFFER_FULL =       2,  /**< then internal buffer is full, wait for connect from client */
	DLT_DAEMON_STATE_SEND_BUFFER =       3,  /**< external logger is connected, but buffer is still not empty or external logger queue is full */
	DLT_DAEMON_STATE_SEND_DIRECT =       4   /**< External logger is connected or internal logging is active, and buffer is empty */
} DltDaemonState;

/**
 * The parameters of a daemon application.
 */
typedef struct
{
	char  apid[DLT_ID_SIZE];                  /**< application id */
	pid_t pid;                   /**< process id of user application */
	int user_handle;    /**< connection handle for connection to user application */
	char *application_description; /**< context description */
	int num_contexts; /**< number of contexts for this application */
} DltDaemonApplication;

/**
 * The parameters of a daemon context.
 */
typedef struct
{
	char apid[DLT_ID_SIZE];               /**< application id */
	char ctid[DLT_ID_SIZE];   	        /**< context id */
	int8_t log_level;		/**< the current log level of the context */
	int8_t trace_status;	/**< the current trace status of the context */
	int log_level_pos;  /**< offset of context in context field on user application */
	int user_handle;    /**< connection handle for connection to user application */
	char *context_description; /**< context description */
} DltDaemonContext;

/**
 * The parameters of a daemon.
 */
typedef struct
{
	int num_contexts;               /**< Total number of all contexts in all applications */
	DltDaemonContext *contexts;         /**< Pointer to contexts */
	int num_applications;			/**< Number of available application */
	DltDaemonApplication *applications; /**< Pointer to applications */
	int8_t default_log_level;          /**< Default log level (of daemon) */
	int8_t default_trace_status;       /**< Default trace status (of daemon) */
	unsigned int overflow_counter;   /**< counts the number of lost messages. */
	int runtime_context_cfg_loaded;         /**< Set to one, if runtime context configuration has been loaded, zero otherwise */
	char ecuid[DLT_ID_SIZE];       /**< ECU ID of daemon */
	int sendserialheader;          /**< 1: send serial header; 0 don't send serial header */
	int timingpackets;              /**< 1: send continous timing packets; 0 don't send continous timing packets */
	DltBuffer client_ringbuffer; /**< Ring-buffer for storing received logs while no client connection is available */
    char runtime_application_cfg[PATH_MAX + 1]; /**< Path and filename of persistent application configuration. Set to path max, as it specifies a full path*/
    char runtime_context_cfg[PATH_MAX + 1]; /**< Path and filename of persistent context configuration */
    char runtime_configuration[PATH_MAX + 1]; /**< Path and filename of persistent configuration */
    DltUserLogMode mode;	/**< Mode used for tracing: off, external, internal, both */
    char connectionState;				/**< state for tracing: 0 = no client connected, 1 = client connected */
    char *ECUVersionString; /**< Version string to send to client. Loaded from a file at startup. May be null. */
    DltDaemonState state;   /**< the current logging state of dlt daemon. */
} DltDaemon;

/**
 * Initialise the dlt daemon structure
 * This function must be called before using further dlt daemon structure
 * @param daemon pointer to dlt daemon structure
 * @param RingbufferMinSize ringbuffer size
 * @param RingbufferMaxSize ringbuffer size
 * @param RingbufferStepSize ringbuffer size
 * @param runtime_directory Directory of persistent configuration
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_init(DltDaemon *daemon,unsigned long RingbufferMinSize,unsigned long RingbufferMaxSize,unsigned long RingbufferStepSize,const char *runtime_directory,int verbose);
/**
 * De-Initialise the dlt daemon structure
 * @param daemon pointer to dlt daemon structure
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_free(DltDaemon *daemon,int verbose);

/**
 * Add (new) application to internal application management
 * @param daemon pointer to dlt daemon structure
 * @param apid pointer to application id
 * @param pid process id of user application
 * @param description description of application
 * @param verbose if set to true verbose information is printed out.
 * @return Pointer to added context, null pointer on error
 */
DltDaemonApplication* dlt_daemon_application_add(DltDaemon *daemon,char *apid,pid_t pid,char *description, int verbose);
/**
 * Delete application from internal application management
 * @param daemon pointer to dlt daemon structure
 * @param application pointer to application to be deleted
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_application_del(DltDaemon *daemon, DltDaemonApplication *application, int verbose);
/**
 * Find application with specific application id
 * @param daemon pointer to dlt daemon structure
 * @param apid pointer to application id
 * @param verbose if set to true verbose information is printed out.
 * @return Pointer to application, null pointer on error or not found
 */
DltDaemonApplication* dlt_daemon_application_find(DltDaemon *daemon,char *apid,int verbose);
/**
 * Load applications from file to internal context management
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file to be used for loading
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_applications_load(DltDaemon *daemon,const char *filename, int verbose);
/**
 * Save applications from internal context management to file
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file to be used for saving
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_applications_save(DltDaemon *daemon,const char *filename, int verbose);
/**
 * Invalidate all applications fd, if fd is reused
 * @param daemon pointer to dlt daemon structure
 * @param fd file descriptor
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_applications_invalidate_fd(DltDaemon *daemon,int fd,int verbose);
/**
 * Clear all applications in internal application management
 * @param daemon pointer to dlt daemon structure
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_applications_clear(DltDaemon *daemon,int verbose);

/**
 * Add (new) context to internal context management
 * @param daemon pointer to dlt daemon structure
 * @param apid pointer to application id
 * @param ctid pointer to context id
 * @param log_level log level of context
 * @param trace_status trace status of context
 * @param log_level_pos offset of context in context field on user application
 * @param user_handle connection handle for connection to user application
 * @param description description of context
 * @param verbose if set to true verbose information is printed out.
 * @return Pointer to added context, null pointer on error
 */
DltDaemonContext* dlt_daemon_context_add(DltDaemon *daemon,char *apid,char *ctid,int8_t log_level,int8_t trace_status,int log_level_pos, int user_handle,char *description,int verbose);
/**
 * Delete context from internal context management
 * @param daemon pointer to dlt daemon structure
 * @param context pointer to context to be deleted
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_context_del(DltDaemon *daemon, DltDaemonContext* context, int verbose);
/**
 * Find context with specific application id and context id
 * @param daemon pointer to dlt daemon structure
 * @param apid pointer to application id
 * @param ctid pointer to context id
 * @param verbose if set to true verbose information is printed out.
 * @return Pointer to context, null pointer on error or not found
 */
DltDaemonContext* dlt_daemon_context_find(DltDaemon *daemon,char *apid,char *ctid,int verbose);
/**
 * Invalidate all contexts fd, if fd is reused
 * @param daemon pointer to dlt daemon structure
 * @param fd file descriptor
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_contexts_invalidate_fd(DltDaemon *daemon,int fd,int verbose);
/**
 * Clear all contexts in internal context management
 * @param daemon pointer to dlt daemon structure
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_contexts_clear(DltDaemon *daemon,int verbose);
/**
 * Load contexts from file to internal context management
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file to be used for loading
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_contexts_load(DltDaemon *daemon,const char *filename, int verbose);
/**
 * Save contexts from internal context management to file
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file to be used for saving
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_contexts_save(DltDaemon *daemon,const char *filename, int verbose);
/**
 * Load persistant configuration
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file to be used for loading
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_configuration_load(DltDaemon *daemon,const char *filename, int verbose);
/**
 * Save configuration persistantly
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file to be used for saving
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_configuration_save(DltDaemon *daemon,const char *filename, int verbose);


/**
 * Send user message DLT_USER_MESSAGE_LOG_LEVEL to user application
 * @param daemon pointer to dlt daemon structure
 * @param context pointer to context for response
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_user_send_log_level(DltDaemon *daemon,DltDaemonContext *context, int verbose);

/**
 * Send user message DLT_USER_MESSAGE_LOG_STATE to user application
 * @param daemon pointer to dlt daemon structure
 * @param app pointer to application for response
 * @param verbose if set to true verbose information is printed out.
 * @return negative value if there was an error
 */
int dlt_daemon_user_send_log_state(DltDaemon *daemon,DltDaemonApplication *app,int verbose);

/**
 * Send user messages to all user applications using default context, or trace status
 * to update those values
 * @param daemon pointer to dlt daemon structure
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_user_send_default_update(DltDaemon *daemon, int verbose);

/**
 * Send user messages to all user applications the log status
 * everytime the client is connected or disconnected.
 * @param daemon pointer to dlt daemon structure
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_user_send_all_log_state(DltDaemon *daemon, int verbose);

/**
 * Process reset to factory default control message
 * @param daemon pointer to dlt daemon structure
 * @param filename name of file containing the runtime defaults for applications
 * @param filename1 name of file containing the runtime defaults for contexts
 * @param verbose if set to true verbose information is printed out.
 */
void dlt_daemon_control_reset_to_factory_default(DltDaemon *daemon,const char *filename, const char *filename1, int verbose);

/**
 * Change the logging state of dlt daemon
 * @param daemon pointer to dlt daemon structure
 * @param newState the requested new state
 */
void dlt_daemon_change_state(DltDaemon *daemon, DltDaemonState newState);

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_DAEMON_COMMON_H */
