# How to DLT for developers

Back to [README.md](../README.md)

Table of Contents
1. [Summary](#Summary)
2. [Example Application](#DLT-Example-Application)
3. [General Rules for Logging](#General-Rules-for-Logging)
4. [The use of Log Levels](#The-use-of-Log-Levels)
5. [DLT Library Runtime Configuration](#DLT-Library-Runtime-Configuration)
6. [DLT API Usage](#DLT-API-Usage)
7. [DLT injection messages](#DLT-Injection-Messages)
8. [Log level changed callback](#Log-level-changed-callback)

## DLT Example Application

To use DLT from an application, it has to be linked against the DLT library.
When the DLT daemon is installed on the system, there will be a shared library
with the name libdlt.so which provides the interface for applications to get a
connection to the DLT daemon. The library path and include path must be set in
the build environment prior to building a program using the shared dlt library.
By default, the header file "dlt.h" is located in a directory called "dlt/"
within the standard include directory.

This example gives an overview of DLT usage inside an application by using a
minimal code example. Detailed information about the API can be found later in
this document.

```
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(ctx); /* declare context */

int main()
{
	DLT_REGISTER_APP("TAPP", "Test Application for Logging");

	DLT_REGISTER_CONTEXT(ctx, "TES1", "Test Context for Logging");

	/* … */

	DLT_LOG(ctx, DLT_LOG_ERROR, DLT_CSTRING("This is an error"));

	/* … */

	DLT_UNREGISTER_CONTEXT(ctx);
	DLT_UNREGISTER_APP();
	return 0;
}
```

DLT is quite easy to use. The first thing a developer has to do is to include
the dlt header file. DLT contexts can be statically declared using the macro
shown in next line.  Firstly, a DLT application has to be registered inside the
main function. For this, an application identifier APID and application
description has to be specified. Afterwards, one or more DLT contexts could be
specified. To log messages in verbose mode, the DLT\_LOG macro can be used. As
parameter, the logging context, the log level and a variable list of parameters
have to be specified. DLT requires each parameter to be strongly typed using
DLT type macros. In this example, DLT\_CSTRING  is used to specify a constant
string. On application cleanup, all DLT contexts, as well as the DLT
application have to be unregistered.

### DLT with cmake
To use DLT with cmake, the following lines are the important ones:

```
find_package(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)
```

to INCLUDE\_DIRECTORIES, add

```
${DLT_INCLUDE_DIRS}
```

and to TARGET\_LINK\_LIBRARIES:

```
${DLT_LIBRARIES}
```

## General Rules for Logging

### Be Smart

Before implementing logging in code one should take a second to think about a
concept first. Often strategic places in the software can be used as a central
place for logging. Such places are often interfaces to other SW components. Use
the solution with the smallest impact. Avoid logging the “good cases” but log
e.g. in your error handling sections – you will need error handling anyway. In
case an error occurred more logs don’t matter as long as your regular code
produces little logs. Keep in mind that tracing comes with a price tag – you
are working in an embedded environment where CPU, memory and Bandwidth are
sparse.

### Avoid high frequency outputs

Certain events occur very often in a system – some of them dozens of times per
second. In such a case do not implement logging for each occurrence. One
example is the screen frame rate. Instead of printing a log for each frame rate
aggregate the information and print an average once every five seconds or –
even better – report once a second if the frame rate is below a critical value.

### Combine multiple messages

Please always consider that each Log message creates a certain overhead. In
case of DLT as the way of logging each has a header of 20 bytes. Therefore
please aggregate information. In this way all necessary information is always
combined. Please always use a human readable format; use identifiers for the
different values, be consistent with separators. This helps to work with the
data, especially when log messages are processed by scripts. Such scripts
often use regular expressions – make the job easier!

For example don’t write log entries like this:

> Total frames:  1000
>
> Sync frames:   0
>
> Reem frames:   0
>
> Valid frames:  0
>
> Urgent frames: 1

Better aggregate Information into a single message:

> Frame info: total=1000, sync=0, reem=1000, valid=0, urgent=1

### Do not use ASCII-art

Information should be “on your fingertips”. Logging is a tool to ease crushing
bugs, not to win a computer art contest. → Don’t use ASCII Art!

### Do not create charts using ASCII

Charts can be a great help to visualize what is going on in the system. This
type can be nicely done by a trace analysis or in case of usage of the DLT
Viewer, in a Plugin. It certainly should always be done in a post processing
step. Doing this on the target is a waste of resources.

### Avoid tracing in loops

Bad example:

```
for(int index=0; index<MAX; index++)
{
  LOG("Loop: %d", index);
  /* ... */
}
```

Good example:

```
for(int index=0; index<MAX; index++)
{
  /* ... */
}

LOG("Loop count: %d", index);
```

### Other do and avoids

Topic | Description
--- | ---
Avoid timestamps | Do not include a timestamp in your log messages. In case of DLT the logging system itself already provides a timestamp.
Avoid high logging at startup | Especially the system startup is always a high load situation. Please avoid to write a log of log outputs during the startup because the helps to avoid an overload situation and speeds up the startup.
Remove old log messages | If certain long log messages are not necessary anymore because the problem has been resolved, please remove them from your code.
Do use constant separators | Use consistent field separators and delimiters between different variables and information in order to facilitate automatic log analysis.
Do use proper logs | The logs should contain the information that you require to identify a problem. If you find yourself producing a new binary to identify a problem every time somebody reports an error something may be wrong.
Avoid eye catchers | Please don’t use custom highlighting for marking the messages which are important for you. The best way is to write a clear identifier in the front of your trace message. This can than (in the viewer) be easily used for filter sets or even for coloring. An example for messages informing about the frame rate could be: **Frame rate**: low: 12, high: 34, avr: 28
Do not log to the console | Do not use \_printf()\_ or similar statements to trace to the console. Such behavior can make it problematic to work e.g. with the serial console or even might slow down the execution of the program. In a vehicle based test system the console messages are not recorded and will not help you.
Do NOT "macrotize" dlt macros | Don’t write your own macros to capsulate DLT macros.

## The use of Log Levels

### Overview

The following log levels are available in DLT:

Log Level | Description
--- | ---
DLT\_LOG\_FATAL | Fatal system errors, should be very rare
DLT\_LOG\_ERROR | Error with impact on correct functionality
DLT\_LOG\_WARN | Warning if correct behavior cannot not be ensured
DLT\_LOG\_INFO | Informational, providing high level understanding
DLT\_LOG\_DEBUG | Detailed debug information for programmers
DLT\_LOG\_VERBOSE | Verbose debug information for programmers

Please be aware the the default Log Level is set to INFO; this means message
which are logged in INFO, WARN, ERROR and FATAL will logged. Hint: The default
Log Level can be changed by setting an environment variable (see DLT Library -
runtime configuration).

### Methods to set the log level for applications and contexts

- dlt-daemon sets initial application log level
    -  There is a configuration parameter (see /etc/dlt.conf) `ContextLogLevel`.
       When a new application registers itself at the daemon, the daemon sets
       the application's log level to the value defined by the parameter.
    - This happens when the application registers itself with
      `DLT_REGISTER_CONTEXT()` or `dlt_register_context()`

- Environment variable `DLT_INITIAL_LOG_LEVEL`
    - There is an environment variable which is called `DLT_INITIAL_LOG_LEVEL`.
      It allows to set a per-application-context log level. Refer to
      [Initial Log level](#initial-log-level) for more detail.

- Application registers itself at daemon with self-defined log level
    - In this case no log level is set by the daemon but by the application
      itself.
    - This happens when the application registers itself with
      `DLT_REGISTER_CONTEXT_LL_TS()` or `dlt_register_context_ll_ts()`

- Client (e.g. DLT Viewer) changes the log level of a particular application
  context at runtime.
    - The context's initial log level is set by one of the two methods described
      above.

### What to log at FATAL level

Fatal errors are the most serious error and should be very rare.
They are, for example:

- Errors that cause the whole system to fail
- A corrupted boot environment which prevents system boot
- A critical hardware component is missing, failing or is preventing start-up.
- When your software/process/component exits due to a fatal error. Log the EXIT
  and the reason.
- Failure of a major critical component to start

### What to log at ERROR level

This level is reserved for errors which impact the correct functionality of the
system or its components. Errors related to connected customer devices such as
phones should be logged at INFO level. Error level logs may be:

- A non-critical component is failing or cannot be found
- A system component is crashing
- A system essential file can’t be read or written
- Detection of corrupted network messages, files, etc. when these impact correct
- Some major functionality could not be provided (e.g. the route in the
  navigation could not be calculated)
- When your software/process/component exits due to an error. Log the EXIT and
  reason

### What to log at WARNING level

This level must be used for problems where a correct behavior cannot be
ensured, i.e. problems that could affect the correct functionality of the
system or its components. Warnings related to connected customer devices such
as phones must be logged at INFO level. Examples for warning log messages could
be:

- DLT dropping logs
- No disk space available for core dump
- Audio stream packet dropped
- If a process of calculation takes longer than the time allowed in
  specification e.g. Calculation of route in the navigation takes longer than
  allowed

### What to log at INFO level

This level is reserved for key information and high-level events which are not
errors or warnings of the system itself or of connected consumer devices.

- Start and non-error related stop of software components. Include version
  information in start log.
- Detection of key hardware components. Include key HW information in log.
- Customer device connected. Include key device and media info.
- Customer device detached or connection lost.
- Failure to connect to customer device. Include reason
- Corrupted disk, song, photo, etc. on customer device.
- Key system/HW information at start-up
- Information needed for reproducing and understanding user activity
- Information for reproducing the environment (Large volume data such as GPS
  traces should be logged at a reasonable rate. Especially with very frequent
  logs it should be taken care of that no redundancy occurs)
- Key information used for KPI (Key performance index) reporting

### What to log at DEBUG level

This level should be used for debug information that can help developers debug
the functionality of their software. For example:

- Information about entering and exiting major procedures
- Values of key variables, but not dumps of arrays and large number of
  variables
- Information about events received
- Network connection information
- Debug relevant information about hardware

### What to log at VERBOSE level

This level is the most detailed level and should be used for in depth debug
information that can help developers debug the functionality of their software.
For example:

- Detailed trace information
- Dumps of a large number of variables, dumps of arrays and structures
- Detailed information about events received, even events that happen very
  frequently
- Detailed network connection information
- Detailed hardware information
- Information about loops and iterations

## DLT Library Runtime Configuration

The DLT library can be configured at runtime – globally or for a specific
process – by setting different environment variables. In the following, these
environment variables are described:

### Initial Log level

The default log level of DLT User library is DLT\_LOG\_INFO. This can be changed
using a DLT client application (e.g. DLT Viewer). But there might be situations
where DEBUG or VERBOSE messages are needed before the DLT Daemon updated the
user library.

In this case DLT\_INITIAL\_LOG\_LEVEL can be exported. Using this environment
variable, the user can specify log level for contexts that will be used on
library startup.

For example, an application “EXA1” has two contexts “CON1” and “CON2”. For
“CON1” log level DEBUG and for “CON2” log level VERBOSE shall be used. The
following has to be exported to configure the library:

> export DLT\_INITIAL\_LOG\_LEVEL=”EXA1:CON1:5;EXA1:CON2:6”

### Local print mode

Sometimes it might be useful to print DLT messages for debugging directly to
console. To force the library to do so, the following environment variable can
be exported:

> export DLT\_LOCAL\_PRINT\_MODE=FORCE\_ON


### Library buffer size

The DLT library contains a message buffer in case the DLT Daemon is not started
yet or the connection to DLT Daemon is temporarily lost. The buffer is
allocated while library initialization with a minimum size. If more messages
need to be stored, the buffer grows in defined steps up to a maximum size. In
case messages are flushed to DLT Daemon, the buffer is reduced to its minimal
size. The default values and the environment variable names to set these values
are described below:

| | Default value [in bytes] |	Environment variable name
--- | --- | ---
Minimal size | 50000 | DLT\_USER\_BUFFER\_MIN
Maximal size | 500000 | DLT\_USER\_BUFFER\_MAX
Step size | 50000 | DLT\_USER\_BUFFER\_STEP

For example, to limit the maximum buffer size to 250k bytes, the following can
be exported:

> export DLT\_USER\_BUFFER\_MAX=250000

## DLT API Usage

### Register application

**Important note**
- DLT may not be used in a forked child until a variant of exec() is called,
  because DLT is using non async-signal-safe functions.
- DLT\_REGISTER\_APP is asynchronous. It may take some milliseconds to establish
  the IPC channel. Because of this, you might lose messages if you log
  immediately after registration. Typically this is not a problem, but may arise
  especially with simple examples.

The DLT application has to be registered as early as possible during the
initialization of the application by calling DLT\_REGISTER\_APP(). It is only
allowed to call DLT\_REGISTER\_APP() once per application. An application id
(maximum four characters) has to be specified and must be unique within an ECU.
In this example "MAPP" is used. And also a description for the application can
be specified, here it is "Test Application for Logging".

```
int main(int argc, const char* argv[])
{
    DLT_REGISTER_APP("MAPP","Test Application for Logging");
}
```

DLT\_REGISTER\_APP is asynchronous. It may take some milliseconds to establish
the IPC channel. Because of this, messages might be lost if logs are emitted
immediately after registering. Typically this is not a problem, but may arise
especially with simple examples.

### Define and register all logging contexts

As many contexts as needed can be defined. These contexts can be declared as
contexts in different C or CPP files. But each context is only allowed to be
declared once. Therefore a unique variable name for each context has to be
used.

```
DLT_DECLARE_CONTEXT(myContext1);
DLT_DECLARE_CONTEXT(myContext2);
DLT_DECLARE_CONTEXT(myContext3);
```

If contexts from another C or CPP file shall be used, these contexts can be
imported by calling:

```
DLT_IMPORT_CONTEXT(myContext1);
DLT_IMPORT_CONTEXT(myContext2);
DLT_IMPORT_CONTEXT(myContext3);
```

After the application is registered and contexts are declared, contexts need to
be registered early during initialization of the application.
DLT\_REGISTER\_CONTEXT() shall not be called before DLT\_REGISTER\_APP().

During registration of each context, a context id must be provided (maximum
four characters long). In this example "TESX" is used. Also a description for
the context can be provided; here it is "Test Context X for Logging". A context
can also be registered with a predefined Log Level and Trace Status by using
the Macro DLT\_REGISTER\_CONTEXT\_LL\_TS. The third context is registered using
this method.

```
int main(int argc, const char* argv[])
{
  DLT_REGISTER_APP("MAPP","Test Application for Logging");

  DLT_REGISTER_CONTEXT(myContext1,"TES1","Test Context 1 for Logging");
  DLT_REGISTER_CONTEXT(myContext2,"TES2","Test Context 2 for Logging");
  DLT_REGISTER_CONTEXT_LL_TS(myContext3, "TES3","Test Context 3 for Logging",
                             DLT_LOG_DEBUG, DLT_TRACE_STATUS_OFF);
}
```

Note: Please be aware that it might be taken up to a second until the
synchronization of loglevel between DLT Daemon and application is done.

### Unregister contexts and application

Before terminating the application registered contexts and at last the
application need to be unregistered.

```
int main(int argc, const char* argv[])
{
/* business logic */

  DLT_UNREGISTER_CONTEXT(myContext1);
  DLT_UNREGISTER_CONTEXT(myContext2);
  DLT_UNREGISTER_CONTEXT(myContext3);

  DLT_UNREGISTER_APP();

  return 0;
}
```

### Logging command

DLT provides functions and macros for logging, whereas the interface for
Verbose and Non-Verbose differs. The following table shows an example of all 4
types for logging using a constant string and an integer.

#### Verbose vs. Non-Verbose API

The following sections show examples of all 4 types for logging e.g. a string
and an integer.

##### MACRO

###### Verbose

```
DLT_LOG(ctx, DLT_LOG_INFO, DLT_STRING("ID: "), DLT_UINT32(123));
```

###### Non-Verbose

```
DLT_LOG_ID(ctx, DLT_LOG_INFO, 42 /* unique message ID */, DLT_STRING("ID: "),
           DLT_UINT32(123));
```

##### Function

###### Verbose

```
if (dlt_user_log_write_start(&ctx, &ctxdata, DLT_LOG_INFO) > 0) {
    dlt_user_log_write_string(&myctxdata, "ID: ");
    dlt_user_log_write_uint32(&myctxdata, 123);
    dlt_user_log_write_finish(&myctxdata);
}
```

###### Non-Verbose

```
if (dlt_user_log_write_start_id(&ctx, &ctxdata, DLT_LOG_INFO, 42) > 0) {
    dlt_user_log_write_string(&myctxdata, "ID: ");
    dlt_user_log_write_uint32(&myctxdata, 123);
    dlt_user_log_write_finish(&myctxdata);
}
```

Drawback of that solution is that the developer has to decide during
development if Verbose or Non-Verbose mode shall be used and the code most
likely ends up as written in the dlt-example-user application:

```
if (gflag) {
    /* Non-verbose mode */
    DLT_LOG_ID(ctx, DLT_LOG_INFO, 42 /* unique msg ID */, DLT_INT(num),
               DLT_STRING(text));
}
else {
    /* Verbose mode */
    DLT_LOG(ctx, DLT_LOG_INFO, DLT_INT(num), DLT_STRING(text));
}
```

##### Switching Verbose and Non-Verbose

To switch Verbose/Non-Verbose mode (Verbose mode is default), the following
APIs are available:

```
DLT_VERBOSE_MODE();
DLT_NONVERBOSE_MODE();
```

### Logging parameters

The following parameter types can be used. Multiple parameters can be added to
a single log message. The size of all logging parameters together should not
exceed 1390 bytes, including the DLT message header.

Type | Description
--- | ---
DLT\_STRING(TEXT) | String
DLT\_CSTRING(TEXT) | Constant String (not send in non-verbose mode)
DLT\_UTF8 | Utf8-encoded string
DLT\_RAW(BUF,LENGTH) | Raw buffer
DLT\_INT(VAR) | Integer variable, dependent on platform
DLT\_INT8(VAR) |Integer 8 Bit variable
DLT\_INT16(VAR) | Integer 16 Bit variable
DLT\_INT32(VAR) | Integer 32 Bit variable
DLT\_INT64(VAR) | Integer 64 bit variable
DLT\_UINT(VAR) | Unsigned integer variable
DLT\_UINT8(VAR) | Unsigned 8 Bit integer variable
DLT\_UINT16(VAR) |Unsigned 16 Bit integer variable
DLT\_UINT32(VAR) | Unsigned 32 Bit integer variable
DLT\_UINT64(VAR) | Unsigned 64 bit integer variable
DLT\_BOOL(VAR) | Boolean variable
DLT\_FLOAT32(VAR) | Float 32 Bit variable
DLT\_FLOAT64(VAR) | Float 64 Bit variable
DLT\_HEX8(UINT\_VAR) | 8 Bit hex value
DLT\_HEX16(UINT\_VAR) | 16 Bit hex value
DLT\_HEX32(UINT\_VAR) | 32 Bit hex value
DLT\_HEX64(UINT\_VAR) | 64 Bit hex value
DLT\_BIN8(UINT\_VAR) | 8 Bit binary value
DLT\_BIN16(UINT\_VAR | 16 Bit binary value
DLT\_PTR(PTR\_VAR) | Architecture independent macro to print pointers

### Network Trace

It is also possible to trace network messages. The interface, here
DLT\_NW\_TRACE\_CAN, the length of the header data and a pointer to the header
data, the length of the payload data and a pointer to the payload data, must be
specified. If no header or payload is available, the corresponding length must
be set to 0, and the corresponding pointer must be set to NULL.

```
DLT_TRACE_NETWORK(mycontext, DLT_NW_TRACE_CAN, headerlen, header, payloadlen, payload);
```

### DLT C++ Extension

The DLT C++ extension was added to DLT in version 2.13. This approach solves
the need to specify the type of each argument for applications written in C++
by using C++ templates and function overloading. The following shows the usage
of this API extension:

```
#define DLT_LOG_CXX(CONTEXT, LOGLEVEL, ...)
#define DLT_LOG_FCN_CXX(CONTEXT, LOGLEVEL, ...)

DLT_LOG_CXX(ctx, DLT_LOG_WARN, 1.0, 65);
DLT_LOG_FCN_CXX(ctx, DLT_LOG_WARN, "Test String", 145, 3.141);
```

This works as well with C++ standard containers like std::vector, std::map,
std::list. Of course, the logToDlt function can be overloaded to print user
defined structures or classes.

```
struct MyStruct
{
    int64_t uuid;
    int32_t interfaceId;
    int32_t registrationState;
};

template<>
inline int logToDlt(DltContextData & log, MyStruct const & value)
{
    int result = 0;

    result += dlt_user_log_write_string(&log, "(");
    result += logToDlt(log, value.uuid);
    result += dlt_user_log_write_string(&log, ",");
    result += logToDlt(log, value.interfaceId);
    result += dlt_user_log_write_string(&log, ",");
    result += logToDlt(log, value.registrationState);
    result += dlt_user_log_write_string(&log, ")");

    if (result != 0)
    {
        result = -1;
    }

    return result;
}
```

### Check if a specific Log Level is enabled

In some scenarios it might be necessary to check if a specific Log Level is
enabled or not, before log data is send to DLT Library. The macro is defined as
follows:

```
DLT_IS_LOG_LEVEL_ENABLED(CONTEXT,LOGLEVEL)
```

In general, there is no need to check the active Log Level to decide if a log
message can be send to not. This is handled inside the DLT\_LOG macro.

## DLT Injection Messages

DLT provides an interface to register injection callbacks which can be sent by
a DLT Client (e.g. DLT Viewer) to the application. An injection message
callback is always registered for a specific context. The API to register a
callback is defined as follows:

```
DLT_REGISTER_INJECTION_CALLBACK(CONTEXT, SERVICEID, CALLBACK);
```

Injection message Service IDs must be bigger than 0xFFF, because IDs up to
0xFFF are reserved for DLT Daemon control messages.  The callback function
needs to have the following definition:

```
int injection_callback(uint32_t service_id, void *data, uint32_t length);
```

For example, registering a callback function for a specific context with the
service ID 0x1000 might look like:

```
DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0x1000, injection_callback);
```

From DLT Viewer, an injection message can be sent by right-clicking the
corresponding context in the project view (“Send injection”). A dialog will pop
up to specify the injection data as shown below.

![alt text](images/dlt-viewer-send-injection-dialog.png "DLT Viewer Send Injection Callback")

## Log level changed callback

A callback function can be registered to be called whenever the Log Level of a
context changed. The usage is similar to DLT\_REGISTER\_INJECTION\_CALLBACK.

```
DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK(CONTEXT, CALLBACK)
```
