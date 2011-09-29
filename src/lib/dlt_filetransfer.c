#include "dlt_filetransfer.h"

//!Defines the buffer size of a single file package which will be logged to dlt
#define BUFFER_SIZE 1024

//!Defines the minimum timeout between two dlt logs. This is important because dlt should not be flooded with too many logs in a short period of time.
#define MIN_TIMEOUT 20


#define ERROR_FILE_COMPLETE -300
#define ERROR_FILE_COMPLETE1 -301
#define ERROR_FILE_COMPLETE2 -302
#define ERROR_FILE_COMPLETE3 -303
#define ERROR_FILE_HEAD -400
#define ERROR_FILE_DATA -500
#define ERROR_FILE_END -600
#define ERROR_INFO_ABOUT -700
#define ERROR_PACKAGE_COUNT -800


//!Buffer for dlt file transfer. The size is defined by BUFFER_SIZE
unsigned char buffer[BUFFER_SIZE];


//!Get some information about the file size of a file
/**See stat(2) for more informations.
 * @param file Absolute file path
 * @return Returns the size of the file (if it is a regular file or a symbolic link) in bytes.
 */
unsigned long getFilesize(const char* file){
	struct stat st;
	stat(file, &st);
	return (unsigned long)st.st_size;
}

//!Get some information about the file serial number of a file
/** See stat(2) for more informations.
 * @param file Absolute file path
 * @return Returns a unique number associated with each filename
 */
unsigned long getFileSerialNumber(const char* file){
	struct stat st;
	stat(file, &st);
	return (unsigned long)st.st_ino;
}

//!Returns the creation date of a file
/** See stat(2) for more informations.
 * @param file Absolute file path
 * @return Returns the creation date of a file
*/
time_t getFileCreationDate(const char* file){
	struct stat st;
	stat(file, &st);
    	return st.st_ctime;
}
char* getFileCreationDate2(const char* file){
	struct stat st;
	stat(file, &st);
	
	struct tm  *ts= localtime(&st.st_ctime);
    	//char       buf[80];
  	/* Format and print the time, "ddd yyyy-mm-dd hh:mm:ss zzz" */
  	//strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
	return asctime(ts);
}




//void sighandler(int sig)
//{
//	
//	fprintf(stderr, "Signal handler called with %d\n", sig);
//	signal(sig, &sighandler);
//	exit(1); 
//	
//}



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
 * @param timeout Timeout to wait in seconds in ms but can not be smaller as MIN_TIMEOUT
 */
