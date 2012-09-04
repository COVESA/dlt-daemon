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
 * \file dlt_common_cfg.h
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
#ifndef __DLT_QUEUE_H_
#define __DLT_QUEUE_H_
#include <stdint.h>

/**
 * \brief One item in the linked list
 */
typedef struct {
	void 					*item;
	uint16_t 				item_size;
	struct dlt_queue_item	*next_item;
}dlt_queue_item;

/**
 * \brief Linked list of items
 */
typedef struct {
	dlt_queue_item	*head;
	dlt_queue_item	*tail;
} dlt_queue;

/**
 * \brief Create new empty queue item
 * Allocates a new item and returns a pointer to it
 * @return Pointer to the new item
 */
dlt_queue_item *dlt_queue_create_item();

/**
 * \brief Free an queue item
 * Frees the memory of the queue item
 * @param item Pointer to the item to be freed
 */
void dlt_queue_free_item(dlt_queue_item *item);

/**
 * \brief Create a new empty queue
 * Queue is created with no items in it.
 * User is responsible for calling dlt_queue_free after
 * the queue is not needed anymore.
 * @return New allocated queue
 */
dlt_queue *dlt_queue_create();

/**
 * \brief Delete a queue
 * Free the memory of the queue structure.
 * Queued items are NOT freed. User must first pop all the items
 * to remove the from queue and fere their memory themself.
 * @param queue the queue to be freed
 */
void dlt_queue_free(dlt_queue *queue);

/**
 * \brief Push an item to the end of the queue
 * Add a new item to the queue. It is now the new tail item.
 * Queue does not take ownership of the item. User still must
 * handle the freeing of the memory.
 * @param item New item to add to the queue
 * @param queue Queue to add the item to.
 */
void dlt_queue_push(dlt_queue_item *item, dlt_queue *queue);

/**
 * \brief Pop an item from the head of the queue
 * Remove an item from the head of the queue and return it.
 * @param queue Queue to pop from
 * @return Pointer to the popped item
 */
dlt_queue_item *dlt_queue_pop(dlt_queue *queue);

/**
 * \brief Checks if the queue is empty
 * Checks if the head and and tail are null pointers
 * and returns the result.
 * @param queue Queue to check
 * @return 1 if queue is empty, 0 if not.
 */
int dlt_queue_is_empty(dlt_queue *queue);

/**
 * \brief Counts the items in the queue
 * Walks the item chain and reports the number of items in
 * the queue. If you only need to know if there are items
 * or not, please use dlt_queue_is_empty, which is cheaper
 * than counting the items.
 * @param queue Queue to check
 * @return number of items.
 */
int dlt_queue_item_count(dlt_queue *queue);
#endif
