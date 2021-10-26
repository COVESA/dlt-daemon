# How to DLT for developers

Back to [README.md](../README.md)

Table of Contents
1. [DLT Example Application](#DLT-Example-Application)
2. [General Rules for Logging](#General-Rules-for-Logging)
3. [The use of Log Levels](#The-use-of-Log-Levels)
4. [DLT Library Runtime Configuration](#DLT-Library-Runtime-Configuration)
5. [DLT API Usage](#DLT-API-Usage)
6. [DLT injection messages](#DLT-Injection-Messages)
7. [Log level changed callback](#Log-level-changed-callback)
8. [Disable injection messages](#Disable-injection-messages)
9. [Use DLT in library](#Use-DLT-in-library)

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

To use DLT with CMake, the recommended way is to use the CMake Config file
that is being generated as part of installation.

You can thus:
```
find_package(automotive-dlt REQUIRED)
...
target_link_libraries(myapp PRIVATE Genivi::dlt)
```
which lets your project automatically gain all necessary compile and link flags
needed by libdlt, including the include directories.

The generated CMake Config file follows "Modern CMake" convention and only
exports an IMPORTED CMake target; it does not set any variables, except for the
`automotive-dlt_FOUND` variable that can be used to treat DLT as an optional
dependency.

The generated CMake config file (which is implicitly being used when you call
`find_package(automotive-dlt)`) by default only adds the top-level directory
to the compiler's header search path; this requires that users' #include
directives are written in the regular form e.g. `<dlt/dlt.h>`. If you want
to be able to use the legacy form `<dlt.h>` as well (as is always allowed by
the pkg-config module for backwards compatibility reasons), you can configure
DLT with the CMake option `-DWITH_LEGACY_INCLUDE_PATH=On` in order to
achieve that.

### DLT with pkg-config

Alternatively to the CMake integration detailed above, it is also possible
to use DLT via pkg-config. This can also be done with CMake's PkgConfig
module as well.

#### PkgConfig usage with "Modern CMake"

Here, you let the PkgConfig module create targets as well; the target's name
is however determined by the PkgConfig module:

```
find_package(PkgConfig)
pkg_check_modules(DLT REQUIRED IMPORTED_TARGET automotive-dlt)
```

As per "Modern CMake", there are again no variables to be added, but only
a CMake target to be added to the link libraries:

```
target_link_libraries(myapp PRIVATE PkgConfig::DLT)
```

#### PkgConfig usage with "Legacy CMake" (<3.0)

Here, you let the PkgConfig module only create variables, but not targets:

```
find_package(PkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)
```

to INCLUDE\_DIRECTORIES (or, since CMake 2.8.11, TARGET\_INCLUDE\_DIRECTORIES), add

```
${DLT_INCLUDE_DIRS}
```

and to TARGET\_LINK\_LIBRARIES:

```
${DLT_LINK_LIBRARIES}  (preferred, for CMake >= 3.12)
${DLT_LIBRARIES}       (otherwise)
```

The contents of `${DLT_LIBRARIES}` do not include the library's path
(e.g. `-L/path/to/lib`), so if the library resides in a location that is not
on the linker's default search path, you'll either have to add that path
to LINK\_DIRECTORIES:
```
link_directories(${DLT_LIBRARY_DIRS})
```
or, alternatively, not use `${DLT_LIBRARIES}`, but `${DLT_LDFLAGS}` instead,
which combines `${DLT_LIBRARIES}` and `${DLT_LIBRARY_DIRS}`:
```
target_link_libraries(myapp ${DLT_LDFLAGS})
```

### Limitation

On Android, definition of `SIGUSR1` in DLT application shall be avoided since
DLT library blocks `SIGUSR1` to terminate housekeeper thread at exit.

## General Rules for Logging

### Be Smart

Before implementing logging in code one should take a second to think about a
concept first. Often strategic places in the software can be used as a central
place for logging. Such places are often interfaces to other SW components. Use
the solution with the smallest impact. Avoid logging the "good cases" but log
e.g. in your error handling sections – you will need error handling anyway. In
case an error occurred more logs don't matter as long as your regular code
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

For example don't write log entries like this:

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

Information should be "on your fingertips". Logging is a tool to ease crushing
bugs, not to win a computer art contest. → Don't use ASCII Art!

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
int index = 0;
for(; index<MAX; index++)
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
Avoid eye catchers | Please don't use custom highlighting for marking the messages which are important for you. The best way is to write a clear identifier in the front of your trace message. This can than (in the viewer) be easily used for filter sets or even for coloring. An example for messages informing about the frame rate could be: **Frame rate**: low: 12, high: 34, avr: 28
Do not log to the console | Do not use \_printf()\_ or similar statements to trace to the console. Such behavior can make it problematic to work e.g. with the serial console or even might slow down the execution of the program. In a vehicle based test system the console messages are not recorded and will not help you.
Do NOT "macrotize" dlt macros | Don't write your own macros to capsulate DLT macros.

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

Please be aware the default Log Level is set to INFO; this means message
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
- A system essential file can't be read or written
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

The default log level of DLT User library is DLT\_LOG\_INFO (when using macro
DLT\_REGISTER\_CONTEXT or dlt\_register\_context() api). This can be changed
using a DLT client application (e.g. DLT Viewer). But there might be situations
where DEBUG or VERBOSE messages are needed before the DLT Daemon updated the user library.

There are several ways to initialize log level in DLT library startup phase.
 
1. It is possible to do that by exporting environment variable DLT\_INITIAL\_LOG\_LEVEL.
   By using this way, the user can specify log level for contexts.

For example, an application "EXA1" has two contexts "CON1" and "CON2". For
"CON1" log level DEBUG and for "CON2" log level VERBOSE shall be used. The
following has to be exported to configure the library:

> export DLT\_INITIAL\_LOG\_LEVEL="EXA1:CON1:5;EXA1:CON2:6"

If the log level of all applications and contexts shall be initialized, then:

> export DLT\_INITIAL\_LOG\_LEVEL="::2"

If the log level for all contexts of application "EXA1" shall be initialized, then:

> export DLT\_INITIAL\_LOG\_LEVEL="EXA1::2"

If the log level of context "CON1" shall be initialized, then:

> export DLT\_INITIAL\_LOG\_LEVEL=":CON1:2"

In case only the log level of context "CON1" of application "EXA1" shall be
initialized, and other contexts will be ignored, then:

> export DLT\_INITIAL\_LOG\_LEVEL="::0;EXA1:CON1:2"

2. If DLT\_INITIAL\_LOG\_LEVEL variable is not exported in the environment,
log level for it each context can be changed in the config file (/etc/dlt.conf).

Default log level will be 4 (DLT\_LOG\_INFO)

> ContextLogLevel = 4

3. DLT user can use dlt\_register\_context\_ll\_ts() api to initialize log
level for each context.

Example:
> //Register new context to daemon, with initial log level is DLT\_LOG\_VERBOSE
>
> dlt\_register\_context\_ll\_ts(&con\_exa1, "CON", "First context", DLT\_LOG\_VERBOSE, DLT\_TRACE\_STATUS\_OFF);

The priority of context log level would be as follows:
- Priority 1: Using dlt\_register\_context\_ll\_ts() api
- Priority 2: Using environment variable DLT\_INITIAL\_LOG\_LEVEL
- Priority 3: Setting in config file dlt.conf

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

### Get application ID

To get the application ID value, requested to allocate a char array at least
4 byte length and input to the function call.

The application ID will be stored in this input char array.

#### MACRO

```
DLT_GET_APPID(appid);
```

#### Function

```
dlt_get_appid(appid);
```

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

DLT provides functions that allow for flexible construction of messages
with an arbitrary number of arguments. Both Verbose and Non-Verbose
messages are supported, with different APIs. Sending a message using
these functions require multiple function calls, for starting message
construction, adding the arguments, and sending off the message.

The following table shows an example of all 4 types for logging
using a constant string and an integer.

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

#### Statefulness of Verbose/Non-Verbose

The library uses a global state that applies to all logging commands being used.
If the library is in global "Verbose" mode, both the "Verbose" and the "Non-Verbose"
API calls shown above will always result in Verbose messages being sent.

However, if the library is in global "Non-Verbose" mode, it is possible to send
both Verbose and Non-Verbose messages in a single session; all "Verbose" APIs will
send Verbose messages, and all "Non-Verbose" APIs will send Non-Verbose messages.

It does not make sense to send a Non-Verbose message via a Verbose API, as there
is then no sensible message ID, which is however mandatory when sending
Non-Verbose messages.

#### Switching Verbose and Non-Verbose

To switch Verbose/Non-Verbose mode (Verbose mode is default), the following
APIs are available:

##### MACRO

```
DLT_VERBOSE_MODE();
DLT_NONVERBOSE_MODE();
```

##### Function

```
dlt_verbose_mode();
dlt_nonverbose_mode();
```

#### String arguments

For string arguments, you can choose between ASCII and UTF-8 encoding. This
encoding is written into the argument header, so that the receiver knows what to
expect when parsing the message.

In addition, you can also choose between null-terminated strings and (potentially)
non-null-terminated ones.
The latter often arise in C++ code when using classes such as std::string\_view,
which does not provide an accessor method that returns a null-terminated C-string.
Copying data out of a std::string\_view thus requires to copy only the desired
number of characters.

For instance, using Verbose mode:

```
std::string_view line = "Name: Ford Prefect";
std::string_view key = line.substr(0, 4);
std::string_view value = line.substr(6);

if (dlt_user_log_write_start_id(&ctx, &ctxdata, DLT_LOG_INFO, 42) > 0) {
    dlt_user_log_write_constant_utf8_string(&myctxdata, "key");
    dlt_user_log_write_sized_utf8_string(&myctxdata, key.data(), key.size());
    dlt_user_log_write_constant_utf8_string(&myctxdata, "value");
    dlt_user_log_write_sized_utf8_string(&myctxdata, value.data(), value.size());
    dlt_user_log_write_finish(&myctxdata);
}
```

#### Using custom timestamps

The timestamp that is transmitted in the header of a DLT message is usually generated automatically by the library itself right before the message is sent. If you wish to change this, e.g. because you want to indicate when an event occured, rather than when the according message was assembled, you can supply a custom timestamp. Compared to the example above, two macros are defined for convenience:

```
uint32_t timestamp = 1234567; /* uptime in 0.1 milliseconds */
if (gflag) {
    /* Non-verbose mode */
    DLT_LOG_ID_TS(ctx, DLT_LOG_INFO, 42, timestamp,
                  DLT_INT(num), DLT_STRING(text));
}
else {
    /* Verbose mode */
    DLT_LOG_TS(ctx, DLT_LOG_INFO, timestamp,
               DLT_INT(num), DLT_STRING(text));
}
```

If you wish to (or have to) use the function interface, you need to set the flag to make use of the user-supplied timestamp manually after calling dlt_user_log_write_start():


```
if (dlt_user_log_write_start(&ctx, &ctxdata, DLT_LOG_INFO) > 0) {
    ctxdata.use_timestamp = DLT_USER_TIMESTAMP;
    ctxdata.user_timestamp = (uint32_t) 1234567;
    dlt_user_log_write_string(&myctxdata, "ID: ");
    dlt_user_log_write_uint32(&myctxdata, 123);
    dlt_user_log_write_finish(&myctxdata);
}
```

#### Send log message with given buffer

DLT applications can prepare a log message buffer by themselves instead
of calling logging parameters. There are two benefits; the applications
can reduce API calls to DLT library as much as possible so that the
APIs won't block the application's sequence, and dynamic allocation can
be avoided in DLT library during runtime.

The applications should prepare following values in order to use this
functionality:

- *char buffer[DLT_USER_BUF_MAX_SIZE]*: Buffer which contains one log message payload
- *size_t size*: Buffer size
- *int32_t args_num*: Number of arguments

One argument in the buffer consists of following:
| Length(byte)   | Description  |
|----------------|--------------|
| 4              | Type Info    |
| x              | Data Payload |

DLT Applications need to simulate what are done in logging parameters to
store data to the buffer (type info given to the buffer, etc.),
otherwise the behavior is undefined.

Also important note here is that the functionality works properly only
with these function combination:

- *Start logging*: dlt\_user\_log\_write\_start\_w\_given\_buffer
- *Finish logging*: dlt\_user\_log\_write\_finish\_w\_given\_buffer

Since the function does not allocate memory dynamically, it could lead
to segmentation fault or memory leak with different APIs. It is mandatory
to check if dlt_user_is_logLevel_enabled is returning DLT_RETURN_TRUE before
calling dlt_user_log_write_start_w_given_buffer (see below code example).

##### Macro

No macro interface is available as of now.

##### Function

```
/* Example: Prepare one log message with uint16 */
char buffer[DLT_USER_BUF_MAX_SIZE] = {0};
size_t size = 0;
int32_t args_num = 0;

uint32_t type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT;
memcpy(buffer + size, &(type_info), sizeof(uint32_t));
size += sizeof(uint32_t);

uint16_t data = 1234;
memcpy(buffer + size, &data, sizeof(uint16_t));
size += sizeof(uint16_t);

args_num++;

/* Give the buffer to DLT library */
if (dlt_user_is_logLevel_enabled(&ctx,DLT_LOG_INFO) == DLT_RETURN_TRUE)
{
  if (dlt_user_log_write_start_w_given_buffer(&ctx, &ctxdata, DLT_LOG_INFO, buffersize, args_num) > 0)
  {
      dlt_user_log_write_finish_w_given_buffer(&ctxdata);
  }
}
```

#### Attributes

In verbose mode, log message arguments can contain attributes. A "name" attribute
describes the purpose or semantics of an argument, and a "unit" attribute
describes its unit (if applicable - not all argument data types support having
a "unit" attribute).

```
dlt_user_log_write_float64_attr(&myctxdata, 4.2, "speed", "m/s");
```

In non-verbose mode, these attributes are not added to the message.

### Logging parameters

The following parameter types can be used. Multiple parameters can be added to
a single log message. The size of all logging parameters together should not
exceed 1390 bytes, including the DLT message header.

Type | Description
--- | ---
DLT\_STRING(TEXT) | String
DLT\_STRING\_ATTR(TEXT,NAME) | String (with attribute)
DLT\_SIZED\_STRING(TEXT,LENGTH) | String with known length
DLT\_SIZED\_STRING\_ATTR(TEXT,LENGTH,NAME) | String with known length (with attribute)
DLT\_CSTRING(TEXT) | Constant string (not sent in non-verbose mode)
DLT\_CSTRING\_ATTR(TEXT,NAME) | Constant string (with attribute; not sent in non-verbose mode)
DLT\_SIZED\_CSTRING(TEXT,LENGTH) | Constant string with known length (not sent in non-verbose mode)
DLT\_SIZED\_CSTRING\_ATTR(TEXT,LENGTH,NAME) | Constant string with known length (with attribute; not sent in non-verbose mode)
DLT\_UTF8(TEXT) | Utf8-encoded string
DLT\_UTF8\_ATTR(TEXT,NAME) | Utf8-encoded string (with attribute)
DLT\_SIZED\_UTF8(TEXT,LENGTH) | Utf8-encoded string with known length
DLT\_SIZED\_UTF8\_ATTR(TEXT,LENGTH,NAME) | Utf8-encoded string with known length (with attribute)
DLT\_RAW(BUF,LENGTH) | Raw buffer
DLT\_RAW\_ATTR(BUF,LENGTH,NAME) | Raw buffer (with attribute)
DLT\_INT(VAR) | Integer variable, dependent on platform
DLT\_INT\_ATTR(VAR,NAME,UNIT) | Integer variable, dependent on platform (with attributes)
DLT\_INT8(VAR) |Integer 8 Bit variable
DLT\_INT8\_ATTR(VAR,NAME,UNIT) |Integer 8 Bit variable (with attributes)
DLT\_INT16(VAR) | Integer 16 Bit variable
DLT\_INT16\_ATTR(VAR,NAME,UNIT) | Integer 16 Bit variable (with attributes)
DLT\_INT32(VAR) | Integer 32 Bit variable
DLT\_INT32\_ATTR(VAR,NAME,UNIT) | Integer 32 Bit variable (with attributes)
DLT\_INT64(VAR) | Integer 64 bit variable
DLT\_INT64\_ATTR(VAR,NAME,UNIT) | Integer 64 bit variable (with attributes)
DLT\_UINT(VAR) | Unsigned integer variable
DLT\_UINT\_ATTR(VAR,NAME,UNIT) | Unsigned integer variable (with attributes)
DLT\_UINT8(VAR) | Unsigned 8 Bit integer variable
DLT\_UINT8\_ATTR(VAR,NAME,UNIT) | Unsigned 8 Bit integer variable (with attributes)
DLT\_UINT16(VAR) |Unsigned 16 Bit integer variable
DLT\_UINT16\_ATTR(VAR,NAME,UNIT) |Unsigned 16 Bit integer variable (with attributes)
DLT\_UINT32(VAR) | Unsigned 32 Bit integer variable
DLT\_UINT32\_ATTR(VAR,NAME,UNIT) | Unsigned 32 Bit integer variable (with attributes)
DLT\_UINT64(VAR) | Unsigned 64 bit integer variable
DLT\_UINT64\_ATTR(VAR,NAME,UNIT) | Unsigned 64 bit integer variable (with attributes)
DLT\_BOOL(VAR) | Boolean variable
DLT\_BOOL\_ATTR(VAR,NAME) | Boolean variable (with attribute)
DLT\_FLOAT32(VAR) | Float 32 Bit variable
DLT\_FLOAT32\_ATTR(VAR,NAME,UNIT) | Float 32 Bit variable (with attributes)
DLT\_FLOAT64(VAR) | Float 64 Bit variable
DLT\_FLOAT64\_ATTR(VAR,NAME,UNIT) | Float 64 Bit variable (with attributes)
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

Note that when DLT_NETWORK_TRACE_ENABLE is disabled, the mqueue.h will not be
included.

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
corresponding context in the project view ("Send injection"). A dialog will pop
up to specify the injection data as shown below.

![alt text](images/dlt-viewer-send-injection-dialog.png "DLT Viewer Send Injection Callback")

## Log level changed callback

A callback function can be registered to be called whenever the Log Level of a
context changed. The usage is similar to DLT\_REGISTER\_INJECTION\_CALLBACK.

```
DLT_REGISTER_LOG_LEVEL_CHANGED_CALLBACK(CONTEXT, CALLBACK)
```

## Disable injection messages

An environment variable named `DLT_DISABLE_INJECTION_MSG_AT_USER` could be used in case
dlt application wants to ignore all data/messages from dlt-daemon completely.

To use:

```
export DLT_DISABLE_INJECTION_MSG_AT_USER=1
```

To clear:

```
unset DLT_DISABLE_INJECTION_MSG_AT_USER
```

## Use DLT in library

There are cases where a library wants to use DLT interface to output its log message.
In such case, applications and contexts can be registered using following way.

### Application registration

The library can check if an application is already registered or not by
`DLT_GET_APPID()` API. If returned application ID is not NULL, it can be considered
that application was already registered previously. If it's NULL, then application
can be registered.

```Example
// Check if an application is already registered in this process
char appid[DLT_ID_SIZE];
DLT_GET_APPID(&appid);
if (appid[0] != '\0')
{
  printf("Application is already registered with AppID=[%s]\n", appid);
}
else
{
  DLT_REGISTER_APP("APP", "Application for library xxx");
}
```

### Context registration

The same context ID can be used among different applications, so context can be
registered as usual.
