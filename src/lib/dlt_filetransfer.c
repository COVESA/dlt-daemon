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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt_filetransfer.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-test-client.c                                             **
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "dlt_filetransfer.h"
#include "dlt_common.h"

//!Defines the buffer size of a single file package which will be logged to dlt
#define BUFFER_SIZE 1024

//!Defines the minimum timeout between two dlt logs. This is important because dlt should not be flooded with too many logs in a short period of time.
#define MIN_TIMEOUT 20


#define DLT_FILETRANSFER_TRANSFER_ALL_PACKAGES LONG_MAX


//!Buffer for dlt file transfer. The size is defined by BUFFER_SIZE
unsigned char buffer[BUFFER_SIZE];


//!Get some information about the file size of a file
/**See stat(2) for more informations.
 * @param file Absolute file path
 * @return Returns the size of the file (if it is a regular file or a symbolic link) in bytes. Returns 0 in case of error. Regard: can also be a size value!
 */
unsigned long getFilesize(const char* file, int *ok){
	struct stat st;

    if ( -1 == stat(file, &st))
    {
        //we can only return 0, as the value is unsigned
        *ok = 0;
        return 0;
    }
    *ok = 1;
	return (unsigned long)st.st_size;
}

/** A simple Hash function for C-strings
 * @param str input string. E.g. a file path.
 * @param hash start and result value for hash computation
 *
 */
void stringHash(const char* str, unsigned long *hash )
{
    if (!str || !hash)
        return;
   unsigned int len = strlen(str);

   unsigned int i = 0;
   if (len <= 0){
    return;
   }

   for(i = 0; i < len;  i++)
   {
      *hash = 53 * *hash  + str[i];
   }

}


//!Get some information about the file serial number of a file
/** See stat(2) for more informations.
 * @param file Absolute file path
 * @return Returns a unique number associated with each filename
 */
unsigned long getFileSerialNumber(const char* file, int *ok){
	struct stat st;
	unsigned long ret;
    if ( -1 == stat(file, &st))
    {
        *ok = 0;
        ret = 0;
    }
    else
    {
        *ok = 1;
        ret = st.st_ino;
        ret = ret << (sizeof(ret)*8)/2;
        ret |= st.st_size;
        ret ^= st.st_ctime;
        stringHash(file, &ret);
    }
	return ret;
}

//!Returns the creation date of a file
/** See stat(2) for more informations.
 * @param file Absolute file path
 * @return Returns the creation date of a file
*/
time_t getFileCreationDate(const char* file,int *ok){
	struct stat st;
    if (-1 == stat(file, &st))
    {
        *ok = 0;
        return 0;
    }
    *ok = 1;
    return st.st_ctime;
}

//!Returns the creation date of a file
/** Format of the creation date is Day Mon dd hh:mm:ss yyyy
 * @param file Absolute file path
 * @return Returns the creation date of a file
*/
char* getFileCreationDate2(const char* file,int *ok){
	struct stat st;
    if (-1 == stat(file, &st))
    {
        *ok = 0;
        return 0;
    }
    *ok = 1;
	struct tm  *ts= localtime(&st.st_ctime);
	return asctime(ts);
}

//!Checks if the file exists
/**@param file Absolute file path
 * @return Returns 1 if the file exists, 0 if the file does not exist
 */
int isFile (const char* file)
{
	struct stat   st;   
	return (stat (file, &st) == 0);
}

//!Waits a period of time
/**Waits a period of time. The minimal time to wait is MIN_TIMEOUT. This makes sure that the FIFO of dlt is not flooded.
 * @param timeout Timeout to in ms but can not be smaller as MIN_TIMEOUT
 */
void doTimeout(int timeout)
{
	usleep(timeout * 1000);
}

//!Checks free space of the user buffer
/**
 * @param returns -1 if more than 50% space in the user buffer is free. Otherwise 1 will be returned.
 */
int checkUserBufferForFreeSpace()
{
	int total_size, used_size;

	dlt_user_check_buffer(&total_size, &used_size);

	if((total_size - used_size) < (total_size/2))
	{
		return -1;
	}
	return 1;
}

//!Deletes the given file
/**
 * @param filename Absolute file path
 * @return If the file is successfully deleted, a zero value is returned.If the file can not be deleted a nonzero value is returned.
 */
int doRemoveFile(const char*filename){
	return remove( filename); 
}

