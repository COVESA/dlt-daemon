# dlt-qnx-system

QNX specific logging features of dlt are managed by the ```dlt-qnx-system``` process.
It currently contains the message forwarder from slogger2 to DLT.

The application listens to the system logs on QNX (slogger2) and sends them to
DLT. The prerequisite is that dlt-daemon is already started and running.
dlt-qnx-system loads by default the configuration file ```/etc/dlt-qnx-system.conf```.

> In order to catch all logs via slog2, the syslog needs to forward to slogger2 in [syslog.conf](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.utilities/topic/s/syslog.conf.html).

To change the log level, use application ID and context ID set in configuration
file. ```DLT_INITIAL_LOG_LEVEL``` can be set on startup phase, or control
message can be sent from DLT client (e.g. dlt-control, DLT Viewer) at runtime.
Refer to [dlt_for_developers.md](dlt_for_developers.md) for more detail.


## Usage

```bash
dlt-qnx-system [-h] [-d] [-c FILENAME]
```

-h

:   Display a short help text.

-d

:   Daemonize. Detach from Terminal and run in background.

-c

:   Load an alternative configuration file. By default the configuration file
    */etc/dlt-qnx-system.conf* is loaded.


Non zero is returned in case of failure.


## Configuration

By default, the configuration is loaded from */etc/dlt-qnx-system.conf*. It
contains a few basic options:

- *ApplicationId*: this QNX system log forwarder will have this application ID
  and appear on DLT client. Default value is *QSYM*.
- *ApplicationContextID*: the context ID of the above application to appear on
  DLT client. Default value is *QSYC*.
- *QnxSlogger2Enable*: when the value is 1 (by default), the QNX slogger2
  adapter which sends slogger2 to DLT will be started. Otherwise if the value
  if 0, it won't be started.
- *QnxSlogger2ContextId*: this will set the context ID of the QNX slogger2
  adapter. Default value is *QSLA*.
- *QnxSlogger2UseOriginalTimestamp*: when the value is 1 (by default), slogger2
  event timestamps will be used as DLT timestamps.



## Slogger2 adapter

The slogger2 adapter that can be enabled in dlt-qnx-system attaches to slogger2 and converts the messages to dlt messages.


### Log level mapping

As the severity levels on QNX system log are different from DLT Log Level, the below table shows how they're converted from the former to the latter.


| QNX System Log Level | DLT Log Level      |
|----------------------|--------------------|
| SLOG2\_SHUTDOWN      | DLT\_LOG\_FATAL    |
| SLOG2\_CRITICAL      | DLT\_LOG\_FATAL    |
| SLOG2\_ERROR         | DLT\_LOG\_ERROR    |
| SLOG2\_WARNING       | DLT\_LOG\_WARN     |
| SLOG2\_NOTICE        | DLT\_LOG\_INFO     |
| SLOG2\_INFO          | DLT\_LOG\_INFO     |
| SLOG2\_DEBUG1        | DLT\_LOG\_DEBUG    |
| SLOG2\_DEBUG2        | DLT\_LOG\_VERBOSE  |


### Context Mapping

The json file dlt-slog2ctxt.json can be used to map slogger names to dlt-contexts.

#### Example

Here is an example that sets a mapping from the "dumper" process to the DLT Context "DUMP", and the QNX Hypervisor to "QVM".
The description field is in the registration with libdlt.

```python
{
  "DUMP": {
    "name": "dumper",
    "description": "Writes core dump files to disk"
  },
  "SYSL": {
    "name": "syslogd",
    "description": ""
  }
}
```


### Runtime enable/disable slog2 adapter

The slog2 parser callback can be disabled and reenabled at runtime by using an injection.

| App ID | Context ID | Service ID | Data | Description            |
|--------|------------|------------|------|------------------------|
| QSYM   | QSYC       | 0x1000     | 00   | Disable slog2 adapter  |
| QSYM   | QSYC       | 0x1000     | 01   | Reenable slog2 adapter |

### Example:

```bash
dlt-control -e QNX -a QSYM -c QSYC -s 4096 -m "00" -u
dlt-control -e QNX -a QSYM -c QSYC -s 4096 -m "01" -u
```
