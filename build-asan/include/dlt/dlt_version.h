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

#define _DLT_PACKAGE_VERSION_STATE "STABLE"
#define _DLT_PACKAGE_VERSION "2.18.10"
#define _DLT_PACKAGE_MAJOR_VERSION "2"
#define _DLT_PACKAGE_MINOR_VERSION "18"
#define _DLT_PACKAGE_PATCH_LEVEL "10"
#define _DLT_PACKAGE_REVISION ""

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
