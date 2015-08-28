/**
 * Copyright (C) 2015 Intel Corporation
 *
 */

#include <unistd.h> /* for fork() */

#include "dlt.h"

/**
 * @brief sample code for using pre-registered contexts
 */
int main()
{
  DltContext mainContext;
  DLT_REGISTER_CONTEXT(mainContext, "CTXP", "main context");

  DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message before app registered"));
  usleep(200000);

  DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("Second message before app registered"));
  usleep(200000);

  DLT_REGISTER_APP("PRNT", "Sample pre-register application");

  DLT_LOG(mainContext, DLT_LOG_WARN, DLT_STRING("First message after app registered"));
  usleep(200000);

  DLT_UNREGISTER_APP();

  return 0;
}
