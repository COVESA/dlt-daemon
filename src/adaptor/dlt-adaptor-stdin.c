/*
 * Dlt Adaptor for forwarding Standard In messages to Dlt
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-adaptor-stdin.c                                           **
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
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlt_common.h"
#include "dlt_user.h"

#define MAXSTRLEN 1024

#define PS_DLT_APP_DESC      "stdin adaptor application"
#define PS_DLT_CONTEXT_DESC  "stdin adaptor context"

#define PS_DLT_APP "SINA"
#define PS_DLT_CONTEXT "SINC"

DLT_DECLARE_CONTEXT(mycontext);

int main(int argc, char* argv[])
{
    char str[MAXSTRLEN];
    int opt;

    char apid[DLT_ID_SIZE];
    char ctid[DLT_ID_SIZE];
    char version[255];

    dlt_set_id(apid, PS_DLT_APP);
    dlt_set_id(ctid, PS_DLT_CONTEXT);

    while ((opt = getopt(argc, argv, "a:c:h")) != -1)
    {
        switch (opt)
        {
        case 'a':
        {
            dlt_set_id(apid,optarg);
            break;
        }
        case 'c':
        {
            dlt_set_id(ctid,optarg);
            break;
        }
        case 'h':
        {
            dlt_get_version(version);

            printf("Usage: dlt-adaptor-stdin [options]\n");
            printf("Adaptor for forwarding input from stdin to DLT daemon.\n");
            printf("%s \n", version);
            printf("Options:\n");
            printf("-a apid      - Set application id to apid (default: SINA)\n");
            printf("-c ctid      - Set context id to ctid (default: SINC)\n");
            printf("-h           - This help\n");
            return 0;
            break;
        }
        default: /* '?' */
        {
            fprintf(stderr, "Unknown option '%c'\n", optopt);
            return -1;
        }
        }
    }

    DLT_REGISTER_APP(apid,PS_DLT_APP_DESC);
    DLT_REGISTER_CONTEXT(mycontext, ctid, PS_DLT_CONTEXT_DESC);

    while (fgets(str, MAXSTRLEN, stdin))
    {
        if (strcmp(str,"")!=0)
        {
            DLT_LOG(mycontext, DLT_LOG_INFO, DLT_STRING(str));
        }
    }

    DLT_UNREGISTER_CONTEXT(mycontext);
    DLT_UNREGISTER_APP();

    return 0;
}

