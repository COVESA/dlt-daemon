#include <dlt_filetransfer.h> 	/*Needed for transferring files with the dlt protocol*/
#include <dlt.h>			/*Needed for dlt logging*/

//!Declare some context for the main program. It's a must have to do this, when you want to log with dlt.
DLT_DECLARE_CONTEXT(mainContext);

//!Declare some context for the file transfer. It's not a must have to do this, but later you can set a filter on this context in the dlt viewer.
DLT_DECLARE_CONTEXT(fileContext);

//!Textfile which will be transferred.
char *file1;
//!Image which will be transferred.
char *file2;
//!Not existing file which will be transferred.
char *file3_1;
//!Not existing file which will be transferred.
char *file3_2;
//!Not existing file which will be transferred.
char *file3_3;
//!Just some variables
int i,countPackages, transferResult, dltResult;

extern int testF1P1();
extern int testF1P2();
extern int testF2P1();
extern int testF2P2();
extern int testF3P1();
extern int testF3P2();
extern int testF3P3();

//!Main program dlt-test-filestransfer starts here
int main(void)
{	
	//First file contains some text
	file1 = "/usr/share/dlt-test-filetransfer-file";
	//Second file is a picture	
	file2 = "/usr/share/dlt-test-filetransfer-image.png";
	//Third file doesn't exist. Just to test the reaction when the file isn't available.
	file3_1 = "dlt-test-filetransfer-doesntExist_1";
	//Third file doesn't exist. Just to test the reaction when the file isn't available.
	file3_2 = "dlt-test-filetransfer-doesntExist_2";
	//Third file doesn't exist. Just to test the reaction when the file isn't available.
	file3_3 = "dlt-test-filetransfer-doesntExist_3";

	//Register the application at the dlt-daemon
	dltResult = DLT_REGISTER_APP("FLTR","Test Application filetransfer");
	if(dltResult < 0){
		printf("Error: DLT_REIGSTER_APP: FLTR\n");
		return -1;
	}
	//Register the context of the main program at the dlt-daemon
	dltResult = DLT_REGISTER_CONTEXT(mainContext,"MAIN","Main context for filetransfer test");
	if(dltResult < 0){
		printf("Error: DLT_REGISTER_CONTEXT: MAIN\n");
		return -1;
	}
	//Register the context in which the file transfer will be logged at the dlt-daemon
	dltResult = DLT_REGISTER_CONTEXT(fileContext,"FLTR","Test Context for filetransfer");
	if(dltResult < 0){
		printf("Error: DLT_REGISTER_CONTEXT:FLTR\n");
		return -1;
	}
	//More details in corresponding methods	
		if(testF1P1() >= 0){
				printf("testF1P1 successful\n");
		} 
		else
		{
				printf("testF1P1 failed\n");
		}
		
		if(testF1P2() >= 0){
				printf("testF1P2 successful\n");
		} 
		else
		{
				printf("testF1P2 failed\n");
		}
		
		if(testF2P1() >= 0){
				printf("testF2P1 successful\n");
		} 
		else
		{
				printf("testF2P1 failed\n");
		}
		
		if(testF2P2() >= 0){
				printf("testF2P2 successful\n");
		} 
		else
		{
				printf("testF2P2 failed\n");
		}
		
		if(testF3P1() < 0){
				printf("testF3P1 successful\n");
		} 
		else
		{
				printf("testF3P1 failed\n");
		}
		
		if(testF3P2() < 0){
				printf("testF3P2 successful\n");
		} 
		else
		{
				printf("testF3P2 failed\n");
		}
		
		if(testF3P3() < 0 ){
				printf("testF3P3 successful\n");
		} 
		else
		{
				printf("testF3P3 failed\n");
		}

	
	//Unregister the context in which the file transfer happened from the dlt-daemon
	DLT_UNREGISTER_CONTEXT(fileContext);
	//Unregister the context of the main program from the dlt-daemon
	DLT_UNREGISTER_CONTEXT(mainContext);
	//Unregister the app from the dlt-daemon
	DLT_UNREGISTER_APP();
	
	return(0);
}

