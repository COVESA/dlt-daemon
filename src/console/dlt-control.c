/*
 * @licence app begin@
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2015, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*!
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt-control.cpp
*/


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-control.cpp                                               **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
*******************************************************************************/

#include <ctype.h>      /* for isprint() */
#include <stdlib.h>     /* for atoi() */
#include <sys/stat.h>   /* for S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <fcntl.h>      /* for open() */
#include <sys/uio.h>    /* for writev() */
#include <string.h>     /* for open() */

#include "dlt_client.h"
#include "dlt_user.h"

#define DLT_RECEIVE_TEXTBUFSIZE 10024  /* Size of buffer for text output */

#define DLT_CTRL_SOCK "/tmp/dlt-ctrl.sock"

#define DLT_RECEIVE_ECU_ID "RECV"

#define DLT_GLOGINFO_APID_NUM_MAX   150
#define DLT_GLOGINFO_DATA_MAX       800
#define DLT_GET_LOG_INFO_HEADER     18      /*Get log info header size in response text */
#define DLT_INVALID_LOG_LEVEL       0xF
/* Option of GET_LOG_INFO */
#define DLT_SERVICE_GET_LOG_INFO_OPT7    7    /* get Apid, ApDescription, Ctid, CtDescription, loglevel, tracestatus */

typedef struct
{
    char apid[DLT_ID_SIZE + 1]; /**< application id */
    char ctid[DLT_ID_SIZE + 1]; /**< context id */
    char *apid_desc;            /**< apid description */
    char *ctid_desc;            /**< ctxt description */
    int  log_level;             /**< log level */
    int  trace_status;          /**< trace_status */
    int  disp;                  /**< display flag */
} DltLoginfoDetail;

typedef struct
{
    DltLoginfoDetail info[DLT_GLOGINFO_DATA_MAX]; /**< structure for each ctxt/app entry*/
    int              count;                       /**< Number of app and ctxt entries */
} DltLoginfo;


typedef struct
{
    uint32_t service_id;            /**< service ID */
} PACKED DltServiceGetDefaultLogLevel;

DltClient  g_dltclient;
DltLoginfo g_get_loginfo;

/* Function prototypes */
int dlt_receive_message_callback(DltMessage *message, void *data);

typedef struct {
    int vflag;
    int yflag;
    char *evalue;

    char *avalue;
    char *cvalue;
    int svalue;
    char *mvalue;
    char *xvalue;
    int tvalue;
    int lvalue;
    int rvalue;
    int dvalue;
    int fvalue;
    int ivalue;
    int oflag;
    int gflag;
    int jvalue;
    int bvalue;
    char ecuid[4];
    DltFile file;
    DltFilter filter;
} DltReceiveData;


void hexAsciiToBinary (const char *ptr,uint8_t *binary,int *size)
{

	char ch = *ptr;
	int pos = 0;
	binary[pos] = 0;
	int first = 1;
	int found;

	for(;;)
	{

		if(ch == 0)
		{
			*size = pos;
			return;
		}


		found = 0;
		if (ch >= '0' && ch <= '9')
		{
			binary[pos] = (binary[pos] << 4) + (ch - '0');
			found = 1;
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			binary[pos] = (binary[pos] << 4) + (ch - 'A' + 10);
			found = 1;
		}
		else if (ch >= 'a' && ch <= 'f')
		{
			binary[pos] = (binary[pos] << 4) + (ch - 'a' + 10);
			found = 1;
		}
		if(found)
		{
			if(first)
				first = 0;
			else
			{
				first = 1;
				pos++;
				if(pos>=*size)
					return;
				binary[pos]=0;
			}
		}

		ch = *(++ptr);
	}

}

/**
 * Print usage information of tool.
 */
