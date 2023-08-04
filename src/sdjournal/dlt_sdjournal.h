/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2022, Amazon.com, Inc. or its affiliates.
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 */

/*!
 * \author
 * Haris Okanovic <harisokn@amazon.com>
 *
 * \copyright Copyright (C) 2022 Amazon.com, Inc. or its affiliates. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 */

#ifndef DLT_SDJOURNAL_H_
#define DLT_SDJOURNAL_H_

#include "dlt-daemon.h"

int dlt_sdjournal_init(DltDaemonLocal *daemon_local, int verbose);

void dlt_sdjournal_deinit(DltDaemonLocal *daemon_local, int verbose);

/**
 * Process log entries from Systemd Journal and emit to clients.
 *
 * @param daemon          DltDaemon
 * @param daemon_local    DltDaemonLocal
 * @param rec             DltReceiver
 * @param verbose verbose flag
 * @return 0 on success, -1 otherwise
 */
int dlt_sdjournal_process(DltDaemon *daemon,
                          DltDaemonLocal *daemon_local,
                          DltReceiver *receiver,
                          int verbose);

DltReceiver* dlt_sdjournal_get_receiver(DltDaemonLocal *daemon_local);

/* Change max log level of specified app */
int dlt_sdjournal_change_app_level(DltDaemonLocal* daemon_local, const char* apid, const char* ctid, uint8_t level);

/* Change default log level */
int dlt_sdjournal_change_default_level(DltDaemonLocal* daemon_local, uint8_t level);

#endif