//!Test the file transfer with the condition that the transferred file is smaller as the file transfer buffer using dlt_user_log_file_complete.
int testF1P1(){
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF1P1 - dlt_user_log_file_complete"),DLT_STRING(file1));

	//Here's the line where the dlt file transfer is called. The method call needs a context, the absolute file path, will the file be deleted after transfer and the timeout between the packages	
	transferResult = dlt_user_log_file_complete(&fileContext,file1,0,20);
	if(transferResult < 0 )
	{
			printf("Error: dlt_user_log_file_complete\n");
			return transferResult;
	}
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF1P1"),DLT_STRING(file1));
	
	return transferResult;
}

//!Test the file transfer with the condition that the transferred file is smaller as the file transfer buffer using single package transfer 
int testF1P2(){
	
	//Get the information how many packages have the file
	countPackages = dlt_user_log_file_packagesCount(&fileContext,file1);
	if(countPackages < 0 )
	{
			printf("Error: dlt_user_log_file_packagesCount\n");
			return -1;
	}
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF1P2 - transfer single package"),DLT_STRING(file1));
						
	//Logs the header of the file transfer. For more details see Mainpage.c. 
	//The header gives information about the file serial number, filename (with absolute path), filesize, packages of file, buffer size
	transferResult = dlt_user_log_file_header(&fileContext,file1);
	if(transferResult >= 0)
	{
		//Loop to log all packages
		for(i=1;i<=countPackages;i++)
		{
			//Logs one single package to the file context	
			transferResult = dlt_user_log_file_data(&fileContext,file1,i,20);
			if(transferResult < 0)
			{
				printf("Error: dlt_user_log_file_data\n");
				return transferResult;
			}
		}

		//Logs the end of the file transfer. For more details see Mainpage.c
		//The end gives just information about the file serial number but is needed to signal that the file transfer has correctly finished and needed for the file transfer plugin of the dlt viewer.	
		transferResult = dlt_user_log_file_end(&fileContext,file1,0);
		if(transferResult < 0)
		{
			printf("Error: dlt_user_log_file_end\n");
			return transferResult;
		}
	}
	else
	{
		printf("Error: dlt_user_log_file_header\n");
		return transferResult;
	}
	
	//Just some log to main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF1P2 - transfer single package"),DLT_STRING(file1));
	
	return 0;
}
//!Test the file transfer with the condition that the transferred file is bigger as the file transfer buffer using dlt_user_log_file_complete.
int testF2P1(){
	//Just some log to main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF2P1 - dlt_user_log_file_complete"),DLT_STRING(file2));

	//Here's the line where the dlt file transfer is called. The method call needs a context, the absolute file path, will the file be deleted after transfer and the timeout between the packages
	transferResult = dlt_user_log_file_complete(&fileContext,file2,0,20);
	if(transferResult < 0)
	{
		printf("Error: dlt_user_log_file_complete\n");
		return transferResult;
	}
	//Just some log to main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF2P1"),DLT_STRING(file2));
	
	return transferResult;
}