void dlt_user_log_file_errorMessage(DltContext *fileContext, const char *filename, int errorCode){

	if(errno != ENOENT)
	{
        int ok = 0;
        unsigned long fserial = getFileSerialNumber(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_errorMessage, error in getFileSerialNumber for: "),DLT_STRING(filename));
        unsigned long fsize = getFilesize(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_errorMessage, error in getFilesize for: "),DLT_STRING(filename));
        char *fcreationdate = getFileCreationDate2(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_errorMessage, error in getFilesize for: "),DLT_STRING(filename));

        int package_count = dlt_user_log_file_packagesCount(fileContext,filename);

		DLT_LOG(*fileContext,DLT_LOG_ERROR,
			DLT_STRING("FLER"),
			DLT_INT(errorCode),
			DLT_INT(-errno),
            DLT_UINT(fserial),
			DLT_STRING(filename),
            DLT_UINT(fsize),
            DLT_STRING(fcreationdate),
            DLT_INT(package_count),
			DLT_UINT(BUFFER_SIZE),
			DLT_STRING("FLER")
		);		
	} else {
		DLT_LOG(*fileContext,DLT_LOG_ERROR,
			DLT_STRING("FLER"),
			DLT_INT(errorCode),
			DLT_INT(-errno),
			DLT_STRING(filename),
			DLT_STRING("FLER")
		);
	}
}



//!Logs specific file inforamtions to dlt
/**The filename, file size, file serial number and the number of packages will be logged to dlt.
 * @param fileContext Specific context
 * @param filename Absolute file path
 * @return Returns 0 if everything was okey.If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_infoAbout(DltContext *fileContext, const char *filename){
	
	if(isFile(filename))
	{
        int ok;

        unsigned long fsize = getFilesize(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_infoAbout, Error getting size of file:"),DLT_STRING(filename));

        unsigned long fserialnumber = getFileSerialNumber(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_infoAbout, Error getting serial number of file:"),DLT_STRING(filename));


        char *creationdate = getFileCreationDate2(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_infoAbout, Error getting creation date of file:"),DLT_STRING(filename));

		DLT_LOG(*fileContext,DLT_LOG_INFO,
			DLT_STRING("FLIF"),
            DLT_STRING("file serialnumber"),DLT_UINT(fserialnumber),
			DLT_STRING("filename"),DLT_STRING(filename),
            DLT_STRING("file size in bytes"),DLT_UINT(fsize),
            DLT_STRING("file creation date"),DLT_STRING(creationdate),
			DLT_STRING("number of packages"),DLT_UINT(dlt_user_log_file_packagesCount(fileContext, filename)),
			DLT_STRING("FLIF")
		);
		return 0;
	} else {
		dlt_user_log_file_errorMessage(fileContext,filename,DLT_FILETRANSFER_ERROR_INFO_ABOUT);
		return DLT_FILETRANSFER_ERROR_INFO_ABOUT;
	}
}

//!Transfer the complete file as several dlt logs.
/**This method transfer the complete file as several dlt logs. At first it will be checked that the file exist.
 * In the next step some generic informations about the file will be logged to dlt.
 * Now the header will be logged to dlt. See the method dlt_user_log_file_header for more informations.
 * Then the method dlt_user_log_data will be called with the parameter to log all packages in a loop with some timeout.
 * At last dlt_user_log_end is called to signal that the complete file transfer was okey. This is important for the plugin of the dlt viewer. 
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param deleteFlag Flag if the file will be deleted after transfer. 1->delete, 0->notDelete
 * @param timeout Timeout in ms to wait between some logs. Important that the FIFO of dlt will not be flooded with to many messages in a short period of time.
 * @return Returns 0 if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_complete(DltContext *fileContext, const char *filename, int deleteFlag, int timeout)
{	
	if(!isFile(filename))
	{
		dlt_user_log_file_errorMessage(fileContext,filename, DLT_FILETRANSFER_ERROR_FILE_COMPLETE);
		return DLT_FILETRANSFER_ERROR_FILE_COMPLETE;
	}
	
	if(dlt_user_log_file_header(fileContext,filename) != 0)
	{
		return DLT_FILETRANSFER_ERROR_FILE_COMPLETE1;
	}
		
	if(dlt_user_log_file_data(fileContext, filename,DLT_FILETRANSFER_TRANSFER_ALL_PACKAGES,timeout) != 0)
	{
		return DLT_FILETRANSFER_ERROR_FILE_COMPLETE2;
	}
		
	if(dlt_user_log_file_end(fileContext,filename, deleteFlag) != 0)
	{
		return DLT_FILETRANSFER_ERROR_FILE_COMPLETE3;
	}		
			
	return 0;
}

//!This method gives information about the number of packages the file have
/**Every file will be divided into several packages. Every package will be logged as a single dlt log.
 * The number of packages depends on the BUFFER_SIZE.
 * At first it will be checked if the file exist. Then the file will be divided into
 * several packages depending on the buffer size.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @return Returns the number of packages if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_packagesCount(DltContext *fileContext, const char *filename){
	int packages;
	long filesize;
	
	if(isFile(filename))
	{
		packages = 1;
        int ok;
        filesize = getFilesize(filename,&ok);
        if (!ok){
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("Error in: dlt_user_log_file_packagesCount, isFile"),DLT_STRING(filename),DLT_INT(DLT_FILETRANSFER_ERROR_PACKAGE_COUNT));
            return -1;
        }
        if(filesize < BUFFER_SIZE)
		{	
			return packages;
		} 
		else 
		{
			packages = filesize/BUFFER_SIZE;
			
			if(filesize%BUFFER_SIZE == 0)
			{	
				return packages;
			}
			else
			{
				return packages+1;
			}
		}
	} else {
        DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("Error in: dlt_user_log_file_packagesCount, !isFile"),DLT_STRING(filename),DLT_INT(DLT_FILETRANSFER_ERROR_PACKAGE_COUNT));
		return -1;
	}
}

//!Transfer the head of the file as a dlt logs.
/**The head of the file must be logged to dlt because the head contains inforamtion about the file serial number,
 * the file name, the file size, package number the file have and the buffer size.
 * All these informations are needed from the plugin of the dlt viewer.
 * See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param alias Alias for the file. An alternative name to show in the receiving end
 * @return Returns 0 if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_header_alias(DltContext *fileContext,const char *filename, const char *alias){

	if(isFile(filename))
    {
        int ok;

        unsigned long fserialnumber = getFileSerialNumber(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_header_alias, Error getting serial number of file:"),DLT_STRING(filename));

        unsigned long fsize = getFilesize(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_header_alias, Error getting size of file:"),DLT_STRING(filename));

        char *fcreationdate = getFileCreationDate2(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_header_alias, Error getting creation date of file:"),DLT_STRING(filename));



        DLT_LOG(*fileContext,DLT_LOG_INFO,
                DLT_STRING("FLST"),
                DLT_UINT(fserialnumber),
                DLT_STRING(alias),
                DLT_UINT(fsize),
                DLT_STRING(fcreationdate);
                DLT_UINT(dlt_user_log_file_packagesCount(fileContext,filename)),
                DLT_UINT(BUFFER_SIZE),
                DLT_STRING("FLST")
                );

		return 0;
	}
	else
	{
		dlt_user_log_file_errorMessage(fileContext,filename, DLT_FILETRANSFER_ERROR_FILE_HEAD);
		return DLT_FILETRANSFER_ERROR_FILE_HEAD;
	}
}

//!Transfer the head of the file as a dlt logs.
/**The head of the file must be logged to dlt because the head contains inforamtion about the file serial number,
 * the file name, the file size, package number the file have and the buffer size.
 * All these informations are needed from the plugin of the dlt viewer.
 * See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @return Returns 0 if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_header(DltContext *fileContext,const char *filename){

	if(isFile(filename))
	{
        int ok;

        unsigned long fserialnumber = getFileSerialNumber(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_header, Error getting serial number of file:"),DLT_STRING(filename));

        unsigned long fsize = getFilesize(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_header, Error getting size of file:"),DLT_STRING(filename));

        char *fcreationdate = getFileCreationDate2(filename,&ok);
        if (!ok)
            DLT_LOG(*fileContext,DLT_LOG_ERROR,DLT_STRING("dlt_user_log_file_header, Error getting creation date of file:"),DLT_STRING(filename));




		DLT_LOG(*fileContext,DLT_LOG_INFO,
					DLT_STRING("FLST"),
                    DLT_UINT(fserialnumber),
					DLT_STRING(filename),
                    DLT_UINT(fsize),
                    DLT_STRING(fcreationdate);
					DLT_UINT(dlt_user_log_file_packagesCount(fileContext,filename)),
					DLT_UINT(BUFFER_SIZE),
					DLT_STRING("FLST")		
				);

		return 0;
	}
	else
	{
		dlt_user_log_file_errorMessage(fileContext,filename, DLT_FILETRANSFER_ERROR_FILE_HEAD);
		return DLT_FILETRANSFER_ERROR_FILE_HEAD;
	}
}

//!Transfer the content data of a file.
/**See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param packageToTransfer Package number to transfer. If this param is LONG_MAX, the whole file will be transferred with a specific timeout
 * @param timeout Timeout to wait between dlt logs. Important because the dlt FIFO should not be flooded. Default is defined by MIN_TIMEOUT. The given timeout in ms can not be smaller than MIN_TIMEOUT.
 * @return Returns 0 if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_data(DltContext *fileContext,const char *filename, int packageToTransfer, int timeout){
	FILE *file;
	int pkgNumber;
	long readBytes;
		
	if(isFile(filename))
	{
	
		file = fopen (filename,"rb");
		if (file == NULL)
		{
			dlt_user_log_file_errorMessage(fileContext,filename,DLT_FILETRANSFER_ERROR_FILE_DATA);
			return DLT_FILETRANSFER_ERROR_FILE_DATA;
		}
		
		if( (packageToTransfer != DLT_FILETRANSFER_TRANSFER_ALL_PACKAGES && packageToTransfer > dlt_user_log_file_packagesCount(fileContext,filename)) || packageToTransfer <= 0)
		{
			DLT_LOG(*fileContext,DLT_LOG_ERROR,
				DLT_STRING("Error at dlt_user_log_file_data: packageToTransfer out of scope"),
				DLT_STRING("packageToTransfer:"),
				DLT_UINT(packageToTransfer),
				DLT_STRING("numberOfMaximalPackages:"),
				DLT_UINT(dlt_user_log_file_packagesCount(fileContext,filename)),
				DLT_STRING("for File:"),
				DLT_STRING(filename)
			);
			fclose(file);
			return DLT_FILETRANSFER_ERROR_FILE_DATA;
		}

		readBytes = 0;
		
		if(packageToTransfer != DLT_FILETRANSFER_TRANSFER_ALL_PACKAGES)
		{
//				If a single package should be transferred. The user has to check that the free space in the user buffer > 50%
//				if(checkUserBufferForFreeSpace()<0)
//					return DLT_FILETRANSFER_ERROR_FILE_DATA_USER_BUFFER_FAILED;

                if ( 0 != fseek ( file , (packageToTransfer-1)*BUFFER_SIZE , SEEK_SET ) )
                {
                        DLT_LOG(*fileContext,DLT_LOG_ERROR,
                        DLT_STRING("failed to fseek in file: "),
                        DLT_STRING(filename),
                        DLT_STRING("ferror:"),
                        DLT_INT(ferror(file))
                    );

                    fclose (file);
                    return -1;

                }
				readBytes = fread(buffer, sizeof(char), BUFFER_SIZE, file);
                int ok;
				DLT_LOG(*fileContext,DLT_LOG_INFO,
				DLT_STRING("FLDA"),
                DLT_UINT(getFileSerialNumber(filename,&ok)),
				DLT_UINT(packageToTransfer),
				DLT_RAW(buffer,readBytes),
				DLT_STRING("FLDA")
				);

				doTimeout(timeout);

		} else {
			pkgNumber = 0;
			while( !feof( file ) )
			{
//				If the complete file should be transferred, the user buffer will be checked.
//				If free space < 50% the package won't be transferred.
				if(checkUserBufferForFreeSpace()>0)
				{
					pkgNumber++;
					readBytes = fread(buffer, sizeof(char), BUFFER_SIZE, file);
					int ok;
					DLT_LOG(*fileContext,DLT_LOG_INFO,
							DLT_STRING("FLDA"),
							DLT_UINT(getFileSerialNumber(filename,&ok)),
							DLT_UINT(pkgNumber),
							DLT_RAW(buffer,readBytes),
							DLT_STRING("FLDA")
					);
				}
				doTimeout(timeout);
			}
		}
		
		fclose(file);
		
		return 0;
		
	} else {
		dlt_user_log_file_errorMessage(fileContext,filename,DLT_FILETRANSFER_ERROR_FILE_DATA);
		return DLT_FILETRANSFER_ERROR_FILE_DATA;
	}
	
}
//!Transfer the end of the file as a dlt logs.
/**The end of the file must be logged to dlt because the end contains inforamtion about the file serial number.
 * This informations is needed from the plugin of the dlt viewer.
 * See the Mainpages.c for more informations.
 * @param fileContext Specific context to log the file to dlt
 * @param filename Absolute file path
 * @param deleteFlag Flag to delete the file after the whole file is transferred (logged to dlt).1->delete,0->NotDelete
 * @return Returns 0 if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_end(DltContext *fileContext,const char *filename,int deleteFlag){

	if(isFile(filename))
	{

		int ok;
		DLT_LOG(*fileContext,DLT_LOG_INFO,
				DLT_STRING("FLFI"),
				DLT_UINT(getFileSerialNumber(filename,&ok)),
				DLT_STRING("FLFI")
		);
		
		if(deleteFlag){
				if( doRemoveFile(filename) != 0 ){
					dlt_user_log_file_errorMessage(fileContext,filename,DLT_FILETRANSFER_ERROR_FILE_END);
					return -1;
				}
		}
	
		return 0;
	}else{
		dlt_user_log_file_errorMessage(fileContext,filename,DLT_FILETRANSFER_ERROR_FILE_END);
		return DLT_FILETRANSFER_ERROR_FILE_END;
	}
}
