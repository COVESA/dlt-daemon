#ifndef DLT_SYSTEM_H
#define DLT_SYSTEM_H

#define DLT_SYSTEM_MODE_OFF 0
#define DLT_SYSTEM_MODE_STARTUP 1
#define DLT_SYSTEM_MODE_REGULAR 2

typedef struct {
	char ConfigurationFile[256];
	char ApplicationId[256];
	char SyslogContextId[256];
	int  SyslogPort;
	char FiletransferDirectory[256];
	char FiletransferContextId[256];
	int  FiletransferTimeStartup;
	int  FiletransferTimeDelay;
	int  LogKernelVersionMode;
	char LogKernelVersionContextId[256];
	int  LogProcessesMode;
	char LogProcessesContextId[256];
} DltSystemOptions;

typedef struct {
	int  timeStartup;	/* time in seconds since startup of dlt-system */
	int  timeFiletransferDelay;	/* time in seconds to start next filetransfer */
	char filetransferFile[256];
	int	 filetransferRunning; 	/* 0 = stooped, 1 = running */
	int  filetransferCountPackages; /* number of packets to be transfered */
	int  filetransferLastSentPackage; /* last packet sent starting from 1 */
} DltSystemRuntime;

#endif /* DLT_SYSTEM_H */
