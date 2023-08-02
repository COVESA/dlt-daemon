/**
 * Copyright (C) 2013 - 2015  Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt - Diagnostic Log and Trace console apps.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christoph Lipka clipka@jp.adit-jv.com
 * \author Anitha BA <anithaammaji.baggam@in.bosch.com> ADIT 2015
 * \author Frederic Berat <fberat@de.adit-jv.com> ADIT 2015
 *
 * \file dlt-logstorage-list.h
 * For further information see http://www.covesa.org/.
 */

#ifndef _DLT_LOGSTORAGE_LIST_H_
#define _DLT_LOGSTORAGE_LIST_H_

/* Return 0 it the node as been added (or is already present) */
int logstorage_store_dev_info(const char *node, const char *path);
/* Returns the mount point if node has been found and deleted. NULL either */
char *logstorage_delete_dev_info(const char *node);

#endif
