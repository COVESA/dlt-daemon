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
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de> BMW 2012
 *
 * \file dlt_queue.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_queue.h                                                   **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
**                                                                            **
**  PURPOSE   : Linked list based dynamic queue                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: no                                           **
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
**  lm          Lassi Marttala             Eureka GmbH                        **
*******************************************************************************/
#include <stdlib.h>
#include <semaphore.h>

#include "dlt_queue.h"

// Thread safety
static sem_t dlt_queue_mutex;
#define DLT_QUEUE_SEM_LOCK() { sem_wait(&dlt_queue_mutex); }
#define DLT_QUEUE_SEM_FREE() { sem_post(&dlt_queue_mutex); }

dlt_queue_item *dlt_queue_create_item()
{
	dlt_queue_item *item = malloc(sizeof(dlt_queue_item));
	item->item = NULL;
	item->item_size = 0;
	item->next_item = NULL;
	return item;
}

void dlt_queue_free_item(dlt_queue_item *item)
{
	if(item != NULL)
	{
		free(item);
		item = NULL;
	}
}

dlt_queue *dlt_queue_create()
{
	static int init_done = 0;
	if(!init_done)
	{
		init_done = 1;
		sem_init(&dlt_queue_mutex, 0, 1);
	}
	return (dlt_queue *)malloc(sizeof(dlt_queue));
}

void dlt_queue_free(dlt_queue *queue)
{
	if(queue != NULL)
	{
		free(queue);
		queue = NULL;
	}
}

void dlt_queue_push(dlt_queue_item *item, dlt_queue *queue)
{
	DLT_QUEUE_SEM_LOCK()
	item->next_item = NULL;
	// Empty queue
	if(dlt_queue_is_empty(queue))
	{
		queue->head = item;
		queue->tail = item;
	}
	// Exactly one item
	else if(queue->head == queue->tail)
	{
		queue->head->next_item = (struct dlt_queue_item *)item;
		queue->tail = item;
	}
	// Default case
	else
	{
		queue->tail->next_item = (struct dlt_queue_item *)item;
		queue->tail = item;
	}
	DLT_QUEUE_SEM_FREE()
}

dlt_queue_item *dlt_queue_pop(dlt_queue *queue)
{
	DLT_QUEUE_SEM_LOCK()
	dlt_queue_item *retval = NULL;
	// Empty queue
	if(dlt_queue_is_empty(queue))
	{
		retval = NULL;
	}
	// Exactly one item
	else if(queue->head == queue->tail)
	{
		retval = queue->head;
		retval->next_item = NULL;
		queue->head = NULL;
		queue->tail = NULL;
	}
	// Default case
	else
	{
		retval = queue->head;
		queue->head = (dlt_queue_item *)retval->next_item;
		retval->next_item = NULL;
	}
	DLT_QUEUE_SEM_FREE()
	return retval;
}

int dlt_queue_is_empty(dlt_queue *queue)
{
	if(queue->head == NULL && queue->tail == NULL)
		return 1;
	return 0;
}

int dlt_queue_item_count(dlt_queue *queue)
{
	DLT_QUEUE_SEM_LOCK()
	int retval = 0;
	dlt_queue_item *ptr = queue->head;
	while(ptr != NULL)
	{
		ptr = (dlt_queue_item *) ptr->next_item;
		retval++;
	}
	DLT_QUEUE_SEM_FREE()
	return retval;
}
