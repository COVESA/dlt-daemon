/*
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of COVESA Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.covesa.org/.
 */

/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
#ifndef __DLT_VERSION_H_
#define __DLT_VERSION_H_

#define _DLT_PACKAGE_VERSION_STATE "@DLT_VERSION_STATE@"
#define _DLT_PACKAGE_VERSION "@PROJECT_VERSION@"
#define _DLT_PACKAGE_MAJOR_VERSION "@PROJECT_VERSION_MAJOR@"
#define _DLT_PACKAGE_MINOR_VERSION "@PROJECT_VERSION_MINOR@"
#define _DLT_PACKAGE_PATCH_LEVEL "@PROJECT_VERSION_PATCH@"
#define _DLT_PACKAGE_REVISION "@DLT_REVISION@"

#ifdef DLT_SYSTEMD_ENABLE
#define _DLT_SYSTEMD_ENABLE "+SYSTEMD"
#else
#define _DLT_SYSTEMD_ENABLE "-SYSTEMD"
#endif

#ifdef DLT_SYSTEMD_WATCHDOG_ENABLE
#define _DLT_SYSTEMD_WATCHDOG_ENABLE "+SYSTEMD_WATCHDOG"
#else
#define _DLT_SYSTEMD_WATCHDOG_ENABLE "-SYSTEMD_WATCHDOG"
#endif

#ifdef DLT_TEST_ENABLE
#define _DLT_TEST_ENABLE "+TEST"
#else
#define _DLT_TEST_ENABLE "-TEST"
#endif

#ifdef DLT_SHM_ENABLE
#define _DLT_SHM_ENABLE "+SHM"
#else
#define _DLT_SHM_ENABLE "-SHM"
#endif

#endif
