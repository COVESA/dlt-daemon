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
 * \file dlt-test-queue.h
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
**  PURPOSE   : Test for queue implementation                                 **
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

#include "dlt_queue.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
	char *item_content = "item_content\n";
	int item_len = strlen(item_content) + 1;
	dlt_queue *queue = dlt_queue_create();
	printf("#1\tdlt_queue_is_empty returns\t%d\n", dlt_queue_is_empty(queue));

	dlt_queue_item *first = dlt_queue_create_item();
	first->item = item_content;
	first->item_size = item_len;
	dlt_queue_push(first, queue);

	printf("#2\tdlt_queue_is_empty returns\t%d\n", dlt_queue_is_empty(queue));
	printf("#3\tdlt_queue_item_count returns\t%d\n", dlt_queue_item_count(queue));

	dlt_queue_item *tret = dlt_queue_pop(queue);
	printf("#4\tItem content after pop:\t%d:%s", tret->item_size, (char *)tret->item);
	dlt_queue_free_item(tret);

	int i=0;
	for(i=0;i<5;i++)
	{
		dlt_queue_item *newitem = dlt_queue_create_item();
		newitem->item = item_content;
		newitem->item_size = i;
		dlt_queue_push(newitem, queue);
	}
	printf("#5\tdlt_queue_item_count returns\t%d\n", dlt_queue_item_count(queue));
	while(!dlt_queue_is_empty(queue))
	{
		dlt_queue_item *olditem = dlt_queue_pop(queue);
		printf("#6\tItem size (item number):%d\n", olditem->item_size);
		dlt_queue_free_item(olditem);
	}
	printf("#7\tdlt_queue_is_empty returns\t%d\n", dlt_queue_is_empty(queue));
	printf("#8\tdlt_queue_item_count returns\t%d\n", dlt_queue_item_count(queue));
	return 0;
}
