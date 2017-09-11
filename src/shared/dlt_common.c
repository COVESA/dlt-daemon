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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 * Markus Klein <Markus.Klein@esk.fraunhofer.de>
 * Mikko Rapeli <mikko.rapeli@bmw.de>
 *
 * \copyright Copyright © 2011-2015 BMW AG. \n
 * License MPL-2.0: Mozilla Public License version 2.0 http://mozilla.org/MPL/2.0/.
 *
 * \file dlt_common.c
 */

#include <stdio.h>
#include <stdlib.h>   /* for malloc(), free() */
#include <string.h>   /* for strlen(), memcmp(), memmove() */
#include <time.h>     /* for localtime(), strftime() */
#include <limits.h>   /* for NAME_MAX */
#include <inttypes.h> /* for PRI formatting macro */
#include <stdarg.h>

#include <errno.h>
#include <sys/stat.h> /* for mkdir() */

#include "dlt_user_shared.h"
#include "dlt_common.h"
#include "dlt_common_cfg.h"

#include "dlt_version.h"

#if defined (__WIN32__) || defined (_MSC_VER)
#include <winsock2.h> /* for socket(), connect(), send(), and recv() */
#else
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <syslog.h>
#include <time.h> /* for clock_gettime() */
#endif

#if defined (_MSC_VER)
#include <io.h>
#else
#include <unistd.h>     /* for read(), close() */
#include <sys/time.h>    /* for gettimeofday() */
#endif

#if defined (__MSDOS__) || defined (_MSC_VER)
#pragma warning(disable : 4996) /* Switch off C4996 warnings */
#include <windows.h>
#include <winbase.h>
#endif

static char str[DLT_COMMON_BUFFER_LENGTH];

const char dltSerialHeader[DLT_ID_SIZE] = { 'D','L','S',1 };
char dltSerialHeaderChar[DLT_ID_SIZE] = { 'D','L','S',1 };
char dltFifoBaseDir[PATH_MAX + 1] = "/tmp";

/* internal logging parameters */
static int logging_mode = DLT_LOG_TO_CONSOLE;
static int logging_level = LOG_INFO;
static char logging_filename[NAME_MAX + 1] = "";
static FILE *logging_handle = NULL;

char *message_type[] = {"log","app_trace","nw_trace","control","","","",""};
char *log_info[] = {"","fatal","error","warn","info","debug","verbose","","","","","","","","",""};
char *trace_type[] = {"","variable","func_in","func_out","state","vfb","","","","","","","","","",""};
char *nw_trace_type[] = {"","ipc","can","flexray","most","vfb","","","","","","","","","",""};
char *control_type[] = {"","request","response","time","","","","","","","","","","","",""};
static char *service_id[] = {"","set_log_level","set_trace_status","get_log_info","get_default_log_level","store_config","reset_to_factory_default",
                             "set_com_interface_status","set_com_interface_max_bandwidth","set_verbose_mode","set_message_filtering","set_timing_packets",
                             "get_local_time","use_ecu_id","use_session_id","use_timestamp","use_extended_header","set_default_log_level","set_default_trace_status",
                             "get_software_version","message_buffer_overflow"
                            };
static char *return_type[] = {"ok","not_supported","error","","","","","","no_matching_context_id"};

/* internal function definitions */
int dlt_buffer_get(DltBuffer *buf,unsigned char *data, int max_size,int delete);
int dlt_buffer_reset(DltBuffer *buf);
int dlt_buffer_increase_size(DltBuffer *buf);
int dlt_buffer_minimize_size(DltBuffer *buf);
void dlt_buffer_write_block(DltBuffer *buf,int *write, const unsigned char *data,unsigned int size);
void dlt_buffer_read_block(DltBuffer *buf,int *read,unsigned char *data,unsigned int size);

void dlt_print_hex(uint8_t *ptr,int size)
{
    int num;

    if (ptr == NULL)
    {
        return;
    }

    for (num=0;num<size;num++)
    {
        if (num>0)
        {
            printf(" ");
        }

        printf("%.2x",((uint8_t*)ptr)[num]);
    }
}