void usage()
{
    char version[255];

    dlt_get_version(version,255);

    printf("Usage: dlt-control [options] hostname/serial_device_name\n");
    printf("Send control message to DLT daemon.\n");
    printf("%s \n", version);
    printf("Options:\n");
    printf("  -v            Verbose mode\n");
    printf("  -h            Usage\n");
    printf("  -y            Serial device mode\n");
    printf("  -b baudrate   Serial device baudrate (Default: 115200)\n");
    printf("  -e ecuid      Set ECU ID (Default: RECV)\n");
    printf("\n");
    printf("  -a id		    Control message application id\n");
    printf("  -c id    		Control message context id\n");
    printf("  -s id    		Control message injection service id\n");
    printf("  -m message    Control message injection in ASCII\n");
    printf("  -x message    Control message injection in Hex e.g. 'ad 01 24 ef'\n");
    printf("  -t milliseconds Timeout to terminate application (Default:1000)'\n");
    printf("  -l loglevel      Set the log level (0=off - 6=verbose default= -1)\n");
    printf("      supported options:\n");
    printf("       -l level -a appid -c ctid\n");
    printf("       -l level -a abc* (set level for all ctxts of apps name starts with abc)\n");
    printf("       -l level -a appid (set level for all ctxts of this app)\n");
    printf("       -l level -c xyz* (set level for all ctxts whose name starts with xyz)\n");
    printf("       -l level -c ctxid (set level for the particular ctxt)\n");
    printf("       -l level (set level for all the registered contexts)\n");
    printf("  -r tracestatus  Set the trace status (0=off - 1=on,255=default)\n");
    printf("  -d loglevel	  Set the default log level (0=off - 5=verbose)\n");
    printf("  -f tracestatus  Set the default trace status (0=off - 1=on)\n");
    printf("  -i enable  	  Enable timing packets (0=off - 1=on)\n");
    printf("  -o 		  	  Store configuration\n");
    printf("  -g 		  	  Reset to factory default\n");
    printf("  -j               Get log info\n");
    printf("  -u               unix port\n");
}
/**
 * Function for sending get log info ctrl msg and printing the response.
 */
void dlt_process_get_log_info(void)
{
    char apid[DLT_ID_SIZE+1] = {0};
    int cnt;

    dlt_getloginfo_init();

    /* send control message*/
    if (0 != dlt_client_get_log_info(&g_dltclient))
    {
        fprintf(stderr, "ERROR: Could not get log info\n");
        return;
    }

    for (cnt = 0; cnt < g_get_loginfo.count; cnt++)
    {
        if (strncmp(apid, g_get_loginfo.info[cnt].apid, DLT_ID_SIZE) != 0)
        {
            printf("APID:%4s ", g_get_loginfo.info[cnt].apid);
            apid[DLT_ID_SIZE] = 0;
            dlt_set_id(apid, g_get_loginfo.info[cnt].apid);

            if (g_get_loginfo.info[cnt].apid_desc != 0)
            {
                printf("%s\n", g_get_loginfo.info[cnt].apid_desc);
            }
            else
            {
                printf("\n");
            }
        }

        if (strncmp(apid, g_get_loginfo.info[cnt].apid, DLT_ID_SIZE) == 0)
        {
            printf("%4s %2d %2d %s\n", g_get_loginfo.info[cnt].ctid, g_get_loginfo.info[cnt].log_level,
            g_get_loginfo.info[cnt].trace_status, g_get_loginfo.info[cnt].ctid_desc);
        }
    }
    dlt_getloginfo_free();
}

/**
 * Main function of tool.
 */
