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
	int  KernelVersionMode;
	char KernelVersionContextId[256];
} DltSystemOptions;

#endif /* DLT_SYSTEM_H */