//!Test the file transfer with the condition that the transferred file is bigger as the file transfer buffer using single package transfer
int testF2P2(){
	
	//Get the information how many packages have the file
	countPackages = dlt_user_log_file_packagesCount(&fileContext,file2);
	if(countPackages < 0 )
	{
			printf("Error: dlt_user_log_file_packagesCount\n");
			return -1;
	}
	
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF2P2 - transfer single package"),DLT_STRING(file2));
	
	//Logs the header of the file transfer. For more details see Mainpage.c. 
	//The header gives information about the file serial number, filename (with absolute path), filesize, packages of file, buffer size
	transferResult = dlt_user_log_file_header(&fileContext,file2);
	if( transferResult >= 0){
	
		//Loop to log all packages	
		for(i=1;i<=countPackages;i++)
		{
			//Logs one single package to the file context
			transferResult = dlt_user_log_file_data(&fileContext,file2,i,20);
			if(transferResult < 0)
			{
				printf("Error: dlt_user_log_file_data\n");
				return transferResult;
			}
		}

		//Logs the end of the file transfer. For more details see Mainpage.c
		//The end gives just information about the file serial number but is needed to signal that the file transfer has correctly finished and needed for the file transfer plugin of the dlt viewer.
		transferResult = dlt_user_log_file_end(&fileContext,file2,0);
		if(transferResult < 0)
		{
			printf("Error: dlt_user_log_file_end\n");
			return transferResult;
		}
	}
	else
	{
		printf("Error: dlt_user_log_file_header\n");
		return transferResult;
	}
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF2P2"),DLT_STRING(file2));

	return 0;
}

//!Test the file transfer with the condition that the transferred file does not exist using dlt_user_log_file_complete.
int testF3P1(){
	
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF3P1"),DLT_STRING(file3_1));

	//Here's the line where the dlt file transfer is called. The method call needs a context, the absolute file path, will the file be deleted after transfer and the timeout between the packages
	transferResult = dlt_user_log_file_complete(&fileContext,file3_1,0,20);
	if(transferResult < 0)
	{
		//Error expected because file doesn't exist
		//printf("Error: dlt_user_log_file_complete\n");
		//Just some log to the main context
		DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF3P1"),DLT_STRING(file3_1));
		return transferResult;
	}
	return transferResult;
}


//!Test the file transfer with the condition that the transferred file does not exist using single package transfer
int testF3P2(){

	//Get the information how many packages have the file
	countPackages = dlt_user_log_file_packagesCount(&fileContext,file3_2);
	if(countPackages < 0 )
	{
			//Error expected because file doesn't exist
			//printf("Error: dlt_user_log_file_packagesCount\n");
			//Just some log to the main context
			DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF3P1"),DLT_STRING(file3_2));
			return -1;
	}
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF3P1"),DLT_STRING(file3_2));
	
	//Logs the header of the file transfer. For more details see Mainpage.c. 
	//The header gives information about the file serial number, filename (with absolute path), filesize, packages of file, buffer size
	transferResult = dlt_user_log_file_header(&fileContext,file3_2);
	if( transferResult >= 0){
	
		//Loop to log all packages	
		for(i=1;i<=countPackages;i++)
		{
			//Logs one single package to the file context
			transferResult = dlt_user_log_file_data(&fileContext,file3_2,i,20);
			if(transferResult < 0)
			{
				printf("Error: dlt_user_log_file_data\n");
				return transferResult;
			}
		}

		//Logs the end of the file transfer. For more details see Mainpage.c
		//The end gives just information about the file serial number but is needed to signal that the file transfer has correctly finished and needed for the file transfer plugin of the dlt viewer.
		transferResult = dlt_user_log_file_end(&fileContext,file3_2,0);
		if(transferResult < 0)
		{
			printf("Error: dlt_user_log_file_end\n");
			return transferResult;
		}
	}
	
	return 0;
}


//!Logs some information about the file.
int testF3P3(){
	
	//Just some log to the main context
	DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Started testF3P2"),DLT_STRING(file3_3));
	
	//Here's the line where the dlt file file info is called. The method call logs some information to dlt about the file, filesize, file serial number and number of packages
	transferResult = dlt_user_log_file_infoAbout(&fileContext,file3_3);
	if(transferResult < 0)
	{
		//Error expected because file doesn't exist
		//printf("Error: dlt_user_log_file_infoAbout\n");
		//Just some log to the main context
		DLT_LOG(mainContext,DLT_LOG_INFO,DLT_STRING("Finished testF3P2"),DLT_STRING(file3_3));

		return transferResult;
	}
	
	return 0;
}