int main(int argc, char* argv[])
{
    DltReceiveData dltdata;
    int c;
    int index;
    char *endptr = NULL;

    /* Initialize dltdata */
    dltdata.vflag = 0;
    dltdata.yflag = 0;
    dltdata.evalue = 0;
    dltdata.bvalue = 0;

    dltdata.avalue = 0;
    dltdata.cvalue = 0;
    dltdata.svalue = 0;
    dltdata.mvalue = 0;
    dltdata.xvalue = 0;
    dltdata.tvalue = 1000;
    dltdata.lvalue = DLT_INVALID_LOG_LEVEL;
    dltdata.rvalue = -1;
    dltdata.dvalue = -1;
    dltdata.fvalue = -1;
    dltdata.ivalue = -1;
    dltdata.oflag = -1;
    dltdata.gflag = -1;
    dltdata.jvalue = 0;
    /* Fetch command line arguments */
    opterr = 0;

    while ((c = getopt (argc, argv, "vhye:b:a:c:s:m:x:t:l:r:d:f:i:ogju")) != -1)
        switch (c)
        {
        case 'v':
			{
            	dltdata.vflag = 1;
            	break;
			}
        case 'h':
			{
            	usage();
            	return -1;
			}
        case 'y':
            {
                dltdata.yflag = DLT_CLIENT_MODE_SERIAL;
                break;
            }
        case 'e':
			{
            	dltdata.evalue = optarg;
            	break;
			}
        case 'b':
			{
            	dltdata.bvalue = atoi(optarg);
            	break;
			}

        case 'a':
            {
                dltdata.avalue = optarg;
                if (strlen(dltdata.avalue) > DLT_ID_SIZE)
                {
                    fprintf (stderr, "Invalid appid\n");
                    return -1;
                }
                break;
            }
        case 'c':
            {
                dltdata.cvalue = optarg;
                if (strlen(dltdata.cvalue) > DLT_ID_SIZE)
                {
                    fprintf (stderr, "Invalid context id\n");
                    return -1;
                }
                break;
            }
        case 's':
			{
            	dltdata.svalue = atoi(optarg);
            	break;
			}
        case 'm':
			{
            	dltdata.mvalue = optarg;
            	break;
			}
        case 'x':
			{
            	dltdata.xvalue = optarg;
            	break;
			}
        case 't':
			{
            	dltdata.tvalue = atoi(optarg);;
            	break;
			}
        case 'l':
            {
                dltdata.lvalue = strtol(optarg, &endptr, 10);
                if ((dltdata.lvalue < DLT_LOG_DEFAULT) || (dltdata.lvalue > DLT_LOG_VERBOSE))
                {
                    fprintf (stderr, "invalid log level, supported log level 0-6\n");
                    return -1;
                }
                break;
            }
        case 'r':
			{
            	dltdata.rvalue = atoi(optarg);;
            	break;
			}
        case 'd':
			{
            	dltdata.dvalue = atoi(optarg);;
            	break;
			}
        case 'f':
			{
            	dltdata.fvalue = atoi(optarg);;
            	break;
			}
        case 'i':
			{
            	dltdata.ivalue = atoi(optarg);;
            	break;
			}
        case 'o':
			{
            	dltdata.oflag = 1;
            	break;
			}
        case 'g':
			{
            	dltdata.gflag = 1;
            	break;
			}
        case 'j':
            {
                dltdata.jvalue = 1;
                break;
            }
        case 'u':
            {
                dltdata.yflag = DLT_CLIENT_MODE_UNIX;
                break;
            }
        case '?':
			{
		        if (optopt == 'o' || optopt == 'f')
				{
		            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		        }
				else if (isprint (optopt))
				{
		            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		        }
				else
				{
		            fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
				}
		        /* unknown or wrong option used, show usage information and terminate */
		        usage();
		        return -1;
			}
        default:
			{
            	abort ();
                return -1;//for parasoft
			}
        }

    /* Initialize DLT Client */
    dlt_client_init(&g_dltclient, dltdata.vflag);

    /* Register callback to be called when message was received */
    dlt_client_register_message_callback(dlt_receive_message_callback);

    /* Setup DLT Client structure */
    if (dltdata.yflag == DLT_CLIENT_MODE_SERIAL)
    {
        g_dltclient.mode = DLT_CLIENT_MODE_SERIAL;
    }
    else if (dltdata.yflag == DLT_CLIENT_MODE_UNIX)
    {
        g_dltclient.mode = DLT_CLIENT_MODE_UNIX;
        g_dltclient.socketPath = DLT_CTRL_SOCK;
    }
    else
    {
        g_dltclient.mode = DLT_CLIENT_MODE_TCP;
    }

    if (g_dltclient.mode==DLT_CLIENT_MODE_TCP)
    {
        for (index = optind; index < argc; index++)
        {
            g_dltclient.servIP = argv[index];
        }

        if (g_dltclient.servIP == 0)
        {
            /* no hostname selected, show usage and terminate */
            fprintf(stderr,"ERROR: No hostname selected\n");
            usage();
            dlt_client_cleanup(&g_dltclient,dltdata.vflag);
            return -1;
        }
    }
    else if (g_dltclient.mode == DLT_CLIENT_MODE_SERIAL)
    {
        for (index = optind; index < argc; index++)
        {
            g_dltclient.serialDevice = argv[index];
        }

        if (g_dltclient.serialDevice == 0)
        {
            /* no serial device name selected, show usage and terminate */
            fprintf(stderr,"ERROR: No serial device name specified\n");
            usage();
            return -1;
        }

		dlt_client_setbaudrate(&g_dltclient,dltdata.bvalue);
    }

    /* initialise structure to use DLT file */
    dlt_file_init(&(dltdata.file),dltdata.vflag);

    /* first parse filter file if filter parameter is used */
    dlt_filter_init(&(dltdata.filter),dltdata.vflag);

    if (dltdata.evalue)
	{
        dlt_set_id(dltdata.ecuid,dltdata.evalue);
    }
	else
	{
        dlt_set_id(dltdata.ecuid,DLT_RECEIVE_ECU_ID);
	}

    /* Connect to TCP socket or open serial device */
    if (dlt_client_connect(&g_dltclient, dltdata.vflag) != DLT_RETURN_ERROR)
    {
    	/* send injection message */
    	if(dltdata.mvalue && dltdata.avalue && dltdata.cvalue)
    	{
    		/* ASCII */
    		printf("Send injection message:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("ServiceId: %d\n",dltdata.svalue);
    		printf("Message: %s\n",dltdata.mvalue);
    		/* send control message in ascii */
            if (dlt_client_send_inject_msg(&g_dltclient,
                                                dltdata.avalue,
                                                dltdata.cvalue,
                                                dltdata.svalue,
                                                (uint8_t*)dltdata.mvalue,
                                                strlen(dltdata.mvalue)) != DLT_RETURN_OK)
            {
                fprintf(stderr, "ERROR: Could not send inject message\n");
            }
    	}
    	else if(dltdata.xvalue && dltdata.avalue && dltdata.cvalue)
    	{
    		/* Hex */
    		uint8_t buffer[1024];
    		int size = 1024;
    		printf("Send injection message:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("ServiceId: %d\n",dltdata.svalue);
    		printf("Message: %s\n",dltdata.xvalue);
    		hexAsciiToBinary(dltdata.xvalue,buffer,&size);
    		printf("Size: %d\n",size);
            /* send control message in hex */
            if (dlt_client_send_inject_msg(&g_dltclient,
                                                dltdata.avalue,
                                                dltdata.cvalue,
                                                dltdata.svalue,
                                                buffer,size) != DLT_RETURN_OK)
            {
                fprintf(stderr, "ERROR: Could not send inject message\n");
            }
		}
         else if (dltdata.lvalue != DLT_INVALID_LOG_LEVEL) /*&& dltdata.avalue && dltdata.cvalue)*/
        {
            if ((dltdata.avalue == 0) && (dltdata.cvalue == 0))
            {
                if (dltdata.vflag)
                {
                    printf("Set all log level:\n");
                    printf("Loglevel: %d\n", dltdata.lvalue);
                }
                if (0 != dlt_client_send_all_log_level(&g_dltclient,
                                                       dltdata.lvalue))
                {
                    fprintf(stderr, "ERROR: Could not send log level\n");
                }
            }
            else
            {
                /* log level */
                if (dltdata.vflag)
                {
                    printf("Set log level:\n");
                    printf("AppId: %s\n", dltdata.avalue);
                    printf("ConId: %s\n", dltdata.cvalue);
                    printf("Loglevel: %d\n", dltdata.lvalue);
                }
                /* send control message*/
                if (0 != dlt_client_send_log_level(&g_dltclient,
                                                   dltdata.avalue,
                                                   dltdata.cvalue,
                                                   dltdata.lvalue))
                {
                    fprintf(stderr, "ERROR: Could not send log level\n");
                }
            }
        }
    	else if(dltdata.rvalue!=-1 && dltdata.avalue && dltdata.cvalue)
    	{
    		/* trace status */
    		printf("Set trace status:\n");
    		printf("AppId: %s\n",dltdata.avalue);
    		printf("ConId: %s\n",dltdata.cvalue);
    		printf("TraceStatus: %d\n",dltdata.rvalue);
    		/* send control message in*/
            if (dlt_client_send_trace_status(&g_dltclient,
                                                  dltdata.avalue,
                                                  dltdata.cvalue,
                                                  dltdata.rvalue) != DLT_RETURN_OK)
            {
                fprintf(stderr, "ERROR: Could not send trace status\n");
            }
    	}
    	else if(dltdata.dvalue!=-1)
    	{
    		/* default log level */
    		printf("Set default log level:\n");
    		printf("Loglevel: %d\n",dltdata.dvalue);
    		/* send control message in*/
            if (dlt_client_send_default_log_level(&g_dltclient, dltdata.dvalue) != DLT_RETURN_OK)
            {
                fprintf (stderr, "ERROR: Could not send default log level\n");
            }
    	}
    	else if(dltdata.rvalue!=-1)
    	{
    		/* default trace status */
    		printf("Set default trace status:\n");
    		printf("TraceStatus: %d\n",dltdata.rvalue);
    		/* send control message in*/
            if (dlt_client_send_default_trace_status(&g_dltclient, dltdata.rvalue) != DLT_RETURN_OK)
            {
                fprintf (stderr, "ERROR: Could not send default trace status\n");
            }
    	}
    	else if(dltdata.ivalue!=-1)
    	{
    		/* timing pakets */
    		printf("Set timing pakets:\n");
    		printf("Timing packets: %d\n",dltdata.ivalue);
    		/* send control message in*/
            if (dlt_client_send_timing_pakets(&g_dltclient, dltdata.ivalue) != DLT_RETURN_OK)
            {
                fprintf (stderr, "ERROR: Could not send timing packets\n");
            }
    	}
    	else if(dltdata.oflag!=-1)
    	{
    		/* default trace status */
    		printf("Store config\n");
    		/* send control message in*/
            if (dlt_client_send_store_config(&g_dltclient) != DLT_RETURN_OK)
            {
                fprintf (stderr, "ERROR: Could not send store config\n");
            }
    	}
    	else if(dltdata.gflag!=-1)
    	{
    		/* reset to factory default */
    		printf("Reset to factory default\n");
    		/* send control message in*/
            if (dlt_client_send_reset_to_factory_default(&g_dltclient) != DLT_RETURN_OK)
            {
                fprintf (stderr, "ERROR: Could send reset to factory default\n");
            }
    	}
        else if (dltdata.jvalue == 1)
        {
            /* get log info */
            printf("Get log info:\n");
            dlt_process_get_log_info();
        }
        /* Dlt Client Main Loop */
        //dlt_client_main_loop(&dltclient, &dltdata, dltdata.vflag);

    	/* Wait timeout */
    	usleep(dltdata.tvalue*1000);

        /* Dlt Client Cleanup */
        dlt_client_cleanup(&g_dltclient,dltdata.vflag);
    }

    dlt_file_free(&(dltdata.file),dltdata.vflag);

    dlt_filter_free(&(dltdata.filter),dltdata.vflag);

    return 0;
}

void dlt_getloginfo_conv_ascii_to_id(char *rp, int *rp_count, char *wp, int len)
{
    char number16[6]={0};
    char *endptr;
    int count;

    if ((rp == NULL) || (rp_count == NULL) || (wp == NULL))
    {
        return;
    }
        /* ------------------------------------------------------
           from: [72 65 6d 6f ] -> to: [0x72,0x65,0x6d,0x6f,0x00]
           ------------------------------------------------------ */
        number16[0] = '+';
        number16[1] = '0';
        number16[2] = 'x';
        for (count = 0; count < (len - 1); count++)
        {
            number16[3] = *(rp + *rp_count + 0);
            number16[4] = *(rp + *rp_count + 1);
            *(wp + count) = strtol(number16, &endptr, 16);
            *rp_count += 3;
        }
        *(wp + count) = 0;
    return;
}

uint16_t dlt_getloginfo_conv_ascii_to_uint16_t(char *rp, int *rp_count)
{
    char num_work[8];
    char *endptr;

    if ((rp == NULL) || (rp_count == NULL))
    {
        return -1;
    }
    /* ------------------------------------------------------
       from: [89 13 ] -> to: ['+0x'1389\0] -> to num
       ------------------------------------------------------ */
    num_work[0] = '+';
    num_work[1] = '0';
    num_work[2] = 'x';
    num_work[3] = *(rp + *rp_count + 3);
    num_work[4] = *(rp + *rp_count + 4);
    num_work[5] = *(rp + *rp_count + 0);
    num_work[6] = *(rp + *rp_count + 1);
    num_work[7] = 0;
    *rp_count += 6;

    return strtol(num_work, &endptr, 16);
}
int16_t dlt_getloginfo_conv_ascii_to_int16_t(char *rp, int *rp_count)
{
    char num_work[6];
    char *endptr;

    if ((rp == NULL) || (rp_count == NULL))
    {
        return -1;
    }
    /* ------------------------------------------------------
       from: [89 ] -> to: ['0x'89\0] -> to num
       ------------------------------------------------------ */
    num_work[0] = '0';
    num_work[1] = 'x';
    num_work[2] = *(rp + *rp_count + 0);
    num_work[3] = *(rp + *rp_count + 1);
    num_work[4] = 0;
    *rp_count += 3;

    return (signed char)strtol(num_work, &endptr, 16);
}

void dlt_getloginfo_init(void)
{
    int cnt;

    g_get_loginfo.count = 0;
    for (cnt = 0; cnt < DLT_GLOGINFO_DATA_MAX; cnt++)
    {
        g_get_loginfo.info[cnt].apid_desc = NULL;
        g_get_loginfo.info[cnt].ctid_desc = NULL;
    }
}

void dlt_getloginfo_free(void)
{
    int cnt;

    for (cnt = 0; cnt < DLT_GLOGINFO_DATA_MAX; cnt++)
    {
        if (g_get_loginfo.info[cnt].apid_desc != 0)
        {
            free(g_get_loginfo.info[cnt].apid_desc);
        }
        if (g_get_loginfo.info[cnt].ctid_desc != 0)
        {
            free(g_get_loginfo.info[cnt].ctid_desc);
        }
    }
}

/**
 * Function to parse the response text and identifying service id and its options.
 */
int dlt_set_loginfo_parse_service_id(char *resp_text, int *service_id, int *service_opt,  char *cb_result)
{
    int ret;
    char get_log_info_tag[13];
    char service_opt_str[3];

    if ((resp_text == NULL) || (service_id == NULL) || (service_opt == NULL) || (cb_result == NULL))
    {
        return -1;
    }
    /* ascii type, syntax is 'get_log_info, ..' */
    /* check target id */
    strncpy(get_log_info_tag, "get_log_info", strlen("get_log_info"));
    ret = memcmp((void *)resp_text, (void *)get_log_info_tag, sizeof(get_log_info_tag)-1);
    if (ret == 0)
    {
        *service_id = DLT_SERVICE_ID_GET_LOG_INFO;
        *cb_result = 0;
        /* reading the response mode from the resp_text. eg. option 7*/
        service_opt_str[0] = *(resp_text+14);
        service_opt_str[1] = *(resp_text+15);
        service_opt_str[2] = 0;
        *service_opt = atoi( service_opt_str );
    }

    return ret;
}

/**
 * Main function to convert the response text in to proper get log info data and
 * filling it in the DltLoginfo structure.
 */
int dlt_getloginfo_make_loginfo(char *resp_text, int service_opt)
{
    char *rp;
    int rp_count;
    int loginfo_count;
    uint16_t reg_apid_count;
    uint16_t reg_ctid_count;
    uint16_t reg_apid_num;
    uint16_t reg_ctid_num;
    uint16_t reg_apid_dsc_len;
    uint16_t reg_ctid_dsc_len;
    char reg_apid[DLT_ID_SIZE+1];
    char reg_ctid[DLT_ID_SIZE+1];
    char *reg_apid_dsc;
    char *reg_ctid_dsc;

    if (resp_text == NULL)
    {
        return -1;
    }
    /* ------------------------------------------------------
       get_log_info data structure(all data is ascii)

       get_log_info, aa, bb bb cc cc cc cc dd dd ee ee ee ee ff gg hh hh ii ii ii .. ..
                     ~~  ~~~~~ ~~~~~~~~~~~ ~~~~~ ~~~~~~~~~~~~~~
                               cc cc cc cc dd dd ee ee ee ee ff gg hh hh ii ii ii .. ..
                         jj jj kk kk kk .. ..
                               ~~~~~~~~~~~ ~~~~~ ~~~~~~~~~~~~~~
       aa         : get mode (fix value at 0x07)
       bb bb      : list num of apid (little endian)
       cc cc cc cc: apid
       dd dd      : list num of ctid (little endian)
       ee ee ee ee: ctid
       ff         : log level
       gg         : trase status
       hh hh      : description length of ctid
       ii ii ..   : description text of ctid
       jj jj      : description length of apid
       kk kk ..   : description text of apid
      ------------------------------------------------------ */

    /* create all target convert list */

    /* rp set header */
    rp = (resp_text + DLT_GET_LOG_INFO_HEADER);
    rp_count = 0;
    /* get reg_apid_num */
    reg_apid_num = dlt_getloginfo_conv_ascii_to_uint16_t(rp, &rp_count);
    loginfo_count = g_get_loginfo.count;

    if (reg_apid_num > DLT_GLOGINFO_APID_NUM_MAX)
    {
        fprintf(stderr, "GET_LOG_INFO ERROR: APID MAX Over\n");
        g_get_loginfo.count = 0;
        return -1;
    }

    /* search for target apid */
    for (reg_apid_count = 0; reg_apid_count < reg_apid_num; reg_apid_count++)
    {
        /* get reg_apid */
        dlt_getloginfo_conv_ascii_to_id(rp, &rp_count, reg_apid, DLT_ID_SIZE+1);

        /* get reg_ctid_num of current reg_apid */
        reg_ctid_num = dlt_getloginfo_conv_ascii_to_uint16_t(rp, &rp_count);
        if (loginfo_count + reg_ctid_num > DLT_GLOGINFO_DATA_MAX)
        {
            fprintf(stderr, "GET_LOG_INFO ERROR: LOG DATA MAX Over\n");
            g_get_loginfo.count = 0;
            return -1;
        }
        for (reg_ctid_count = 0; reg_ctid_count < reg_ctid_num; reg_ctid_count++)
        {
            /* get reg_ctid */
            dlt_getloginfo_conv_ascii_to_id(rp, &rp_count, reg_ctid, DLT_ID_SIZE+1);

            g_get_loginfo.info[ loginfo_count ].apid[DLT_ID_SIZE] = 0;
            dlt_set_id(g_get_loginfo.info[ loginfo_count ].apid, reg_apid);
            g_get_loginfo.info[ loginfo_count ].ctid[DLT_ID_SIZE] = 0;
            dlt_set_id(g_get_loginfo.info[ loginfo_count ].ctid, reg_ctid);
            g_get_loginfo.info[ loginfo_count ].log_level = dlt_getloginfo_conv_ascii_to_int16_t(rp, &rp_count);
            g_get_loginfo.info[ loginfo_count ].trace_status = dlt_getloginfo_conv_ascii_to_int16_t(rp, &rp_count);

            /* Description Information */
            if (service_opt == DLT_SERVICE_GET_LOG_INFO_OPT7)
            {
                reg_ctid_dsc_len = dlt_getloginfo_conv_ascii_to_uint16_t(rp, &rp_count);
                reg_ctid_dsc = (char *)malloc(sizeof(char) * reg_ctid_dsc_len + 1);
                if (reg_ctid_dsc == 0)
                {
                    fprintf(stderr, "malloc failed for ctxt desc\n");
                    return -1;
                }
                dlt_getloginfo_conv_ascii_to_id(rp, &rp_count, reg_ctid_dsc, reg_ctid_dsc_len+1);
                g_get_loginfo.info[ loginfo_count ].ctid_desc = reg_ctid_dsc;
            }
            loginfo_count++;
        }
        /* Description Information */
        if (service_opt == DLT_SERVICE_GET_LOG_INFO_OPT7)
        {
            reg_apid_dsc_len = dlt_getloginfo_conv_ascii_to_uint16_t(rp, &rp_count);
            reg_apid_dsc = (char *)malloc(sizeof(char) * reg_apid_dsc_len + 1);
            if (reg_apid_dsc == 0)
            {
                fprintf(stderr, "malloc failed for apid desc\n");
                return -1;
            }
            dlt_getloginfo_conv_ascii_to_id(rp, &rp_count, reg_apid_dsc, reg_apid_dsc_len+1);
            g_get_loginfo.info[ loginfo_count - reg_ctid_num ].apid_desc = reg_apid_dsc;
        }
    }
    g_get_loginfo.count = loginfo_count;

    return 0;
}

int dlt_receive_message_callback(DltMessage *message, void *data)
{
    static char resp_text[DLT_RECEIVE_TEXTBUFSIZE];
    int ret;
    int service_id;
    int service_opt;
    char cb_result;

    /* parameter check */
    if (message == NULL)
    {
        return -1;
    }
    /* to avoid warning */
    data = data;

    /* prepare storage header */
    if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
    {
        dlt_set_storageheader(message->storageheader, message->headerextra.ecu);
    }
    else
    {
        dlt_set_storageheader(message->storageheader, "LCTL");
    }

    /* get response data */
    ret = dlt_message_header(message, resp_text, DLT_RECEIVE_TEXTBUFSIZE, 0);
    if (ret < 0)
    {
        fprintf(stderr, "GET_LOG_INFO message_header result failed..\n");
        dlt_client_cleanup(&g_dltclient, 0);
        return -1;
    }

    ret = dlt_message_payload(message, resp_text, DLT_RECEIVE_TEXTBUFSIZE, DLT_OUTPUT_ASCII, 0);
    if (ret < 0)
    {
        fprintf(stderr, "GET_LOG_INFO message_payload result failed..\n");
        dlt_client_cleanup(&g_dltclient, 0);
        return -1;
    }

    /* check service id */
    ret = dlt_set_loginfo_parse_service_id(resp_text, &service_id, &service_opt, &cb_result);
    if ((ret == 0) && (service_id == DLT_SERVICE_ID_GET_LOG_INFO ))
    {
        ret = dlt_getloginfo_make_loginfo(resp_text, service_opt);

        if (ret != 0)
        {
            fprintf(stderr, "GET_LOG_INFO result failed..\n");
         }
        dlt_client_cleanup(&g_dltclient, 0);
    }

    return ret;
}
