#ifndef DLT_SYSTEM_H
#define DLT_SYSTEM_H

#define DLT_SYSTEM_MODE_OFF 0
#define DLT_SYSTEM_MODE_STARTUP 1
#define DLT_SYSTEM_MODE_REGULAR 2

#define DLT_SYSTEM_LOG_FILE_MAX 32

typedef struct {
	char ConfigurationFile[256];
	char ApplicationId[256];
	char SyslogContextId[256];
	int  SyslogPort;
	char FiletransferDirectory[256];
	char FiletransferContextId[256];
	int  FiletransferTimeStartup;
	int  FiletransferTimeDelay;
	int  LogFileNumber;
	char LogFileFilename[DLT_SYSTEM_LOG_FILE_MAX][256];
	int  LogFileMode[DLT_SYSTEM_LOG_FILE_MAX];
	int  LogFileTimeDelay[DLT_SYSTEM_LOG_FILE_MAX];
	char LogFileContextId[DLT_SYSTEM_LOG_FILE_MAX][256];
	int  LogProcessesMode;
	char LogProcessesContextId[256];
} DltSystemOptions;

typedef struct {
	int  timeStartup;	/* time in seconds since startup of dlt-system */
	int  timeFiletransferDelay;	/* time in seconds to start next filetransfer */
	char filetransferFile[256];
	int  timeLogFileDelay[DLT_SYSTEM_LOG_FILE_MAX];	/* time in seconds to start next file log */
	int	 filetransferRunning; 	/* 0 = stooped, 1 = running */
	int  filetransferCountPackages; /* number of packets to be transfered */
	int  filetransferLastSentPackage; /* last packet sent starting from 1 */
} DltSystemRuntime;

#endif /* DLT_SYSTEM_H */