void doTimeout(int timeout)
{
	if(timeout>MIN_TIMEOUT)
	{	
		usleep(timeout * 1000);
	}
	else
	{
		usleep(MIN_TIMEOUT * 1000);
	}	  	
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

	if(-errno != -2)
	{
		DLT_LOG(*fileContext,DLT_LOG_ERROR,
			DLT_STRING("FLER"),
			DLT_INT(errorCode),
			DLT_INT(-errno),
			DLT_UINT(getFileSerialNumber(filename)),
			DLT_STRING(filename),
			DLT_UINT(getFilesize(filename)),
			DLT_STRING(getFileCreationDate2(filename)),
			DLT_UINT(dlt_user_log_file_packagesCount(fileContext,filename)),
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
		DLT_LOG(*fileContext,DLT_LOG_INFO,
			DLT_STRING("FLIF"),
			DLT_STRING("file serialnumber"),DLT_UINT(getFileSerialNumber(filename)),
			DLT_STRING("filename"),DLT_STRING(filename),
			DLT_STRING("file size in bytes"),DLT_UINT(getFilesize(filename)),
			DLT_STRING("file creation date"),DLT_STRING(getFileCreationDate2(filename)),
			DLT_STRING("number of packages"),DLT_UINT(dlt_user_log_file_packagesCount(fileContext, filename)),
			DLT_STRING("FLIF")
		);
		return 0;
	} else {
	
		dlt_user_log_file_errorMessage(fileContext,filename,ERROR_INFO_ABOUT);
		return ERROR_INFO_ABOUT;
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
	//No signal handling in library - part of the main program!
	//signal(SIGABRT, &sighandler);
	//signal(SIGTERM, &sighandler);
	//signal(SIGINT, &sighandler);

	if(!isFile(filename))
	{
		dlt_user_log_file_errorMessage(fileContext,filename, ERROR_FILE_COMPLETE);
		return ERROR_FILE_COMPLETE;
	}
	
	//dlt_user_log_file_infoAbout(fileContext,filename);
	
	if(dlt_user_log_file_header(fileContext,filename) != 0)
	{
		return ERROR_FILE_COMPLETE1;
	}
		
	if(dlt_user_log_file_data(fileContext, filename,LONG_MAX,timeout) != 0)
	{
		return ERROR_FILE_COMPLETE2;
	}
		
	if(dlt_user_log_file_end(fileContext,filename, deleteFlag) != 0)
	{
		return ERROR_FILE_COMPLETE3;
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
		filesize = getFilesize(filename);
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
		dlt_user_log_file_errorMessage(fileContext,filename,ERROR_PACKAGE_COUNT);
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
 * @return Returns 0 if everything was okey. If there was a failure a value < 0 will be returned.
 */
int dlt_user_log_file_header(DltContext *fileContext,const char *filename){

	if(isFile(filename))
	{
		DLT_LOG(*fileContext,DLT_LOG_INFO,
					DLT_STRING("FLST"),
					DLT_UINT(getFileSerialNumber(filename)),
					DLT_STRING(filename),
					DLT_UINT(getFilesize(filename)),
					DLT_STRING(getFileCreationDate2(filename));
					DLT_UINT(dlt_user_log_file_packagesCount(fileContext,filename)),
					DLT_UINT(BUFFER_SIZE),
					DLT_STRING("FLST")		
				);

		return 0;
	}
	else
	{
		dlt_user_log_file_errorMessage(fileContext,filename, ERROR_FILE_HEAD);
		return ERROR_FILE_HEAD;
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
	int i,pkgNumber;
	long positionIndicator,readBytes;
		
	if(isFile(filename))
	{
	
		file = fopen (filename,"rb");
		if (file == NULL)
		{
			dlt_user_log_file_errorMessage(fileContext,filename,ERROR_FILE_DATA);
			return ERROR_FILE_DATA;
		}
		
		if( (packageToTransfer != LONG_MAX && packageToTransfer > dlt_user_log_file_packagesCount(fileContext,filename)) || packageToTransfer <= 0)
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
			return ERROR_FILE_DATA;
		}

		readBytes = 0;
		
		if(packageToTransfer != LONG_MAX)
		{
				fseek ( file , (packageToTransfer-1)*BUFFER_SIZE , SEEK_SET );
				readBytes = fread(buffer, sizeof(char), BUFFER_SIZE, file);
				
				DLT_LOG(*fileContext,DLT_LOG_INFO,
					DLT_STRING("FLDA"),
					DLT_UINT(getFileSerialNumber(filename)),
					DLT_UINT(packageToTransfer),
					DLT_RAW(buffer,readBytes),
					DLT_STRING("FLDA")	
				);
				
				doTimeout(timeout);
					
		} else {
			pkgNumber = 0;
			while( !feof( file ) )
			{
				pkgNumber++;
				readBytes = fread(buffer, sizeof(char), BUFFER_SIZE, file);	
				
				DLT_LOG(*fileContext,DLT_LOG_INFO,
					DLT_STRING("FLDA"),
					DLT_UINT(getFileSerialNumber(filename)),
					DLT_UINT(pkgNumber),
					DLT_RAW(buffer,readBytes),
					DLT_STRING("FLDA")		
				);
				
				doTimeout(timeout); 
			}
		}
		
		fclose(file);
		
		return 0;
		
	} else {
		dlt_user_log_file_errorMessage(fileContext,filename,ERROR_FILE_DATA);
		return ERROR_FILE_DATA;
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

		DLT_LOG(*fileContext,DLT_LOG_INFO,
				DLT_STRING("FLFI"),
				DLT_UINT(getFileSerialNumber(filename)),
				DLT_STRING("FLFI")
		);
		
		if(deleteFlag){
				if( doRemoveFile(filename) != 0 ){
					dlt_user_log_file_errorMessage(fileContext,filename,ERROR_FILE_END);
					return -1;
				}
		}
	
		return 0;
	}else{
		dlt_user_log_file_errorMessage(fileContext,filename,ERROR_FILE_END);
		return ERROR_FILE_END;
	}
}
