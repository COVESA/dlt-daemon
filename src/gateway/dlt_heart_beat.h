/*
* SPDX license identifier: MPL-2.0
*
* Copyright (C) 2022, Daimler TSS GmbH
*
* This file is part of COVESA Project DLT - Diagnostic Log and Trace.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License (MPL), v. 2.0.
* If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* For further information see https://www.covesa.global/.
*
* \file dlt_heart_beat.h
*
*/


#ifndef AUTOMOTIVE_DLT_SRC_GATEWAY_DLT_HEART_BEAT_H_
#define AUTOMOTIVE_DLT_SRC_GATEWAY_DLT_HEART_BEAT_H_

#define HEART_BEAT_INTERVAL 1 // send heart beat every second
#define HEART_BEAT_WAIT 3 //the interval to wait heart beat response


/*
 * List access methods
 */
#define	SLIST_FIRST(head)		(head)
#define	SLIST_END(head)			NULL
#define	SLIST_EMPTY(head)		(LIST_FIRST(head) == LIST_END(head))
#define	SLIST_NEXT(elm)		((elm)->next)


#define SLIST_FOREACH(var, head)					\
	for((var) = SLIST_FIRST(head);					\
	    (var)!= SLIST_END(head);					\
	    (var) = SLIST_NEXT(var))


#define SLIST_LAST(var, hear) do { \
    SLIST_FOREACH(var, head) {     \
        if(SLIST_NEXT(var) == SLIST_END(head)) { \
            break;                           \
        }                               \
    }\
} while(0)


#endif //AUTOMOTIVE_DLT_SRC_GATEWAY_DLT_HEART_BEAT_H_