DltReturnValue dlt_print_hex_string(char *text, int textlength, uint8_t *ptr, int size)
{
    int num;

    if ((ptr == NULL) || (text == NULL) || (textlength <= 0) || (size < 0))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* Length 3: AB_ , A is first digit of hex number, B is second digit of hex number, _ is space */
    if (textlength<(size*3))
    {
        char str[255];
        snprintf(str, 254, "String does not fit hex data (available=%d, required=%d) !\n", textlength, size*3);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    for (num=0; num<size; num++)
    {
        if (num>0)
        {
            snprintf(text,2," ");
            text++;
        }

        snprintf(text,3,"%.2x",((uint8_t*)ptr)[num]);
        text+=2; /* 2 chars */
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_print_mixed_string(char *text, int textlength ,uint8_t *ptr, int size, int html)
{
    int required_size = 0;
    int lines, rest, i;

    if ((ptr == NULL) || (text == NULL) || (textlength <= 0) || (size < 0))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* Check maximum required size and do a length check */
    if (html == 0)
    {
        required_size=(DLT_COMMON_HEX_LINELEN+(2*DLT_COMMON_HEX_CHARS+(DLT_COMMON_HEX_CHARS-1))+DLT_COMMON_CHARLEN+DLT_COMMON_HEX_CHARS+DLT_COMMON_CHARLEN) *
                      ((size/DLT_COMMON_HEX_CHARS) + 1);
        /* Example: (8 chars line number + (2*16 chars + 15 spaces) + space + 16 ascii chars + CR) *
           ((size/16) lines + extra line for the rest) */
    }
    else
    {
        required_size=(DLT_COMMON_HEX_LINELEN+(2*DLT_COMMON_HEX_CHARS+(DLT_COMMON_HEX_CHARS-1))+DLT_COMMON_CHARLEN+DLT_COMMON_HEX_CHARS+4*DLT_COMMON_CHARLEN) *
                      ((size/DLT_COMMON_HEX_CHARS) + 1);
        /* Example: (8 chars line number + (2*16 chars + 15 spaces) + space + 16 ascii chars + 4 [HTML CR: <BR>]) *
           ((size/16) lines + extra line for the rest) */
    }

    if (textlength<required_size)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH, "String does not fit mixed data (available=%d, required=%d) !\n", textlength, required_size);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    /* print full lines */
    for (lines=0; lines< (size / DLT_COMMON_HEX_CHARS); lines++)
    {
        /* Line number */
        snprintf(text,DLT_COMMON_HEX_LINELEN+1,"%.6x: ",lines * DLT_COMMON_HEX_CHARS);
        text+=DLT_COMMON_HEX_LINELEN; /* 'XXXXXX: ' */

        /* Hex-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_hex_string(text,textlength,(uint8_t*)(ptr+(lines*DLT_COMMON_HEX_CHARS)),DLT_COMMON_HEX_CHARS);
        text+=((2*DLT_COMMON_HEX_CHARS)+(DLT_COMMON_HEX_CHARS-1)); /* 32 characters + 15 spaces */

        snprintf(text,2," ");
        text+=DLT_COMMON_CHARLEN;

        /* Char-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_char_string(&text,textlength,(uint8_t*)(ptr+(lines*DLT_COMMON_HEX_CHARS)),DLT_COMMON_HEX_CHARS);

        if (html==0)
        {
            snprintf(text,2,"\n");
            text+=DLT_COMMON_CHARLEN;
        }
        else
        {
            snprintf(text,5,"<BR>");
            text+=(4*DLT_COMMON_CHARLEN);
        }
    }

    /* print partial line */
    rest = size % DLT_COMMON_HEX_CHARS;

    if (rest>0)
    {
        /* Line number */
        snprintf(text,9,"%.6x: ", (size / DLT_COMMON_HEX_CHARS) * DLT_COMMON_HEX_CHARS);
        text+=DLT_COMMON_HEX_LINELEN; /* 'XXXXXX: ' */

        /* Hex-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_hex_string(text,textlength,(uint8_t*)(ptr+ ((size / DLT_COMMON_HEX_CHARS) * DLT_COMMON_HEX_CHARS)),rest);
        text+=2*rest+(rest-1);

        for (i=0;i<(DLT_COMMON_HEX_CHARS-rest);i++)
        {
            snprintf(text,4," xx");
            text+=(3*DLT_COMMON_CHARLEN);
        }

        snprintf(text,2," ");
        text+=DLT_COMMON_CHARLEN;

        /* Char-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_char_string(&text,textlength,(uint8_t*)(ptr+ ((size / DLT_COMMON_HEX_CHARS) * DLT_COMMON_HEX_CHARS)),rest);
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_print_char_string(char **text, int textlength, uint8_t *ptr, int size)
{
    int num;

    if ((text == NULL) || (ptr == NULL) || (*text == NULL) || (textlength <= 0) || (size < 0))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (textlength < size)
    {
        char str[255];
        snprintf(str, 254, "String does not fit character data (available=%d, required=%d) !\n", textlength, size);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    for (num=0; num<size; num++)
    {
        if ( (((char*)ptr)[num]<DLT_COMMON_ASCII_CHAR_SPACE) || (((char*)ptr)[num]>DLT_COMMON_ASCII_CHAR_TILDE) )
        {
            snprintf(*text,2,".");
        }
        else
        {
            /* replace < with . */
            if (((char*)ptr)[num]!=DLT_COMMON_ASCII_CHAR_LT)
            {
                snprintf(*text,2,"%c",((char *)ptr)[num]);
            }
            else
            {
                snprintf(*text,2,".");
            }
        }
        (*text)++;
    }

    return DLT_RETURN_OK;
}

void dlt_print_id(char *text, const char *id)
{
    // check nullpointer
    if(text == NULL || id == NULL)
        return;

    int i, len;

    /* Initialize text */
    for (i=0; i<DLT_ID_SIZE; i++)
    {
        text[i]='-';
    }

    text[DLT_ID_SIZE] = 0;

    len = ((strlen(id)<=DLT_ID_SIZE)?strlen(id):DLT_ID_SIZE);

    /* Check id*/
    for (i=0; i<len; i++)
        text[i] = id[i];
}

void dlt_set_id(char *id, const char *text)
{
    // check nullpointer
    if(id == NULL || text == NULL)
        return;

    id[0] = 0;
    id[1] = 0;
    id[2] = 0;
    id[3] = 0;

    if (text[0]!=0)
    {
        id[0] = text[0];
    }
    else
    {
        return;
    }

    if (text[1]!=0)
    {
        id[1] = text[1];
    }
    else
    {
        return;
    }

    if (text[2]!=0)
    {
        id[2] = text[2];
    }
    else
    {
        return;
    }

    if (text[3]!=0)
    {
        id[3] = text[3];
    }
    else
    {
        return;
    }
}

void dlt_clean_string(char *text, int length)
{
    int num;

    if (text == NULL)
    {
        return;
    }

    for (num=0;num<length;num++)
    {
        if (text[num]=='\r' || text[num]=='\n')
        {
            text[num] = ' ';
        }
    }
}

DltReturnValue dlt_filter_init(DltFilter *filter, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (filter == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    filter->counter = 0;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_filter_free(DltFilter *filter, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (filter == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_filter_load(DltFilter *filter, const char *filename, int verbose)
{
    if (filter == NULL || filename == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    FILE *handle;
    char str1[DLT_COMMON_BUFFER_LENGTH];
    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    handle = fopen(filename,"r");
    if (handle == NULL)
    {
        snprintf(str, DLT_COMMON_BUFFER_LENGTH, "Filter file %s cannot be opened!\n", filename);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    /* Reset filters */
    filter->counter=0;

    while (!feof(handle))
    {
        str1[0]=0;
        if (fscanf(handle,"%s",str1)!=1)
        {
            break;
        }
        if (str1[0]==0)
        {
            break;
        }
        printf(" %s",str1);
        if (strcmp(str1,"----")==0)
        {
            dlt_set_id(apid,"");
        }
        else
        {
            dlt_set_id(apid,str1);
        }

        str1[0]=0;
        if (fscanf(handle,"%s",str1)!=1)
        {
            break;
        }
        if (str1[0]==0)
        {
            break;
        }
        printf(" %s\r\n",str1);
        if (strcmp(str1,"----")==0)
        {
            dlt_set_id(ctid,"");
        }
        else
        {
            dlt_set_id(ctid,str1);
        }

        if (filter->counter<DLT_FILTER_MAX)
        {
            dlt_filter_add(filter,apid,ctid,verbose);
        }
        else
        {
            snprintf(str,DLT_COMMON_BUFFER_LENGTH, "Maximum number (%d) of allowed filters reached, ignoring rest of filters!\n", DLT_FILTER_MAX);
            dlt_log(LOG_WARNING, str);
        }
    }

    fclose(handle);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_filter_save(DltFilter *filter, const char *filename, int verbose)
{
    if (filter == NULL || filename == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    FILE *handle;
    int num;
    char buf[DLT_COMMON_BUFFER_LENGTH];

    PRINT_FUNCTION_VERBOSE(verbose);

    handle = fopen(filename,"w");
    if (handle == NULL)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Filter file %s cannot be opened!\n",filename);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    for (num=0;num<filter->counter;num++)
    {
        if (filter->apid[num][0]==0)
        {
            fprintf(handle,"---- ");
        }
        else
        {
            dlt_print_id(buf,filter->apid[num]);
            fprintf(handle,"%s ",buf);
        }
        if (filter->ctid[num][0]==0)
        {
            fprintf(handle,"---- ");
        }
        else
        {
            dlt_print_id(buf,filter->ctid[num]);
            fprintf(handle,"%s ",buf);
        }
    }

    fclose(handle);

    return DLT_RETURN_OK;
}

int dlt_filter_find(DltFilter *filter, const char *apid, const char *ctid, int verbose)
{
    int num;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((filter == NULL) || (apid == NULL))
    {
        return -1;
    }

    for (num=0; num<filter->counter; num++)
    {
        if (memcmp(filter->apid[num],apid,DLT_ID_SIZE)==0)
        {
            /* apid matches, now check for ctid */
            if (ctid == NULL)
            {
                /* check if empty ctid matches */
                //if (memcmp(filter->ctid[num],"",DLT_ID_SIZE)==0)//coverity complains here about Out-of-bounds access.
                char empty_ctid[DLT_ID_SIZE]="";
                if (memcmp(filter->ctid[num],empty_ctid,DLT_ID_SIZE)==0)
                {
                    return num;
                }
            }
            else
            {
                if (memcmp(filter->ctid[num],ctid,DLT_ID_SIZE)==0)
                {
                    return num;
                }
            }
        }
    }

    return -1; /* Not found */
}

DltReturnValue dlt_filter_add(DltFilter *filter,const char *apid, const char *ctid, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((filter == NULL) || (apid == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (filter->counter >= DLT_FILTER_MAX)
    {
        sprintf(str, "Maximum number (%d) of allowed filters reached, ignoring filter!\n", DLT_FILTER_MAX);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    /* add each filter (apid, ctid) only once to filter array */
    if (dlt_filter_find(filter,apid,ctid,verbose)<0)
    {
        /* filter not found, so add it to filter array */
        if (filter->counter<DLT_FILTER_MAX)
        {
            dlt_set_id(filter->apid[filter->counter],apid);
            dlt_set_id(filter->ctid[filter->counter],(ctid?ctid:""));

            filter->counter++;

            return DLT_RETURN_OK;
        }
    }

    return DLT_RETURN_ERROR;
}

DltReturnValue dlt_filter_delete(DltFilter *filter,const char *apid,const char *ctid, int verbose)
{
    int j,k;
    int found=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((filter == NULL) || (apid == NULL) || (ctid == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (filter->counter>0)
    {
        /* Get first occurence of apid and ctid in filter array */
        for (j=0; j<filter->counter; j++)
        {
            if ((memcmp(filter->apid[j],apid,DLT_ID_SIZE)==0) &&
                (memcmp(filter->ctid[j],ctid,DLT_ID_SIZE)==0)
               )

            {
                found=1;
                break;
            }
        }

        if (found)
        {
            /* j is index */
            /* Copy from j+1 til end to j til end-1 */

            dlt_set_id(filter->apid[j],"");
            dlt_set_id(filter->ctid[j],"");

            for (k=j; k<(filter->counter-1); k++)
            {
                dlt_set_id(filter->apid[k],filter->apid[k+1]);
                dlt_set_id(filter->ctid[k],filter->ctid[k+1]);
            }

            filter->counter--;
            return DLT_RETURN_OK;
        }
    }

    return DLT_RETURN_ERROR;
}

DltReturnValue dlt_message_init(DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* initalise structure parameters */
    msg->headersize = 0;
    msg->datasize = 0;

    msg->databuffer = NULL;
    msg->databuffersize = 0;

    msg->storageheader = NULL;
    msg->standardheader = NULL;
    msg->extendedheader = NULL;

    msg->found_serialheader = 0;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_free(DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }
    /* delete databuffer if exists */
    if (msg->databuffer)
    {
        free(msg->databuffer);
        msg->databuffer = NULL;
        msg->databuffersize = 0;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_header(DltMessage *msg, char *text, int textlength, int verbose)
{
    return dlt_message_header_flags(msg, text, textlength, DLT_HEADER_SHOW_ALL, verbose);
}

DltReturnValue dlt_message_header_flags(DltMessage *msg, char *text, int textlength, int flags, int verbose)
{
    struct tm * timeinfo;
    char buffer [DLT_COMMON_BUFFER_LENGTH];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg == NULL) || (text == NULL) || (textlength <= 0))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if ((flags<DLT_HEADER_SHOW_NONE) || (flags>DLT_HEADER_SHOW_ALL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    text[0] = 0;

    if ((flags & DLT_HEADER_SHOW_TIME) == DLT_HEADER_SHOW_TIME)
    {
        /* print received time */
        time_t tt = msg->storageheader->seconds;
        timeinfo = localtime (&tt);

        if (timeinfo != NULL)
        {
            strftime (buffer,sizeof(buffer),"%Y/%m/%d %H:%M:%S",timeinfo);
            snprintf(text,textlength,"%s.%.6d ",buffer,msg->storageheader->microseconds);
        }
    }

    if ((flags & DLT_HEADER_SHOW_TMSTP) == DLT_HEADER_SHOW_TMSTP)
    {
        /* print timestamp if available */
        if ( DLT_IS_HTYP_WTMS(msg->standardheader->htyp) )
        {
            snprintf(text+strlen(text),textlength-strlen(text),"%10u ",msg->headerextra.tmsp);
        }
        else
        {
            snprintf(text+strlen(text),textlength-strlen(text),"---------- ");
        }
    }

    if ((flags & DLT_HEADER_SHOW_MSGCNT) == DLT_HEADER_SHOW_MSGCNT)
    {
        /* print message counter */
        snprintf(text+strlen(text),textlength-strlen(text),"%.3d ",msg->standardheader->mcnt);
    }

    if ((flags & DLT_HEADER_SHOW_ECUID) == DLT_HEADER_SHOW_ECUID)
    {
        /* print ecu id, use header extra if available, else storage header value */
        if ( DLT_IS_HTYP_WEID(msg->standardheader->htyp) )
        {
            dlt_print_id(text+strlen(text),msg->headerextra.ecu);
        }
        else
        {
            dlt_print_id(text+strlen(text),msg->storageheader->ecu);
        }
    }

    /* print app id and context id if extended header available, else '----' */#
    if ((flags & DLT_HEADER_SHOW_APID) == DLT_HEADER_SHOW_APID)
    {
        snprintf(text+strlen(text),textlength-strlen(text)," ");
        if ((DLT_IS_HTYP_UEH(msg->standardheader->htyp)) && (msg->extendedheader->apid[0]!=0))
        {
            dlt_print_id(text+strlen(text),msg->extendedheader->apid);
        }
        else
        {
            snprintf(text+strlen(text),textlength-strlen(text),"----");
        }

        snprintf(text+strlen(text),textlength-strlen(text)," ");
    }

    if ((flags & DLT_HEADER_SHOW_CTID) == DLT_HEADER_SHOW_CTID)
    {
        if ((DLT_IS_HTYP_UEH(msg->standardheader->htyp)) && (msg->extendedheader->ctid[0]!=0))
        {
            dlt_print_id(text+strlen(text),msg->extendedheader->ctid);
        }
        else
        {
            snprintf(text+strlen(text),textlength-strlen(text),"----");
        }

        snprintf(text+strlen(text),textlength-strlen(text)," ");
    }

    /* print info about message type and length */
    if (DLT_IS_HTYP_UEH(msg->standardheader->htyp))
    {
        if ((flags & DLT_HEADER_SHOW_MSGTYPE) == DLT_HEADER_SHOW_MSGTYPE)
        {
            snprintf(text+strlen(text),textlength-strlen(text),"%s",message_type[DLT_GET_MSIN_MSTP(msg->extendedheader->msin)]);
            snprintf(text+strlen(text),textlength-strlen(text)," ");
        }

        if ((flags & DLT_HEADER_SHOW_MSGSUBTYPE) == DLT_HEADER_SHOW_MSGSUBTYPE)
        {
            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_LOG)
            {
                snprintf(text+strlen(text),textlength-strlen(text),"%s",log_info[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_APP_TRACE)
            {
                snprintf(text+strlen(text),textlength-strlen(text),"%s",trace_type[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_NW_TRACE)
            {
                snprintf(text+strlen(text),textlength-strlen(text),"%s",nw_trace_type[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_CONTROL)
            {
                snprintf(text+strlen(text),textlength-strlen(text),"%s",control_type[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            snprintf(text+strlen(text),textlength-strlen(text)," ");
        }

        if ((flags & DLT_HEADER_SHOW_VNVSTATUS) == DLT_HEADER_SHOW_VNVSTATUS)
        {
            /* print verbose status pf message */
            if (DLT_IS_MSIN_VERB(msg->extendedheader->msin))
            {
                snprintf(text+strlen(text),textlength-strlen(text),"V");
            }
            else
            {
                snprintf(text+strlen(text),textlength-strlen(text),"N");
            }

            snprintf(text+strlen(text),textlength-strlen(text)," ");
        }

        if ((flags & DLT_HEADER_SHOW_NOARG) == DLT_HEADER_SHOW_NOARG)
        {
            /* print number of arguments */
            snprintf(text+strlen(text),textlength-strlen(text),"%d", msg->extendedheader->noar);
        }

    }
    else
    {
        if ((flags & DLT_HEADER_SHOW_MSGTYPE) == DLT_HEADER_SHOW_MSGTYPE)
        {
            snprintf(text+strlen(text),textlength-strlen(text),"--- ");
        }

        if ((flags & DLT_HEADER_SHOW_MSGSUBTYPE) == DLT_HEADER_SHOW_MSGSUBTYPE)
        {
            snprintf(text+strlen(text),textlength-strlen(text),"--- ");
        }

        if ((flags & DLT_HEADER_SHOW_VNVSTATUS) == DLT_HEADER_SHOW_VNVSTATUS)
        {
            snprintf(text+strlen(text),textlength-strlen(text),"N ");
        }

        if ((flags & DLT_HEADER_SHOW_NOARG) == DLT_HEADER_SHOW_NOARG)
        {
            snprintf(text+strlen(text),textlength-strlen(text),"-");
        }
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_payload(DltMessage *msg, char *text, int textlength, int type, int verbose)
{
    uint32_t id=0, id_tmp=0;
    uint8_t retval=0;

    uint8_t *ptr;
    int32_t datalength;

    /* Pointer to ptr and datalength */
    uint8_t **pptr;
    int32_t *pdatalength;

    int ret=0;

    int num;
    uint32_t type_info=0,type_info_tmp=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg == NULL) || (text == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (textlength <= 0)
    {
        dlt_log(LOG_WARNING, "String does not fit binary data!\n");
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* start with empty string */
    text[0] = 0;

    /* print payload only as hex */
    if (type==DLT_OUTPUT_HEX)
    {
        return dlt_print_hex_string(text, textlength, msg->databuffer, msg->datasize);
    }

    /* print payload as mixed */
    if (type==DLT_OUTPUT_MIXED_FOR_PLAIN)
    {
        return dlt_print_mixed_string(text, textlength, msg->databuffer, msg->datasize,0);
    }

    if (type==DLT_OUTPUT_MIXED_FOR_HTML)
    {
        return dlt_print_mixed_string(text, textlength, msg->databuffer, msg->datasize,1);
    }

    ptr = msg->databuffer;
    datalength = msg->datasize;

    /* Pointer to ptr and datalength */
    pptr = &ptr;
    pdatalength = &datalength;

    /* non-verbose mode */

    /* print payload as hex */
    if (DLT_MSG_IS_NONVERBOSE(msg))
    {

        DLT_MSG_READ_VALUE(id_tmp,ptr,datalength,uint32_t);
        id=DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

        if (textlength<((datalength*3)+20))
        {
            char str[255];
            snprintf(str, 254, "String does not fit binary data (available=%d, required=%d) !\n", textlength, (datalength*3)+20);
            dlt_log(LOG_WARNING, str);
            return DLT_RETURN_ERROR;
        }

        /* process message id / service id */
        if (DLT_MSG_IS_CONTROL(msg))
        {
            if (id > 0 && id < DLT_SERVICE_ID_LAST_ENTRY)
            {
                snprintf(text+strlen(text),textlength-strlen(text),"%s",service_id[id]); /* service id */
            }
            else
            {
                if (!(DLT_MSG_IS_CONTROL_TIME(msg)))
                {
                    snprintf(text+strlen(text),textlength-strlen(text),"service(%u)",id); /* service id */
                }
            }

            if (datalength>0)
            {
                snprintf(text+strlen(text),textlength-strlen(text),", ");
            }
        }
        else
        {
            snprintf(text+strlen(text),textlength-strlen(text),"%u, ",id); /* message id */
        }

        /* process return value */
        if (DLT_MSG_IS_CONTROL_RESPONSE(msg))
        {
            if (datalength>0)
            {
                DLT_MSG_READ_VALUE(retval,ptr,datalength,uint8_t); /* No endian conversion necessary */
                if ( (retval<3) || (retval==8))
                {
                    snprintf(text+strlen(text),textlength-strlen(text),"%s",return_type[retval]);
                }
                else
                {
                    snprintf(text+strlen(text),textlength-strlen(text),"%.2x",retval);
                }

                if (datalength>=1)
                {
                    snprintf(text+strlen(text),textlength-strlen(text),", ");
                }
            }
        }

        if (type==DLT_OUTPUT_ASCII_LIMITED)
        {
            ret=dlt_print_hex_string(text+strlen(text),textlength-strlen(text),ptr,
                                     (datalength>DLT_COMMON_ASCII_LIMIT_MAX_CHARS?DLT_COMMON_ASCII_LIMIT_MAX_CHARS:datalength));
            if ((datalength>DLT_COMMON_ASCII_LIMIT_MAX_CHARS) &&
                    ((textlength-strlen(text))>4))
            {
                snprintf(text+strlen(text),textlength-strlen(text)," ...");
            }
        }
        else
        {
            ret=dlt_print_hex_string(text+strlen(text),textlength-strlen(text),ptr,datalength);
        }

        return ret;
    }

    /* At this point, it is ensured that a extended header is available */

    /* verbose mode */
    type_info=0;
    type_info_tmp=0;

    for (num=0;num<(int)(msg->extendedheader->noar);num++)
    {
        if (num!=0)
        {
            snprintf(text+strlen(text),textlength-strlen(text)," ");
        }

        /* first read the type info of the argument */
        DLT_MSG_READ_VALUE(type_info_tmp,ptr,datalength,uint32_t);
        type_info=DLT_ENDIAN_GET_32(msg->standardheader->htyp, type_info_tmp);

        /* print out argument */
        if (dlt_message_argument_print(msg, type_info, pptr, pdatalength, text, textlength, -1, 0) == DLT_RETURN_ERROR)
        {
            return DLT_RETURN_ERROR;
        }
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_filter_check(DltMessage *msg, DltFilter *filter, int verbose)
{
    /* check the filters if message is used */
    int num;
    DltReturnValue found = DLT_RETURN_OK;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg == NULL) || (filter == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if ((filter->counter==0) || (!(DLT_IS_HTYP_UEH(msg->standardheader->htyp))))
    {
        /* no filter is set, or no extended header is available, so do as filter is matching */
        return DLT_RETURN_TRUE;
    }

    for (num=0 ;num<filter->counter; num++)
    {
        /* check each filter if it matches */
        if ((DLT_IS_HTYP_UEH(msg->standardheader->htyp)) &&
                (filter->apid[num][0]==0 || memcmp(filter->apid[num],msg->extendedheader->apid,DLT_ID_SIZE)==0) &&
                (filter->ctid[num][0]==0 || memcmp(filter->ctid[num],msg->extendedheader->ctid,DLT_ID_SIZE)==0) )
        {
            found = DLT_RETURN_TRUE;
            break;
        }
    }

    return found;
}

int dlt_message_read(DltMessage *msg, uint8_t *buffer, unsigned int length, int resync, int verbose)
{
    int extra_size = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg == NULL) || (buffer == NULL) || (length <= 0))
    {
        return DLT_MESSAGE_ERROR_UNKNOWN;
    }

    /* initialize resync_offset */
    msg->resync_offset=0;

    /* check if message contains serial header, smaller than standard header */
    if (length<sizeof(dltSerialHeader))
    {
        /* dlt_log(LOG_ERR, "Length smaller than serial header!\n"); */
        return DLT_MESSAGE_ERROR_SIZE;
    }

    if (memcmp(buffer, dltSerialHeader, sizeof(dltSerialHeader)) == 0)
    {
        /* serial header found */
        msg->found_serialheader = 1;
        buffer += sizeof(dltSerialHeader);
        length -= sizeof(dltSerialHeader);
    }
    else
    {
        /* serial header not found */
        msg->found_serialheader = 0;
        if (resync)
        {
            /* resync if necessary */
            msg->resync_offset=0;

            do
            {
                if (memcmp(buffer+msg->resync_offset, dltSerialHeader, sizeof(dltSerialHeader)) == 0)
                {
                    /* serial header found */
                    msg->found_serialheader = 1;
                    buffer += sizeof(dltSerialHeader);
                    length -= sizeof(dltSerialHeader);
                    break;
                }

                msg->resync_offset++;
            }
            while ((sizeof(dltSerialHeader)+msg->resync_offset)<=length);

            /* Set new start offset */
            if (msg->resync_offset>0)
            {
                /* Resyncing connection */
                buffer+=msg->resync_offset;
                length-=msg->resync_offset;
            }
        }
    }

    /* check that standard header fits buffer */
    if (length<sizeof(DltStandardHeader))
    {
        /* dlt_log(LOG_ERR, "Length smaller than standard header!\n"); */
        return DLT_MESSAGE_ERROR_SIZE;
    }
    memcpy(msg->headerbuffer+sizeof(DltStorageHeader), buffer, sizeof(DltStandardHeader));

    /* set ptrs to structures */
    msg->storageheader = (DltStorageHeader*) msg->headerbuffer;
    msg->standardheader = (DltStandardHeader*) (msg->headerbuffer + sizeof(DltStorageHeader));

    /* calculate complete size of headers */
    extra_size = DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp)+(DLT_IS_HTYP_UEH(msg->standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    msg->headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + extra_size;
    msg->datasize =  DLT_BETOH_16(msg->standardheader->len) - (msg->headersize - sizeof(DltStorageHeader));

    if (verbose)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH, "BufferLength=%d, HeaderSize=%d, DataSize=%d\n", length, msg->headersize, msg->datasize);
        dlt_log(LOG_DEBUG, str);
    }

    /* check data size */
    if (msg->datasize < 0)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH, "Plausibility check failed. Complete message size too short (%d)!\n",msg->datasize);
        dlt_log(LOG_WARNING, str);
        return DLT_MESSAGE_ERROR_CONTENT;
    }

    /* load standard header extra parameters and Extended header if used */
    if (extra_size>0)
    {
        if (length  < (msg->headersize - sizeof(DltStorageHeader)))
        {
            return DLT_MESSAGE_ERROR_SIZE;
        }

        memcpy(msg->headerbuffer+sizeof(DltStorageHeader)+sizeof(DltStandardHeader),buffer+sizeof(DltStandardHeader),extra_size);

        /* set extended header ptr and get standard header extra parameters */
        if (DLT_IS_HTYP_UEH(msg->standardheader->htyp))
        {
            msg->extendedheader = (DltExtendedHeader*) (msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                                  DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp));
        }
        else
        {
            msg->extendedheader = NULL;
        }

        dlt_message_get_extraparameters(msg,verbose);
    }

    /* check if payload fits length */
    if (length  < (msg->headersize - sizeof(DltStorageHeader) + msg->datasize))
    {
        /* dlt_log(LOG_ERR,"length does not fit!\n"); */
        return DLT_MESSAGE_ERROR_SIZE;
    }

    /* free last used memory for buffer */
    if (msg->databuffer)
    {
        if (msg->datasize>msg->databuffersize){
            free(msg->databuffer);
            msg->databuffer=(uint8_t *)malloc(msg->datasize);
            msg->databuffersize = msg->datasize;
        }
    }else{
        /* get new memory for buffer */
        msg->databuffer = (uint8_t *)malloc(msg->datasize);
        msg->databuffersize = msg->datasize;
    }
    if (msg->databuffer == NULL)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Cannot allocate memory for payload buffer of size %d!\n",msg->datasize);
        dlt_log(LOG_WARNING, str);
        return DLT_MESSAGE_ERROR_UNKNOWN;
    }

    /* load payload data from buffer */
    memcpy(msg->databuffer,buffer+(msg->headersize-sizeof(DltStorageHeader)),msg->datasize);

    return DLT_MESSAGE_ERROR_OK;
}

DltReturnValue dlt_message_get_extraparameters(DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (DLT_IS_HTYP_WEID(msg->standardheader->htyp))
    {
        memcpy(msg->headerextra.ecu,msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader),DLT_ID_SIZE);
    }

    if (DLT_IS_HTYP_WSID(msg->standardheader->htyp))
    {
        memcpy(&(msg->headerextra.seid),msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0), DLT_SIZE_WSID);
        msg->headerextra.seid = DLT_BETOH_32(msg->headerextra.seid);
    }

    if (DLT_IS_HTYP_WTMS(msg->standardheader->htyp))
    {
        memcpy(&(msg->headerextra.tmsp),msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0)
               + (DLT_IS_HTYP_WSID(msg->standardheader->htyp) ? DLT_SIZE_WSID : 0),DLT_SIZE_WTMS);
        msg->headerextra.tmsp = DLT_BETOH_32(msg->headerextra.tmsp);
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_set_extraparameters(DltMessage *msg, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (DLT_IS_HTYP_WEID(msg->standardheader->htyp))
    {
        memcpy(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader),msg->headerextra.ecu,DLT_ID_SIZE);
    }

    if (DLT_IS_HTYP_WSID(msg->standardheader->htyp))
    {
        msg->headerextra.seid = DLT_HTOBE_32(msg->headerextra.seid);
        memcpy(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0), &(msg->headerextra.seid),DLT_SIZE_WSID);
    }

    if (DLT_IS_HTYP_WTMS(msg->standardheader->htyp))
    {
        msg->headerextra.tmsp = DLT_HTOBE_32(msg->headerextra.tmsp);
        memcpy(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0)
               + (DLT_IS_HTYP_WSID(msg->standardheader->htyp) ? DLT_SIZE_WSID : 0), &(msg->headerextra.tmsp),DLT_SIZE_WTMS);
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_init(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* initalise structure parameters */
    file->handle = NULL;
    file->counter = 0;
    file->counter_total = 0;
    file->index = NULL;

    file->filter = NULL;
    file->filter_counter = 0;
    file->file_position = 0;

    file->position = 0;

    file->error_messages = 0;

    return dlt_message_init(&(file->msg),verbose);
}

DltReturnValue dlt_file_set_filter(DltFile *file, DltFilter *filter, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* set filter */
    file->filter = filter;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_read_header(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* load header from file */
    if (fread(file->msg.headerbuffer, sizeof(DltStorageHeader)+sizeof(DltStandardHeader), 1, file->handle)!=1)
    {
        if (!feof(file->handle))
        {
            dlt_log(LOG_WARNING, "Cannot read header from file!\n");
        }
        return DLT_RETURN_ERROR;
    }

    /* set ptrs to structures */
    file->msg.storageheader = (DltStorageHeader*) file->msg.headerbuffer;
    file->msg.standardheader = (DltStandardHeader*) (file->msg.headerbuffer + sizeof(DltStorageHeader));

    /* check id of storage header */
    if (dlt_check_storageheader(file->msg.storageheader) != DLT_RETURN_TRUE)
    {
        dlt_log(LOG_WARNING, "DLT storage header pattern not found!\n");
        return DLT_RETURN_ERROR;
    }

    /* calculate complete size of headers */
    file->msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                           DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp) + (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    file->msg.datasize = DLT_BETOH_16(file->msg.standardheader->len) + sizeof(DltStorageHeader) - file->msg.headersize;
    if (verbose)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"HeaderSize=%d, DataSize=%d\n",file->msg.headersize, file->msg.datasize);
        dlt_log(LOG_DEBUG, str);
    }

    /* check data size */
    if (file->msg.datasize < 0)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Plausibility check failed. Complete message size too short! (%d)\n", file->msg.datasize);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_read_header_raw(DltFile *file, int resync, int verbose)
{
    char dltSerialHeaderBuffer[DLT_ID_SIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* check if serial header exists, ignore if found */
    if (fread(dltSerialHeaderBuffer,sizeof(dltSerialHeaderBuffer),1,file->handle)!=1)
    {
        /* cannot read serial header, not enough data available in file */
        if (!feof(file->handle))
        {
            dlt_log(LOG_WARNING, "Cannot read header from file!\n");
        }
        return DLT_RETURN_ERROR;
    }
    if (memcmp(dltSerialHeaderBuffer,dltSerialHeader,sizeof(dltSerialHeader)) == 0)
    {
        /* serial header found */
        /* nothing to do continue reading */

    }
    else
    {
        /* serial header not found */
        if(resync)
        {
            /* increase error counter */
            file->error_messages++;

            /* resync to serial header */
            do
            {
                memmove(dltSerialHeaderBuffer,dltSerialHeaderBuffer+1,sizeof(dltSerialHeader)-1);
                if (fread(dltSerialHeaderBuffer+3,1,1,file->handle)!=1)
                {
                    /* cannot read any data, perhaps end of file reached */
                    return DLT_RETURN_ERROR;
                }
                if (memcmp(dltSerialHeaderBuffer,dltSerialHeader,sizeof(dltSerialHeader)) == 0)
                {
                    /* serial header synchronised */
                    break;
                }
            } while(1);
        }
        else
        {
            /* go back to last file position */
            if (0 != fseek(file->handle,file->file_position,SEEK_SET))
                return DLT_RETURN_ERROR;
        }
    }

    /* load header from file */
    if (fread(file->msg.headerbuffer+sizeof(DltStorageHeader),sizeof(DltStandardHeader),1,file->handle)!=1)
    {
        if (!feof(file->handle))
        {
            dlt_log(LOG_WARNING, "Cannot read header from file!\n");
        }
        return DLT_RETURN_ERROR;
    }

    /* set ptrs to structures */
    file->msg.storageheader = (DltStorageHeader*) file->msg.headerbuffer; // this points now to a empty storage header (filled with '0')
    file->msg.standardheader = (DltStandardHeader*) (file->msg.headerbuffer + sizeof(DltStorageHeader));

    /* Skip storage header field, fill this field with '0' */
    memset(file->msg.storageheader, 0, sizeof(DltStorageHeader));

    /* Set storage header */
    dlt_set_storageheader(file->msg.storageheader,DLT_COMMON_DUMMY_ECUID);

    /* no check for storage header id*/

    /* calculate complete size of headers */
    file->msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                           DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp) + (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    file->msg.datasize = DLT_BETOH_16(file->msg.standardheader->len) + sizeof(DltStorageHeader) - file->msg.headersize;
    if (verbose)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"HeaderSize=%d, DataSize=%d\n",file->msg.headersize, file->msg.datasize);
        dlt_log(LOG_DEBUG, str);
    }

    /* check data size */
    if (file->msg.datasize < 0)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Plausibility check failed. Complete message size too short! (%d)\n", file->msg.datasize);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_read_header_extended(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* load standard header extra parameters if used */
    if (DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp))
    {
        if (fread(file->msg.headerbuffer+sizeof(DltStorageHeader)+sizeof(DltStandardHeader),
                  DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp),
                  1,file->handle)!=1)
        {
            dlt_log(LOG_WARNING, "Cannot read standard header extra parameters from file!\n");
            return DLT_RETURN_ERROR;
        }

        dlt_message_get_extraparameters(&(file->msg),verbose);
    }

    /* load Extended header if used */
    if (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp)==0)
    {
        /* there is nothing to be loaded */
        return DLT_RETURN_OK;
    }

    if (fread(file->msg.headerbuffer+sizeof(DltStorageHeader)+sizeof(DltStandardHeader)+DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp),
              (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0),
              1,file->handle)!=1)
    {
        dlt_log(LOG_WARNING, "Cannot read extended header from file!\n");
        return DLT_RETURN_ERROR;
    }

    /* set extended header ptr */
    if (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp))
    {
        file->msg.extendedheader = (DltExtendedHeader*) (file->msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                                   DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp));
    }
    else
    {
        file->msg.extendedheader = NULL;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_read_data(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* free last used memory for buffer */
    if (file->msg.databuffer && (file->msg.databuffersize < file->msg.datasize))
    {
        free(file->msg.databuffer);
        file->msg.databuffer = NULL;
    }

    if (file->msg.databuffer == NULL){
        /* get new memory for buffer */
        file->msg.databuffer = (uint8_t *)malloc(file->msg.datasize);
        file->msg.databuffersize = file->msg.datasize;
    }

    if (file->msg.databuffer == NULL)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Cannot allocate memory for payload buffer of size %d!\n",file->msg.datasize);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    /* load payload data from file */
    if (fread(file->msg.databuffer,file->msg.datasize,1,file->handle)!=1)
    {
        if (file->msg.datasize!=0)
        {
            snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Cannot read payload data from file of size %d!\n",file->msg.datasize);
            dlt_log(LOG_WARNING, str);
            return DLT_RETURN_ERROR;
        }
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_open(DltFile *file,const char *filename,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if(file == NULL || filename == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* reset counters */
    file->counter = 0;
    file->counter_total = 0;
    file->position = 0;
    file->file_position = 0;
    file->file_length = 0;
    file->error_messages = 0;

    if (file->handle)
    {
        fclose(file->handle);
    }

    /* open dlt file */
    file->handle = fopen(filename,"rb");
    if (file->handle == NULL)
    {
        snprintf(str, DLT_COMMON_BUFFER_LENGTH - 1 ,"File %s cannot be opened!\n", filename);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    if (0 != fseek(file->handle, 0, SEEK_END))
    {
        snprintf(str, DLT_COMMON_BUFFER_LENGTH, "dlt_file_open: Seek failed to 0,SEEK_END");
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }
    file->file_length = ftell(file->handle);

    if (0 != fseek(file->handle,0,SEEK_SET))
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"dlt_file_open: Seek failed to 0,SEEK_SET");
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    if (verbose)
    {
        /* print file length */
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"File is %lu bytes long\n",file->file_length);
        dlt_log(LOG_DEBUG, str);
    }
    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_read(DltFile *file,int verbose)
{
    long *ptr;
    int found = DLT_RETURN_OK;

    if (verbose)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"%s: Message %d:\n",__func__, file->counter_total);
        dlt_log(LOG_DEBUG, str);
    }

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* allocate new memory for index if number of messages exceeds a multiple of DLT_COMMON_INDEX_ALLOC (e.g.: 1000) */
    if (file->counter % DLT_COMMON_INDEX_ALLOC == 0)
    {
        ptr = (long *) malloc(((file->counter/DLT_COMMON_INDEX_ALLOC) + 1) * DLT_COMMON_INDEX_ALLOC * sizeof(long));

        if (ptr == NULL)
        {
            return DLT_RETURN_ERROR;
        }

        if (file->index)
        {
            memcpy(ptr,file->index,file->counter * sizeof(long));
            free(file->index);
        }
        file->index = ptr;
    }

    /* set to end of last succesful read message, because of conflicting calls to dlt_file_read and dlt_file_message */
    if (0 !=  fseek(file->handle,file->file_position,SEEK_SET))
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek failed to file_position %ld \n",file->file_position);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    /* get file position at start of DLT message */
    if (verbose)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Position in file: %ld\n",file->file_position);
        dlt_log(LOG_INFO, str);
    }

    /* read header */
    if (dlt_file_read_header(file,verbose) < DLT_RETURN_OK)
    {
        /* go back to last position in file */
        fseek(file->handle,file->file_position,SEEK_SET);
        return DLT_RETURN_ERROR;
    }

    if (file->filter)
    {
        /* read the extended header if filter is enabled and extended header exists */
        if (dlt_file_read_header_extended(file, verbose) < DLT_RETURN_OK)
        {
            /* go back to last position in file */
            if (0 != fseek(file->handle,file->file_position,SEEK_SET))
            {
                snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek to last file pos failed!\n");
                dlt_log(LOG_WARNING, str);
            }
            return DLT_RETURN_ERROR;
        }

        /* check the filters if message is used */
        if (dlt_message_filter_check(&(file->msg),file->filter,verbose) == DLT_RETURN_TRUE)
        {
            /* filter matched, consequently store current message */
            /* store index pointer to message position in DLT file */
            file->index[file->counter] = file->file_position;
            file->counter++;
            file->position = file->counter - 1;

            found = DLT_RETURN_TRUE;
        }

        /* skip payload data */
        if (fseek(file->handle,file->msg.datasize,SEEK_CUR)!=0)
        {
            /* go back to last position in file */
            snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek failed to skip payload data of size %d!\n",file->msg.datasize);
            dlt_log(LOG_WARNING, str);

            if (0 != fseek(file->handle,file->file_position,SEEK_SET))
            {
                snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek back also failed!\n");
                dlt_log(LOG_WARNING, str);
            }

            return DLT_RETURN_ERROR;
        }
    }
    else
    {
        /* filter is disabled */
        /* skip additional header parameters and payload data */
        if (fseek(file->handle,file->msg.headersize - sizeof(DltStorageHeader) - sizeof(DltStandardHeader) + file->msg.datasize,SEEK_CUR))
        {

            snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek failed to skip extra header and payload data from file of size %d!\n",
                    file->msg.headersize - (int32_t)sizeof(DltStorageHeader) - (int32_t)sizeof(DltStandardHeader) + file->msg.datasize);
            dlt_log(LOG_WARNING, str);
            /* go back to last position in file */
            if (fseek(file->handle,file->file_position,SEEK_SET))
            {
                snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek back also failed!\n");
                dlt_log(LOG_WARNING, str);
            }
            return DLT_RETURN_ERROR;
        }

        /* store index pointer to message position in DLT file */
        file->index[file->counter] = file->file_position;
        file->counter++;
        file->position = file->counter - 1;

        found = DLT_RETURN_TRUE;
    }

    /* increase total message counter */
    file->counter_total++;

    /* store position to next message */
    file->file_position = ftell(file->handle);

    return found;
}

DltReturnValue dlt_file_read_raw(DltFile *file, int resync, int verbose)
{
    int found = DLT_RETURN_OK;
    long *ptr;

    if (verbose)
    {
        snprintf(str, DLT_COMMON_BUFFER_LENGTH, "%s: Message %d:\n", __func__, file->counter_total);
        dlt_log(LOG_DEBUG, str);
    }

    if (file == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    /* allocate new memory for index if number of messages exceeds a multiple of DLT_COMMON_INDEX_ALLOC (e.g.: 1000) */
    if (file->counter % DLT_COMMON_INDEX_ALLOC == 0)
    {
        ptr = (long *) malloc(((file->counter/DLT_COMMON_INDEX_ALLOC) + 1) * DLT_COMMON_INDEX_ALLOC * sizeof(long));

        if (ptr == NULL)
        {
            return DLT_RETURN_ERROR;
        }

        if (file->index)
        {
            memcpy(ptr,file->index,file->counter * sizeof(long));
            free(file->index);
        }
        file->index = ptr;
    }

    /* set to end of last successful read message, because of conflicting calls to dlt_file_read and dlt_file_message */
    if (0 != fseek(file->handle,file->file_position,SEEK_SET))
        return DLT_RETURN_ERROR;

    /* get file position at start of DLT message */
    if (verbose)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Position in file: %ld\n",file->file_position);
        dlt_log(LOG_DEBUG, str);
    }

    /* read header */
    if (dlt_file_read_header_raw(file,resync,verbose) < DLT_RETURN_OK)
    {
        /* go back to last position in file */
        if (0!= fseek(file->handle,file->file_position,SEEK_SET))
        {
            snprintf(str,DLT_COMMON_BUFFER_LENGTH,"dlt_file_read_raw, fseek failed 1\n");
            dlt_log(LOG_WARNING, str);
        }
        return DLT_RETURN_ERROR;
    }

    /* read the extended header if filter is enabled and extended header exists */
    if (dlt_file_read_header_extended(file, verbose) < DLT_RETURN_OK)
    {
        /* go back to last position in file */
        if (0 != fseek(file->handle,file->file_position,SEEK_SET))
        {
            snprintf(str,DLT_COMMON_BUFFER_LENGTH,"dlt_file_read_raw, fseek failed 2\n");
            dlt_log(LOG_WARNING, str);
        }
        return DLT_RETURN_ERROR;
    }

    if (dlt_file_read_data(file,verbose) < DLT_RETURN_OK)
    {
        /* go back to last position in file */
        if (0 != fseek(file->handle,file->file_position,SEEK_SET))
        {
            snprintf(str,DLT_COMMON_BUFFER_LENGTH,"dlt_file_read_raw, fseek failed 3\n");
            dlt_log(LOG_WARNING, str);
        }
        return DLT_RETURN_ERROR;
    }

    /* store index pointer to message position in DLT file */
    file->index[file->counter] = file->file_position;
    file->counter++;
    file->position = file->counter - 1;

    found = DLT_RETURN_TRUE;

    /* increase total message counter */
    file->counter_total++;

    /* store position to next message */
    file->file_position = ftell(file->handle);

    return found;
}

DltReturnValue dlt_file_close(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (file->handle)
    {
        fclose(file->handle);
    }

    file->handle = NULL;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_message(DltFile *file, int index, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* check if message is in range */
    if (index >= file->counter)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH, "Message %d out of range!\r\n", index);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* seek to position in file */
    if (fseek(file->handle,file->index[index],SEEK_SET)!=0)
    {
        snprintf(str,DLT_COMMON_BUFFER_LENGTH,"Seek to message %d to position %ld failed!\r\n",index,file->index[index]);
        dlt_log(LOG_WARNING, str);
        return DLT_RETURN_ERROR;
    }

    /* read all header and payload */
    if (dlt_file_read_header(file,verbose) < DLT_RETURN_OK)
    {
        return DLT_RETURN_ERROR;
    }

    if (dlt_file_read_header_extended(file,verbose) < DLT_RETURN_OK)
    {
        return DLT_RETURN_ERROR;
    }

    if (dlt_file_read_data(file,verbose) < DLT_RETURN_OK)
    {
        return DLT_RETURN_ERROR;
    }

    /* set current position in file */
    file->position = index;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_file_free(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* delete index lost if exists */
    if (file->index)
    {
        free(file->index);
    }
    file->index = NULL;

    /* close file */
    if (file->handle)
    {
        fclose(file->handle);
    }
    file->handle = NULL;

    return dlt_message_free(&(file->msg),verbose);
}

void dlt_log_set_level(int level)
{
    if(level < 0 || level > LOG_DEBUG)
    {
        if(logging_level < LOG_WARNING)
            logging_level = LOG_WARNING;

        dlt_vlog(LOG_WARNING, "Wrong parameter for level: %d\n", level);
    }
    else
        logging_level = level;
}

void dlt_log_set_filename(const char *filename)
{
    // check nullpointer
    if(filename == NULL)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: filename is NULL\n");
        return;
    }

    strncpy(logging_filename, filename, NAME_MAX);
    logging_filename[NAME_MAX]=0;
}

void dlt_log_set_fifo_basedir(const char * env_pipe_dir)
{
    strncpy(dltFifoBaseDir, env_pipe_dir, PATH_MAX);
    dltFifoBaseDir[PATH_MAX] = 0;
}

void dlt_log_init(int mode)
{
    if(mode < DLT_LOG_TO_CONSOLE || mode > DLT_LOG_DROPPED)
    {
        dlt_vlog(LOG_WARNING, "Wrong parameter for mode: %d\n", mode);
        return;
    }

    logging_mode = mode;

    if(logging_mode == DLT_LOG_TO_FILE)
    {
        /* internal logging to file */
        logging_handle = fopen(logging_filename,"w");
        if (logging_handle == NULL)
        {
            printf("Internal log file %s cannot be opened!\n", logging_filename);
            return;
        }
    }
}

void dlt_log_free(void)
{
    if(logging_mode == DLT_LOG_TO_FILE) {
        fclose(logging_handle);
    }
}

DltReturnValue dlt_log(int prio, char *s)
{
    static const char asSeverity[LOG_DEBUG+2][11] = {"EMERGENCY", "ALERT    ", "CRITICAL ", "ERROR    ", "WARNING  ", "NOTICE   ", "INFO     ", "DEBUG    ", "         "};
    static const char sFormatString[] = "[%5d.%06d]~DLT~%5d~%s~%s";
    struct timespec sTimeSpec;

    if (s == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }
    if(logging_level<prio)
    {
        return DLT_RETURN_OK;
    }
    if ((prio < 0) || (prio > LOG_DEBUG))
    {
        prio = LOG_DEBUG + 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &sTimeSpec);
    switch(logging_mode)
    {
        case DLT_LOG_TO_CONSOLE:
            /* log to stdout */
            printf(sFormatString, (unsigned int)sTimeSpec.tv_sec, (unsigned int)(sTimeSpec.tv_nsec/1000), getpid(), asSeverity[prio], s);
            break;
        case DLT_LOG_TO_SYSLOG:
            /* log to syslog */
#if !defined (__WIN32__) && !defined(_MSC_VER)
            openlog("DLT",LOG_PID,LOG_DAEMON);
            syslog(prio, sFormatString, (unsigned int)sTimeSpec.tv_sec, (unsigned int)(sTimeSpec.tv_nsec/1000), getpid(), asSeverity[prio], s);
            closelog();
#endif
            break;
        case DLT_LOG_TO_FILE:
            /* log to file */
            if(logging_handle) {
                fprintf(logging_handle,sFormatString, (unsigned int)sTimeSpec.tv_sec, (unsigned int)(sTimeSpec.tv_nsec/1000), getpid(), asSeverity[prio], s);
                fflush(logging_handle);
            }
            break;
        case DLT_LOG_DROPPED:
        default:
            break;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_vlog(int prio, const char *format, ...)
{
    char outputString[2048] = { 0 }; /* TODO: what is a reasonable string length here? */

    va_list args;

    if (format == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (logging_level < prio)
        return DLT_RETURN_OK;

    va_start(args, format);
    vsnprintf(outputString, 2047, format, args);
    va_end(args);

    dlt_log(prio, outputString);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_vnlog(int prio, size_t size, const char *format, ...)
{
    char *outputString = NULL;

    va_list args;

    if (format == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if (logging_level < prio || size == 0)
        return DLT_RETURN_OK;

    if ( (outputString = (char*) calloc(size + 1, sizeof(char))) == NULL )
        return DLT_RETURN_ERROR;

    va_start(args, format);
    vsnprintf(outputString, size, format, args);
    va_end(args);

    dlt_log(prio, outputString);

    free(outputString);
    outputString = NULL;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_receiver_init(DltReceiver *receiver, int fd, int buffersize)
{
    if (receiver == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    receiver->lastBytesRcvd = 0;
    receiver->bytesRcvd = 0;
    receiver->totalBytesRcvd = 0;
    receiver->buffersize = buffersize;
    receiver->fd = fd;
    receiver->buffer = (char*)malloc(receiver->buffersize);

    if (receiver->buffer == NULL)
    {
        receiver->buf = NULL;
        return DLT_RETURN_ERROR;
    }
    else
    {
        receiver->buf = receiver->buffer;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_receiver_free(DltReceiver *receiver)
{

    if (receiver == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (receiver->buffer)
    {
        free(receiver->buffer);
    }

    receiver->buffer = NULL;
    receiver->buf = NULL;

    return DLT_RETURN_OK;
}

#ifndef QT_VIEWER
int dlt_receiver_receive_socket(DltReceiver *receiver)
{
    if (receiver == NULL)
    {
        return -1;
    }

    if (receiver->buffer == NULL)
    {
        return -1;
    }

    receiver->buf = (char *)receiver->buffer;
    receiver->lastBytesRcvd = receiver->bytesRcvd;

    /* wait for data from socket */
    if ((receiver->bytesRcvd = recv(receiver->fd, receiver->buf + receiver->lastBytesRcvd, receiver->buffersize - receiver->lastBytesRcvd , 0)) <= 0)
    {
        receiver->bytesRcvd = 0;

        return receiver->bytesRcvd;
    } /* if */

    receiver->totalBytesRcvd += receiver->bytesRcvd;
    receiver->bytesRcvd += receiver->lastBytesRcvd;

    return receiver->bytesRcvd;
}
#endif

int dlt_receiver_receive_fd(DltReceiver *receiver)
{
    if (receiver == NULL)
    {
        return -1;
    }

    if (receiver->buffer == NULL)
    {
        return -1;
    }

    receiver->buf = (char *)receiver->buffer;
    receiver->lastBytesRcvd = receiver->bytesRcvd;

    /* wait for data from fd */
    if ((receiver->bytesRcvd = read(receiver->fd, receiver->buf + receiver->lastBytesRcvd, receiver->buffersize - receiver->lastBytesRcvd)) <= 0)
    {
        receiver->bytesRcvd = 0;

        return receiver->bytesRcvd;
    } /* if */

    receiver->totalBytesRcvd += receiver->bytesRcvd;
    receiver->bytesRcvd += receiver->lastBytesRcvd;

    return receiver->bytesRcvd;
}

DltReturnValue dlt_receiver_remove(DltReceiver *receiver, int size)
{
    if (receiver == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if (receiver->buf == NULL)
    {
        return DLT_RETURN_ERROR;
    }

    if (size > receiver->bytesRcvd || size <= 0)
    {
        receiver->buf = receiver->buf + receiver->bytesRcvd;
        receiver->bytesRcvd = 0;
        return DLT_RETURN_WRONG_PARAMETER;
    }

    receiver->bytesRcvd = receiver->bytesRcvd - size;
    receiver->buf = receiver->buf + size;

    return DLT_RETURN_OK;
}

DltReturnValue dlt_receiver_move_to_begin(DltReceiver *receiver)
{
    if (receiver == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    if ((receiver->buffer == NULL) || (receiver->buf == NULL))
    {
        return DLT_RETURN_ERROR;
    }

    if ((receiver->buffer!=receiver->buf) && (receiver->bytesRcvd!=0))
    {
        memmove(receiver->buffer,receiver->buf,receiver->bytesRcvd);
    }

    return DLT_RETURN_OK;
}

int dlt_receiver_check_and_get(DltReceiver *receiver,
                               void *dest,
                               unsigned int to_get,
                               unsigned int flags)
{
    unsigned int min_size = to_get;
    void *src = NULL;

    if (flags & DLT_RCV_SKIP_HEADER) {
        min_size += sizeof(DltUserHeader);
    }

    if (!receiver ||
        (receiver->bytesRcvd < (int32_t)min_size) ||
        !receiver->buf ||
        !dest) {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    src = (void *)receiver->buf;

    if (flags & DLT_RCV_SKIP_HEADER) {
        src += sizeof(DltUserHeader);
    }

    memcpy(dest, src, to_get);

    if (flags & DLT_RCV_REMOVE) {
        if (dlt_receiver_remove(receiver, min_size) != DLT_RETURN_OK)
        {
            dlt_log(LOG_WARNING,"Can't remove bytes from receiver\n");
            return DLT_RETURN_ERROR;
        }
    }

    return to_get;
}

DltReturnValue dlt_set_storageheader(DltStorageHeader *storageheader, const char *ecu)
{

#if !defined(_MSC_VER)
    struct timeval tv;
#endif

    if (storageheader == NULL || ecu == NULL)
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    /* get time of day */
#if defined(_MSC_VER)
    time(&(storageheader->seconds));
#else
    gettimeofday(&tv, NULL);
#endif

    /* prepare storage header */
    storageheader->pattern[0] = 'D';
    storageheader->pattern[1] = 'L';
    storageheader->pattern[2] = 'T';
    storageheader->pattern[3] = 0x01;

    dlt_set_id(storageheader->ecu,ecu);

    /* Set current time */
#if defined(_MSC_VER)
    storageheader->microseconds = 0;
#else
    storageheader->seconds = (time_t)tv.tv_sec; /* value is long */
    storageheader->microseconds = (int32_t)tv.tv_usec; /* value is long */
#endif

    return DLT_RETURN_OK;
}

DltReturnValue dlt_check_storageheader(DltStorageHeader *storageheader)
{
    if (storageheader == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    return  ((storageheader->pattern[0] == 'D') &&
             (storageheader->pattern[1] == 'L') &&
             (storageheader->pattern[2] == 'T') &&
             (storageheader->pattern[3] == 1))
             ? DLT_RETURN_TRUE : DLT_RETURN_OK;
}












DltReturnValue dlt_buffer_init_static_server(DltBuffer *buf, const unsigned char *ptr, uint32_t size)
{
    if(buf == NULL || ptr == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    char str[256];
    DltBufferHead *head;

    // Init parameters
    buf->shm = (unsigned char *)ptr;
    buf->min_size = size;
    buf->max_size = size;
    buf->step_size = 0;

    // Init pointers
     head = (DltBufferHead*)buf->shm;
    head->read = 0;
    head->write = 0;
    head->count = 0;
    buf->mem = (unsigned char *)(buf->shm + sizeof(DltBufferHead));
    buf->size = buf->min_size - sizeof(DltBufferHead);

    // clear memory
    memset(buf->mem, 0, buf->size);

    snprintf(str,sizeof(str),"Buffer: Size %d\n",buf->size);
    dlt_log(LOG_DEBUG, str);

    return DLT_RETURN_OK; /* OK */
}

DltReturnValue dlt_buffer_init_static_client(DltBuffer *buf, const unsigned char *ptr, uint32_t size)
{
    if(buf == NULL || ptr == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    char str[256];

    // Init parameters
    buf->shm = (unsigned char *)ptr;
    buf->min_size = size;
    buf->max_size = size;
    buf->step_size = 0;

    // Init pointers
    buf->mem = (unsigned char *)(buf->shm + sizeof(DltBufferHead));
    buf->size = buf->min_size - sizeof(DltBufferHead);

    snprintf(str,sizeof(str),"Buffer: Size %d\n",buf->size);
    dlt_log(LOG_DEBUG, str);

    return DLT_RETURN_OK; /* OK */
}

DltReturnValue dlt_buffer_init_dynamic(DltBuffer *buf, uint32_t min_size, uint32_t max_size,uint32_t step_size)
{
    //Do not DLT_SEM_LOCK inside here!
    char str[256];
    DltBufferHead *head;

    // catch null pointer
    if(buf == NULL)
        return DLT_RETURN_WRONG_PARAMETER;
    // catch 0 logical errors
    if(min_size == 0 || max_size == 0 || step_size == 0)
        return DLT_RETURN_WRONG_PARAMETER;

    if(min_size > max_size)
        return DLT_RETURN_WRONG_PARAMETER;

    if(step_size > max_size)
        return DLT_RETURN_WRONG_PARAMETER;

    // Init parameters
    buf->min_size = min_size;
    buf->max_size = max_size;
    buf->step_size = step_size;

    // allocat memory
    buf->shm = malloc(buf->min_size);
    if(buf->shm == NULL) {
        snprintf(str,sizeof(str),"Buffer: Cannot allocate %d bytes\n",buf->min_size);
        dlt_log(LOG_EMERG, str);
        return DLT_RETURN_ERROR;
    }

    // Init pointers
    head = (DltBufferHead*)buf->shm;
    head->read = 0;
    head->write = 0;
    head->count = 0;
    buf->mem = (unsigned char *)(buf->shm + sizeof(DltBufferHead));
    buf->size = buf->min_size - sizeof(DltBufferHead);

    // clear memory
    memset(buf->mem,0,buf->size);

    //snprintf(str,sizeof(str),"Buffer: Size %d bytes\n",buf->size);
    //dlt_log(LOG_INFO, str);

    return DLT_RETURN_OK; /* OK */
}

DltReturnValue dlt_buffer_free_static(DltBuffer *buf)
{
    // catch null pointer
    if(buf == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if(buf->mem == NULL) {
        // buffer not initialized
        dlt_log(LOG_WARNING,"Buffer: Buffer not initialized\n");
        return DLT_RETURN_ERROR; /* ERROR */
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_buffer_free_dynamic(DltBuffer *buf)
{
    // catch null pointer
    if(buf == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    if(buf->shm == NULL) {
        // buffer not initialized
        dlt_log(LOG_WARNING,"Buffer: Buffer not initialized\n");
        return DLT_RETURN_ERROR; /* ERROR */
    }

    free(buf->shm);
    buf->shm = NULL;
    buf->mem = NULL;

    return DLT_RETURN_OK;
}

void dlt_buffer_write_block(DltBuffer *buf, int *write, const unsigned char *data, unsigned int size)
{
    // catch null pointer
    if(buf != NULL && write != NULL && data != NULL) {
        if((int)(*write+size) <= buf->size) {
            // write one block
            memcpy(buf->mem+*write,data,size);
            *write += size;
        }
        else {
            // write two blocks
            memcpy(buf->mem+*write, data, buf->size-*write);
            memcpy(buf->mem, data+buf->size-*write, size-buf->size+*write);
            *write += size-buf->size;
        }
    }
    else
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
    }
}

void dlt_buffer_read_block(DltBuffer *buf, int *read, unsigned char *data, unsigned int size)
{
    // catch nullpointer
    if(buf != NULL && read != NULL && data != NULL) {
        if((int)(*read+size) <= buf->size) {
            // read one block
            memcpy(data,buf->mem+*read,size);
            *read += size;
        }
        else {
            // read two blocks
            memcpy(data, buf->mem+*read, buf->size-*read);
            memcpy(data+buf->size-*read, buf->mem, size-buf->size+*read);
            *read += size-buf->size;
        }
    }
    else
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
    }
}

int dlt_buffer_increase_size(DltBuffer *buf)
{
    // catch null pointer
    if(buf == NULL) {
        return -1;
    }

    DltBufferHead *head,*new_head;
    unsigned char *new_ptr;

    /* check size */
    if(buf->step_size==0) {
        /* cannot increase size */
        return -1;
    }

    /* check size */
    if((buf->size + sizeof(DltBufferHead) + buf->step_size) > buf->max_size) {
        /* max size reached, do not increase */
        return -1;
    }

    /* allocate new buffer */
    new_ptr = malloc(buf->size + sizeof(DltBufferHead) + buf->step_size);
    if(new_ptr == NULL) {
        snprintf(str,sizeof(str),"Buffer: Cannot increase size because allocate %d bytes failed\n",buf->min_size);
        dlt_log(LOG_WARNING, str);
        return -1;
    }

    /* copy data */
    head = (DltBufferHead*)buf->shm;
    new_head = (DltBufferHead*)new_ptr;
    if(head->read < head->write) {
        memcpy(new_ptr+sizeof(DltBufferHead) , buf->mem+head->read , head->write-head->read);
        new_head->read = 0;
        new_head->write = head->write-head->read;
        new_head->count = head->count;
    }
    else {
        memcpy(new_ptr+sizeof(DltBufferHead) , buf->mem+head->read , buf->size-head->read);
        memcpy(new_ptr+sizeof(DltBufferHead)+buf->size-head->read , buf->mem , head->write);
        new_head->read = 0;
        new_head->write = buf->size-head->read+head->write;
        new_head->count = head->count;
    }

    /* free old data */
    free(buf->shm);

    /* update data */
    buf->shm = new_ptr;
    buf->mem = new_ptr+sizeof(DltBufferHead);
    buf->size += buf->step_size;

    snprintf(str,sizeof(str), "Buffer: Size increased to %d bytes\n", buf->size+(int32_t)sizeof(DltBufferHead));
    dlt_log(LOG_DEBUG, str);

    return 0; // OK
}

int dlt_buffer_minimize_size(DltBuffer *buf)
{
    unsigned char *new_ptr;

    // catch null pointer
    if(buf == NULL)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return -1;
    }

    if((buf->size + sizeof(DltBufferHead)) == buf->min_size)
    {
        /* already minimized */
        return 0;
    }

    /* allocate new buffer */
    new_ptr = malloc(buf->min_size);
    if(new_ptr == NULL) {
        snprintf(str,sizeof(str),"Buffer: Cannot set to min size of %d bytes\n",buf->min_size);
        dlt_log(LOG_WARNING, str);
        return -1;
    }

    /* free old data */
    free(buf->shm);

    /* update data */
    buf->shm = new_ptr;
    buf->mem = new_ptr+sizeof(DltBufferHead);
    buf->size = buf->min_size - sizeof(DltBufferHead);

    /* reset pointers and counters */
    ((int*)(buf->shm))[0] = 0;  // pointer to write memory
    ((int*)(buf->shm))[1] = 0;  // pointer to read memory
    ((int*)(buf->shm))[2] = 0;  // number of packets

    // clear memory
    memset(buf->mem, 0, buf->size);

    dlt_log(LOG_DEBUG,"Buffer: Buffer minimized.\n");

    return 0; /* OK */
}

int dlt_buffer_reset(DltBuffer *buf)
{
    // catch null pointer
    if(buf == NULL)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return -1;
    }

    dlt_log(LOG_WARNING, "Buffer: Buffer reset triggered.\n");

    /* reset pointers and counters */
    ((int*)(buf->shm))[0] = 0;  // pointer to write memory
    ((int*)(buf->shm))[1] = 0;  // pointer to read memory
    ((int*)(buf->shm))[2] = 0;  // number of packets

    // clear memory
    memset(buf->mem,0,buf->size);

    return 0; /* OK */
}

DltReturnValue dlt_buffer_push(DltBuffer *buf,const unsigned char *data,unsigned int size)
{
    return dlt_buffer_push3(buf, data, size, 0, 0, 0, 0);
}

int dlt_buffer_push3(DltBuffer *buf, const unsigned char *data1, unsigned int size1, const unsigned char *data2, unsigned int size2, const unsigned char *data3, unsigned int size3)
{
    int free_size;
    int write, read, count;
    DltBufferBlockHead head;

    // catch null pointer
    if (buf == NULL)
        return DLT_RETURN_ERROR;

    if (buf->shm == NULL)
    {
        // buffer not initialised
        dlt_log(LOG_ERR, "Buffer: Buffer not initialised\n");
        return DLT_RETURN_ERROR; /* ERROR */
    }

    // get current write pointer
    write = ((int*) (buf->shm))[0];
    read = ((int*) (buf->shm))[1];
    count = ((int*) (buf->shm))[2];

    // check pointers
    if ((read > buf->size) || (write > buf->size))
    {
        dlt_log(LOG_ERR, "Buffer: Pointer out of range\n");
        dlt_buffer_reset(buf);
        return DLT_RETURN_ERROR; // ERROR
    }

    // calculate free size
    if (read > write)
        free_size = read - write;
    else if (count && (write == read))
        free_size = 0;
    else
        free_size = buf->size - write + read;

    // check size
    if (free_size < (int) (sizeof(DltBufferBlockHead) + size1 + size2 + size3))
    {
        // try to increase size if possible
        if (dlt_buffer_increase_size(buf))
        {
            /* increase size is not possible */
            //dlt_log(LOG_ERR,"Buffer: Buffer is full\n");
            return DLT_RETURN_ERROR; // ERROR
        }

        // update pointers
        write = ((int*) (buf->shm))[0];
        read = ((int*) (buf->shm))[1];
    }

    // set header
    strncpy(head.head, DLT_BUFFER_HEAD, 3);
    head.head[3] = 0;
    head.status = 2;
    head.size = size1 + size2 + size3;

    // write data
    dlt_buffer_write_block(buf, &write, (unsigned char*) &head, sizeof(DltBufferBlockHead));
    if (size1)
        dlt_buffer_write_block(buf, &write, data1, size1);
    if (size2)
        dlt_buffer_write_block(buf, &write, data2, size2);
    if (size3)
        dlt_buffer_write_block(buf, &write, data3, size3);

    // update global shm pointers
    ((int*) (buf->shm))[0] = write; // set new write pointer
    ((int*) (buf->shm))[2] += 1; // increase counter

    return DLT_RETURN_OK; // OK

}

int dlt_buffer_get(DltBuffer *buf, unsigned char *data, int max_size, int delete)
{
    int used_size;
    int write, read, count;
    char head_compare[] = DLT_BUFFER_HEAD;
    DltBufferBlockHead head;

    // catch null pointer
    if(buf == NULL)
        return -1;

    if(buf->shm == NULL) {
        // shm not initialised
        dlt_log(LOG_ERR,"Buffer: SHM not initialised\n");
        return -1; /* ERROR */
    }

    // get current write pointer
    write = ((int*)(buf->shm))[0];
    read = ((int*)(buf->shm))[1];
    count = ((int*)(buf->shm))[2];

    // check pointers
    if((read>buf->size) || (write>buf->size) || (count<0))
    {
        dlt_log(LOG_ERR,"Buffer: Pointer out of range\n");
        dlt_buffer_reset(buf);
        return -1; // ERROR
    }

    // check if data is in there
    if(count==0) {
        if(write!=read)
        {
            dlt_log(LOG_ERR,"Buffer: SHM should be empty, but is not\n");
            dlt_buffer_reset(buf);
        }
        return -1; // ERROR
    }

    // calculate used size
    if(write>read)
        used_size = write - read;
    else
        used_size = buf->size - read + write;

    // first check size
    if(used_size < (int)(sizeof(DltBufferBlockHead))) {
        dlt_log(LOG_ERR,"Buffer: Size check 1 failed\n");
        dlt_buffer_reset(buf);
        return -1; // ERROR
    }

    // read header
    dlt_buffer_read_block(buf,&read,(unsigned char*)&head,sizeof(DltBufferBlockHead));

    // check header
    if(memcmp((unsigned char*)(head.head),head_compare,sizeof(head_compare))!=0)
    {
        dlt_log(LOG_ERR,"Buffer: Header head check failed\n");
        dlt_buffer_reset(buf);
        return -1; // ERROR
    }
    if(head.status != 2)
    {
        dlt_log(LOG_ERR,"Buffer: Header status check failed\n");
        dlt_buffer_reset(buf);
        return -1; // ERROR
    }

    // second check size
    if(used_size < (int)(sizeof(DltBufferBlockHead)+head.size)) {
        dlt_log(LOG_ERR,"Buffer: Size check 2 failed\n");
        dlt_buffer_reset(buf);
        return -1; // ERROR
    }

    // third check size
    if(max_size && (head.size > max_size)) {
        dlt_log(LOG_ERR,"Buffer: Size check 3 failed\n");
        // nothing to do but data does not fit provided buffer
    }

    if(data != NULL && max_size)
    {
        // read data
        dlt_buffer_read_block(buf,&read,data,head.size);

        if(delete)
        {
            // update buffer pointers
            ((int*)(buf->shm))[1] = read; // set new read pointer
        }
    }
    else
    {
        if(delete) {
            if( (read+head.size) <= buf->size)
                ((int*)(buf->shm))[1] = read+head.size; // set new read pointer
            else
                ((int*)(buf->shm))[1] = read+head.size-buf->size; // set new read pointer
        }
    }
    if(delete) {
        ((int*)(buf->shm))[2] -= 1; // decrease counter

        if(((int*)(buf->shm))[2] == 0)
        {
            // try to minimize size
            dlt_buffer_minimize_size(buf);
        }
    }

    return head.size; // OK
}

int dlt_buffer_pull(DltBuffer *buf,unsigned char *data, int max_size)
{
    return dlt_buffer_get(buf,data,max_size,1);
}

int dlt_buffer_copy(DltBuffer *buf,unsigned char *data, int max_size)
{
    return dlt_buffer_get(buf,data,max_size,0);
}

int dlt_buffer_remove(DltBuffer *buf)
{
    return dlt_buffer_get(buf,0,0,1);
}

void dlt_buffer_info(DltBuffer *buf)
{
    // check nullpointer
    if(buf == NULL)
        return;

    char str[256];

    snprintf(str,sizeof(str),"Buffer: Available size: %d\n",buf->size);
    dlt_log(LOG_DEBUG, str);
    snprintf(str,sizeof(str),"Buffer: Buffer full start address: %lX\n",(unsigned long)buf->shm);
    dlt_log(LOG_DEBUG, str);
    snprintf(str,sizeof(str),"Buffer: Buffer start address: %lX\n",(unsigned long)buf->mem);
    dlt_log(LOG_DEBUG, str);

}

void dlt_buffer_status(DltBuffer *buf)
{
    int write, read, count;
    char str[256];

    // check nullpointer
    if(buf == NULL) {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    /* check if buffer available */
    if(buf->shm == NULL)
        return;

    write = ((int*)(buf->shm))[0];
    read = ((int*)(buf->shm))[1];
    count = ((int*)(buf->shm))[2];

    snprintf(str,sizeof(str),"Buffer: Write: %d\n",write);
    dlt_log(LOG_DEBUG, str);
    snprintf(str,sizeof(str),"Buffer: Read: %d\n",read);
    dlt_log(LOG_DEBUG, str);
    snprintf(str,sizeof(str),"Buffer: Count: %d\n",count);
    dlt_log(LOG_DEBUG, str);
}

uint32_t dlt_buffer_get_total_size(DltBuffer *buf)
{
    // catch null pointer
    if(buf == NULL)
        return -1;

    return buf->max_size;
}

int dlt_buffer_get_used_size(DltBuffer *buf)
{
    int write, read, count;

    // catch null pointer
    if(buf == NULL)
        return -1;

    /* check if buffer available */
    if(buf->shm == NULL)
        return 0;

    write = ((int*)(buf->shm))[0];
    read = ((int*)(buf->shm))[1];
    count = ((int*)(buf->shm))[2];

    if(count == 0)
        return 0;

    if(write>read)
        return (write - read);

    return (buf->size - read + write);
}

int dlt_buffer_get_message_count(DltBuffer *buf)
{
    // catch null pointer
    if(buf == NULL)
        return -1;

    /* check if buffer available */
    if(buf->shm == NULL)
        return 0;

    return ((int*)(buf->shm))[2];
}

#if !defined (__WIN32__)

DltReturnValue dlt_setup_serial(int fd, speed_t speed)
{
#if !defined (__WIN32__) && !defined(_MSC_VER)
    struct termios config;

    if (isatty(fd)==0)
    {
        return DLT_RETURN_ERROR;
    }

    if (tcgetattr(fd, &config) < 0)
    {
        return DLT_RETURN_ERROR;
    }

    /* Input flags - Turn off input processing
       convert break to null byte, no CR to NL translation,
       no NL to CR translation, don't mark parity errors or breaks
       no input parity check, don't strip high bit off,
       no XON/XOFF software flow control
    */
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);

    /* Output flags - Turn off output processing
       no CR to NL translation, no NL to CR-NL translation,
       no NL to CR translation, no column 0 CR suppression,
       no Ctrl-D suppression, no fill characters, no case mapping,
       no local output processing

       config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
                           ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    */
    config.c_oflag = 0;

    /* No line processing:
       echo off, echo newline off, canonical mode off,
       extended input processing off, signal chars off
    */
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    /* Turn off character processing
       clear current char size mask, no parity checking,
       no output processing, force 8 bit input
    */
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;

    /* One input byte is enough to return from read()
       Inter-character timer off
    */
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    /* Communication speed (simple version, using the predefined
       constants)
    */
    if (cfsetispeed(&config, speed) < 0 || cfsetospeed(&config, speed) < 0)
    {
        return DLT_RETURN_ERROR;
    }

    /* Finally, apply the configuration
    */
    if (tcsetattr(fd, TCSAFLUSH, &config) < 0)
    {
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
#else
    return DLT_RETURN_ERROR;
#endif
}

speed_t dlt_convert_serial_speed(int baudrate)
{
#if !defined (__WIN32__) && !defined(_MSC_VER) && !defined(__CYGWIN__)
    speed_t ret;

    switch (baudrate)
    {
    case  50:
    {
        ret = B50;
        break;
    }
    case  75:
    {
        ret = B75;
        break;
    }
    case  110:
    {
        ret = B110;
        break;
    }
    case  134:
    {
        ret = B134;
        break;
    }
    case  150:
    {
        ret = B150;
        break;
    }
    case  200:
    {
        ret = B200;
        break;
    }
    case  300:
    {
        ret = B300;
        break;
    }
    case  600:
    {
        ret = B600;
        break;
    }
    case  1200:
    {
        ret = B1200;
        break;
    }
    case  1800:
    {
        ret = B1800;
        break;
    }
    case  2400:
    {
        ret = B2400;
        break;
    }
    case  4800:
    {
        ret = B4800;
        break;
    }
    case  9600:
    {
        ret = B9600;
        break;
    }
    case  19200:
    {
        ret = B19200;
        break;
    }
    case  38400:
    {
        ret = B38400;
        break;
    }
    case  57600:
    {
        ret = B57600;
        break;
    }
    case  115200:
    {
        ret = B115200;
        break;
    }
    case 230400:
    {
        ret = B230400;
        break;
    }
    case 460800:
    {
        ret = B460800;
        break;
    }
    case  500000:
    {
        ret = B500000;
        break;
    }
    case  576000:
    {
        ret = B576000;
        break;
    }
    case  921600:
    {
        ret = B921600;
        break;
    }
    case  1000000:
    {
        ret = B1000000;
        break;
    }
    case  1152000:
    {
        ret = B1152000;
        break;
    }
    case  1500000:
    {
        ret = B1500000;
        break;
    }
    case  2000000:
    {
        ret = B2000000;
        break;
    }
    case  2500000:
    {
        ret = B2500000;
        break;
    }
    case  3000000:
    {
        ret = B3000000;
        break;
    }
    case  3500000:
    {
        ret = B3500000;
        break;
    }
    case  4000000:
    {
        ret = B4000000;
        break;
    }
    default:
    {
        ret = B115200;
        break;
    }
    }

    return ret;
#else
    return 0;
#endif
}

#endif

void dlt_get_version(char *buf, size_t size)
{
    if(buf == NULL && size > 0)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    snprintf(buf,size,"DLT Package Version: %s %s, Package Revision: %s, build on %s %s\n%s %s %s %s\n",
            _DLT_PACKAGE_VERSION, _DLT_PACKAGE_VERSION_STATE, _DLT_PACKAGE_REVISION, __DATE__ , __TIME__,_DLT_SYSTEMD_ENABLE,_DLT_SYSTEMD_WATCHDOG_ENABLE,_DLT_TEST_ENABLE,_DLT_SHM_ENABLE );
}

void dlt_get_major_version(char *buf, size_t size)
{
    if (buf == NULL && size > 0)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    snprintf(buf, size, "%s", _DLT_PACKAGE_MAJOR_VERSION);
}

void dlt_get_minor_version(char *buf, size_t size)
{
    if (buf == NULL && size > 0)
    {
        dlt_log(LOG_WARNING, "Wrong parameter: Null pointer\n");
        return;
    }

    snprintf(buf, size, "%s", _DLT_PACKAGE_MINOR_VERSION);
}


uint32_t dlt_uptime(void)
{

#if defined (__WIN32__) || defined(_MSC_VER)

    return (uint32_t)(GetTickCount()*10); /* GetTickCount() return DWORD */

#else
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC,&ts)==0)
    {
        return ( (uint32_t)ts.tv_sec*10000 + (uint32_t)ts.tv_nsec/100000 );// in 0.1 ms = 100 us
    }
    else
    {
        return 0;
    }

#endif

}

DltReturnValue dlt_message_print_header(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message == NULL) || (text == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_message_header(message, text, size, verbose);
    printf("%s\n",text);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_print_hex(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message == NULL) || (text == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s ",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_HEX,verbose);
    printf("[%s]\n",text);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_print_ascii(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message == NULL) || (text == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s ",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_ASCII,verbose);
    printf("[%s]\n",text);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_print_mixed_plain(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message == NULL) || (text == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s \n",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_MIXED_FOR_PLAIN,verbose);
    printf("[%s]\n",text);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_print_mixed_html(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message == NULL) || (text == NULL))
    {
        return DLT_RETURN_WRONG_PARAMETER;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s \n",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_MIXED_FOR_HTML,verbose);
    printf("[%s]\n",text);

    return DLT_RETURN_OK;
}

DltReturnValue dlt_message_argument_print(DltMessage *msg,uint32_t type_info,uint8_t **ptr,int32_t *datalength,char *text,int textlength,int byteLength,int __attribute__((unused)) verbose)
{
    // check null pointers
    if(msg == NULL || ptr == NULL || datalength == NULL || text == NULL)
        return DLT_RETURN_WRONG_PARAMETER;

    int16_t length=0,length_tmp=0; /* the macro can set this variable to -1 */
    uint16_t length2=0,length2_tmp=0,length3=0,length3_tmp=0;

    uint8_t value8u=0;
    uint16_t value16u=0,value16u_tmp=0;
    uint32_t value32u=0,value32u_tmp=0;
    uint64_t value64u=0,value64u_tmp=0;

    int8_t  value8i=0;
    int16_t value16i=0,value16i_tmp=0;
    int32_t value32i=0,value32i_tmp=0;
    int64_t value64i=0,value64i_tmp=0;

    float32_t value32f=0,value32f_tmp=0;
    int32_t value32f_tmp_int32i=0,value32f_tmp_int32i_swaped=0;
    float64_t value64f=0,value64f_tmp=0;
    int64_t value64f_tmp_int64i=0,value64f_tmp_int64i_swaped=0;

    uint32_t quantisation_tmp = 0;

    /* apparently this makes no sense but needs to be done to prevent compiler warning.
     * This variable is only written by DLT_MSG_READ_VALUE macro in if (type_info & DLT_TYPE_INFO_FIXP)
     * case but never read anywhere */
    quantisation_tmp += quantisation_tmp;

    if ( (type_info & DLT_TYPE_INFO_STRG) && (((type_info & DLT_TYPE_INFO_SCOD) == DLT_SCOD_ASCII) || ((type_info & DLT_TYPE_INFO_SCOD) == DLT_SCOD_UTF8)) )
    {
        /* string type or utf8-encoded string type */
        if (byteLength<0)
        {
            DLT_MSG_READ_VALUE(length_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length_tmp);
        }
        else
        {
            length=(int16_t)byteLength;
        }

        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            if((*datalength)<length2)
                return DLT_RETURN_ERROR;
            *ptr += length2;
            *datalength-=length2;
        }

        DLT_MSG_READ_STRING((text+strlen(text)),*ptr,*datalength,length);
        if((*datalength)<0)
            return DLT_RETURN_ERROR;

    }
    else if (type_info & DLT_TYPE_INFO_BOOL)
    {
        /* Boolean type */
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            if((*datalength)<length2)
                return DLT_RETURN_ERROR;
            *ptr += length2;
            *datalength-=length2;
        }
        value8u=0;
        DLT_MSG_READ_VALUE(value8u,*ptr,*datalength,uint8_t); /* No endian conversion necessary */
        if((*datalength)<0)
            return DLT_RETURN_ERROR;
        snprintf(text+strlen(text),textlength-strlen(text),"%d",value8u);
    }
    else if ((type_info & DLT_TYPE_INFO_UINT) && (DLT_SCOD_BIN == (type_info & DLT_TYPE_INFO_SCOD)))
    {
        if (DLT_TYLE_8BIT == (type_info & DLT_TYPE_INFO_TYLE))
        {
            DLT_MSG_READ_VALUE(value8u,*ptr,*datalength,uint8_t); /* No endian conversion necessary */
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            char binary[10] = {'\0'}; // e.g.: "0b1100 0010"
            int i;
            for (i = (1 << 7); i > 0; i >>= 1)
            {
                if ((1<<3) == i)
                    strcat(binary, " ");
                strcat(binary, (i == (value8u & i)) ? "1" : "0");
            }

            snprintf(text+strlen(text),textlength-strlen(text),"0b%s",binary);
        }
        if (DLT_TYLE_16BIT == (type_info & DLT_TYPE_INFO_TYLE))
        {
            DLT_MSG_READ_VALUE(value16u,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            char binary[20] = {'\0'}; // e.g.: "0b1100 0010 0011 0110"
            int i;
            for (i = (1 << 15); i > 0; i >>= 1)
            {
                if (((1<<3) == i) || ((1<<7) == i) || ((1<<11) == i))
                    strcat(binary, " ");
                strcat(binary, (i == (value16u & i)) ? "1" : "0");
            }

            snprintf(text+strlen(text),textlength-strlen(text),"0b%s",binary);
        }
    }
    else if ((type_info & DLT_TYPE_INFO_UINT) && (DLT_SCOD_HEX == (type_info & DLT_TYPE_INFO_SCOD)))
    {
        if (DLT_TYLE_8BIT == (type_info & DLT_TYPE_INFO_TYLE))
        {
            DLT_MSG_READ_VALUE(value8u,*ptr,*datalength,uint8_t); /* No endian conversion necessary */
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            snprintf(text+strlen(text),textlength-strlen(text),"0x%02x",value8u);
        }
        if (DLT_TYLE_16BIT == (type_info & DLT_TYPE_INFO_TYLE))
        {
            DLT_MSG_READ_VALUE(value16u,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            snprintf(text+strlen(text),textlength-strlen(text),"0x%04x",value16u);
        }
        if (DLT_TYLE_32BIT == (type_info & DLT_TYPE_INFO_TYLE))
        {
            DLT_MSG_READ_VALUE(value32u,*ptr,*datalength,uint32_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            snprintf(text+strlen(text),textlength-strlen(text),"0x%08x",value32u);
        }
        if (DLT_TYLE_64BIT == (type_info & DLT_TYPE_INFO_TYLE))
        {
            *ptr += 4;
            DLT_MSG_READ_VALUE(value32u,*ptr,*datalength,uint32_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            snprintf(text+strlen(text),textlength-strlen(text),"0x%08x",value32u);
            *ptr -= 8;
            DLT_MSG_READ_VALUE(value32u,*ptr,*datalength,uint32_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            snprintf(text+strlen(text),textlength-strlen(text),"%08x",value32u);
            *ptr += 4;
        }
    }
    else if ((type_info & DLT_TYPE_INFO_SINT) || (type_info & DLT_TYPE_INFO_UINT))
    {
        /* signed or unsigned argument received */
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            DLT_MSG_READ_VALUE(length3_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length3=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length3_tmp);
            if((*datalength)<length2)
                return DLT_RETURN_ERROR;
            *ptr += length2;
            *datalength-=length2;
            if((*datalength)<length3)
                return DLT_RETURN_ERROR;
            *ptr += length3;
            *datalength-=length3;
        }
        if (type_info & DLT_TYPE_INFO_FIXP)
        {
            DLT_MSG_READ_VALUE(quantisation_tmp, *ptr, *datalength, uint32_t);

            if((*datalength)<0)
                return DLT_RETURN_ERROR;

            switch (    type_info & DLT_TYPE_INFO_TYLE)
            {
                case DLT_TYLE_8BIT:
                case DLT_TYLE_16BIT:
                case DLT_TYLE_32BIT:
                {
                    if((*datalength)<4)
                        return DLT_RETURN_ERROR;
                    *ptr += 4;
                    *datalength-=4;
                    break;
                }
                case DLT_TYLE_64BIT:
                {
                    if((*datalength)<8)
                        return DLT_RETURN_ERROR;
                    *ptr += 8;
                    *datalength-=8;
                    break;
                }
                case DLT_TYLE_128BIT:
                {
                    if((*datalength)<16)
                        return DLT_RETURN_ERROR;
                    *ptr += 16;
                    *datalength-=16;
                    break;
                }
                default:
                {
                    return DLT_RETURN_ERROR;
                }
            }
        }
        switch (type_info & DLT_TYPE_INFO_TYLE)
        {
            case DLT_TYLE_8BIT:
            {
                if (type_info & DLT_TYPE_INFO_SINT)
                {
                    value8i=0;
                    DLT_MSG_READ_VALUE(value8i,*ptr,*datalength,int8_t); /* No endian conversion necessary */
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    snprintf(text+strlen(text),textlength-strlen(text),"%d",value8i);
                }
                else
                {
                    value8u=0;
                    DLT_MSG_READ_VALUE(value8u,*ptr,*datalength,uint8_t); /* No endian conversion necessary */
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    snprintf(text+strlen(text),textlength-strlen(text),"%d",value8u);
                }
                break;
            }
            case DLT_TYLE_16BIT:
            {
                if (type_info & DLT_TYPE_INFO_SINT)
                {
                    value16i=0;
                    value16i_tmp=0;
                    DLT_MSG_READ_VALUE(value16i_tmp,*ptr,*datalength,int16_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    value16i=DLT_ENDIAN_GET_16(msg->standardheader->htyp, value16i_tmp);
                    snprintf(text+strlen(text),textlength-strlen(text),"%hd",value16i);
                }
                else
                {
                    value16u=0;
                    value16u_tmp=0;
                    DLT_MSG_READ_VALUE(value16u_tmp,*ptr,*datalength,uint16_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    value16u=DLT_ENDIAN_GET_16(msg->standardheader->htyp, value16u_tmp);
                    snprintf(text+strlen(text),textlength-strlen(text),"%hu",value16u);
                }
                break;
            }
            case DLT_TYLE_32BIT:
            {
                if (type_info & DLT_TYPE_INFO_SINT)
                {
                    value32i=0;
                    value32i_tmp=0;
                    DLT_MSG_READ_VALUE(value32i_tmp,*ptr,*datalength,int32_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    value32i=DLT_ENDIAN_GET_32(msg->standardheader->htyp, (uint32_t)value32i_tmp);
                    snprintf(text+strlen(text),textlength-strlen(text),"%d",value32i);
                }
                else
                {
                    value32u=0;
                    value32u_tmp=0;
                    DLT_MSG_READ_VALUE(value32u_tmp,*ptr,*datalength,uint32_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    value32u=DLT_ENDIAN_GET_32(msg->standardheader->htyp, value32u_tmp);
                    snprintf(text+strlen(text),textlength-strlen(text),"%u",value32u);
                }
                break;
            }
            case DLT_TYLE_64BIT:
            {
                if (type_info & DLT_TYPE_INFO_SINT)
                {
                    value64i=0;
                    value64i_tmp=0;
                    DLT_MSG_READ_VALUE(value64i_tmp,*ptr,*datalength,int64_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    value64i=DLT_ENDIAN_GET_64(msg->standardheader->htyp, (uint64_t)value64i_tmp);
    #if defined (__WIN32__) && !defined(_MSC_VER)
                    snprintf(text+strlen(text),textlength-strlen(text),"%I64d",value64i);
    #else
                    snprintf(text+strlen(text),textlength-strlen(text),"%"PRId64, value64i);
    #endif
                }
                else
                {
                    value64u=0;
                    value64u_tmp=0;
                    DLT_MSG_READ_VALUE(value64u_tmp,*ptr,*datalength,uint64_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    value64u=DLT_ENDIAN_GET_64(msg->standardheader->htyp, value64u_tmp);
    #if defined (__WIN32__) && !defined(_MSC_VER)
                    snprintf(text+strlen(text),textlength-strlen(text),"%I64u",value64u);
    #else
                    snprintf(text+strlen(text),textlength-strlen(text),"%"PRId64,value64u);
    #endif
                }
                break;
            }
            case DLT_TYLE_128BIT:
            {
                if (*datalength>=16)
                    dlt_print_hex_string(text+strlen(text),textlength,*ptr,16);
                if((*datalength)<16)
                    return DLT_RETURN_ERROR;
                *ptr += 16;
                *datalength-=16;
                break;
            }
            default:
            {
                return DLT_RETURN_ERROR;
            }
        }
    }
    else if (type_info & DLT_TYPE_INFO_FLOA)
    {
        /* float data argument */
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            DLT_MSG_READ_VALUE(length3_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length3=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length3_tmp);
            if((*datalength)<length2)
                return DLT_RETURN_ERROR;
            *ptr += length2;
            *datalength-=length2;
            if((*datalength)<length3)
                return DLT_RETURN_ERROR;
            *ptr += length3;
            *datalength-=length3;
        }
        switch (    type_info & DLT_TYPE_INFO_TYLE)
        {
            case DLT_TYLE_8BIT:
            {
                if (*datalength>=1)
                    dlt_print_hex_string(text+strlen(text),textlength,*ptr,1);
                if((*datalength)<1)
                    return DLT_RETURN_ERROR;
                *ptr += 1;
                *datalength-=1;
                break;
            }
            case DLT_TYLE_16BIT:
            {
                if (*datalength>=2)
                    dlt_print_hex_string(text+strlen(text),textlength,*ptr,2);
                if((*datalength)<2)
                    return DLT_RETURN_ERROR;
                *ptr += 2;
                *datalength-=2;
                break;
            }
            case DLT_TYLE_32BIT:
            {
                if (sizeof(float32_t)==4)
                {
                    value32f=0;
                    value32f_tmp=0;
                    value32f_tmp_int32i=0;
                    value32f_tmp_int32i_swaped=0;
                    DLT_MSG_READ_VALUE(value32f_tmp,*ptr,*datalength,float32_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    memcpy(&value32f_tmp_int32i,&value32f_tmp,sizeof(float32_t));
                    value32f_tmp_int32i_swaped=DLT_ENDIAN_GET_32(msg->standardheader->htyp, (uint32_t)value32f_tmp_int32i);
                    memcpy(&value32f,&value32f_tmp_int32i_swaped,sizeof(float32_t));
                    snprintf(text+strlen(text),textlength-strlen(text),"%g",value32f);
                }
                else
                {
                    dlt_log(LOG_ERR, "Invalid size of float32_t\n");
                    return DLT_RETURN_ERROR;
                }
                break;
            }
            case DLT_TYLE_64BIT:
            {
                if (sizeof(float64_t)==8)
                {
                    value64f=0;
                    value64f_tmp=0;
                    value64f_tmp_int64i=0;
                    value64f_tmp_int64i_swaped=0;
                    DLT_MSG_READ_VALUE(value64f_tmp,*ptr,*datalength,float64_t);
                    if((*datalength)<0)
                        return DLT_RETURN_ERROR;
                    memcpy(&value64f_tmp_int64i,&value64f_tmp,sizeof(float64_t));
                    value64f_tmp_int64i_swaped=DLT_ENDIAN_GET_64(msg->standardheader->htyp, (uint64_t)value64f_tmp_int64i);
                    memcpy(&value64f,&value64f_tmp_int64i_swaped,sizeof(float64_t));
#ifdef __arm__
                    snprintf(text+strlen(text),textlength-strlen(text),"ILLEGAL");
#else
                    snprintf(text+strlen(text),textlength-strlen(text),"%g",value64f);
#endif
                }
                else
                {
                    dlt_log(LOG_ERR, "Invalid size of float64_t\n");
                    return DLT_RETURN_ERROR;
                }
                break;
            }
            case DLT_TYLE_128BIT:
            {
                if (*datalength>=16)
                    dlt_print_hex_string(text+strlen(text),textlength,*ptr,16);
                if((*datalength)<16)
                    return DLT_RETURN_ERROR;
                *ptr += 16;
                *datalength-=16;
                break;
            }
            default:
            {
                return DLT_RETURN_ERROR;
            }
        }

    }
    else if (type_info & DLT_TYPE_INFO_RAWD)
    {
        /* raw data argument */
        DLT_MSG_READ_VALUE(length_tmp,*ptr,*datalength,uint16_t);
        if((*datalength)<0)
            return DLT_RETURN_ERROR;
        length=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length_tmp);
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return DLT_RETURN_ERROR;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            if((*datalength)<length2)
                return DLT_RETURN_ERROR;
            *ptr += length2;
            *datalength-=length2;
        }
        if((*datalength)<length)
            return DLT_RETURN_ERROR;
        dlt_print_hex_string(text+strlen(text),textlength,*ptr,length);
        *ptr+=length;
        *datalength-=length;
    }
    else if (type_info & DLT_TYPE_INFO_TRAI)
    {
        /* trace info argument */
        DLT_MSG_READ_VALUE(length_tmp,*ptr,*datalength,uint16_t);
        if((*datalength)<0)
            return DLT_RETURN_ERROR;
        length=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length_tmp);
        DLT_MSG_READ_STRING((text+strlen(text)),*ptr,*datalength,length);
        if((*datalength)<0)
            return DLT_RETURN_ERROR;
    }
    else
    {
        return DLT_RETURN_ERROR;
    }

    if (*datalength<0)
    {
        dlt_log(LOG_ERR, "Payload of DLT message corrupted\n");
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

void dlt_check_envvar()
{
    char* env_log_filename = getenv("DLT_LOG_FILENAME");

    if( env_log_filename != NULL )
    {
        dlt_log_set_filename(env_log_filename);
    }

    char* env_log_level_str = getenv("DLT_LOG_LEVEL");

	if( env_log_level_str != NULL )
	{
		int level = 0;
		if( sscanf(env_log_level_str, "%d", &level) )
		{
			dlt_log_set_level(level);
		}
	}

    char* env_log_mode = getenv("DLT_LOG_MODE");

	if( env_log_mode != NULL )
	{
		int mode = 0;
		if( sscanf(env_log_mode, "%d", &mode) )
		{
			dlt_log_init(mode);
		}
	}

    char* env_pipe_dir = getenv("DLT_PIPE_DIR");
    if( env_pipe_dir != NULL )
    {
        dlt_log_set_fifo_basedir(env_pipe_dir);
    }
}

int dlt_mkdir_recursive(const char *dir)
{
  int ret = 0;
  char tmp[PATH_MAX+1];
  char *p = NULL;
  char *end = NULL;
  size_t len;

  strncpy(tmp, dir, PATH_MAX);
  len = strlen(tmp);
  if(tmp[len - 1] == '/')
  {
    tmp[len - 1] = 0;
  }
  end = tmp + len;

  for(p = tmp + 1; ((*p) && (ret == 0)) || ((ret == -1 && errno == EEXIST) && (p != end)); p++)
  {
    if(*p == '/')
    {
      *p = 0;
      ret = mkdir(tmp, S_IRWXU);
      *p = '/';
    }
  }

  if (ret == 0 || (ret == -1 && errno == EEXIST))
  {
    ret = mkdir(tmp, S_IRWXU);
  }

  if (ret == -1 && errno == EEXIST)
  {
    ret = 0;
  }

  return ret;
}
