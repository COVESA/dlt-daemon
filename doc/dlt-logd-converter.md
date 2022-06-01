# dlt-logd-converter

Android specific logging features of dlt are managed by the ```dlt-logd-converter``` process.
It currently contains the message forwarder from logd buffer to DLT.

The application listens to the system logs on Android (logd) and sends them to
DLT. The prerequisite is that dlt-daemon is already started and running.
dlt-logd-converter loads by default the configuration file ```/vendor/etc/dlt-logd-converter.conf```.

To change the log level, use application ID and context ID set in configuration
file. ```DLT_INITIAL_LOG_LEVEL``` can be set on startup phase, or control
message can be sent from DLT client (e.g. dlt-control, DLT Viewer) at runtime.
Refer to [dlt_for_developers.md](dlt_for_developers.md) for more detail.


## Usage

```bash
dlt-logd-converter [-h] [-c FILENAME]
```

-h

:   Display a short help text.

-c

:   Load an alternative configuration file. By default the configuration file
    */vendor/etc/dlt-logd-converter.conf* is loaded.


Non zero is returned in case of failure.


## Configuration

By default, the configuration is loaded from */vendor/etc/dlt-logd-converter.conf*. It
contains a few basic options:

- *ApplicationID*: this Android system log forwarder will have this applicationn ID
and appear on DLT client. Default value is **LOGD**.

- *ContextID*: the context ID of the above application to appear on DLT client.
Default value is **LOGF**.

- *AndroidLogdJSONpath*: the JSON file for MAIN buffer app logs should be looked up at
a defined path to use the context ID extension feature. If not found, the primitive context ID
**MAIN** is applied for all. Default path is */vendor/etc/dlt-logdctxt.json*.

- *AndroidLogdContextID*: If the JSON path is found, but applications are not listed in JSON file,
default context ID is **OTHE**.

### Log level mapping

As the severity levels on Android logs are different from DLT Log Level, the below table shows how they're converted from the former to the latter.


| Android System Log Level | DLT Log Level      |
|--------------------------|--------------------|
| ANDROID\_LOG\_DEFAULT    | DLT\_LOG\_DEFAULT  |
| ANDROID\_LOG\_UNKNOWN    | DLT\_LOG\_DEFAULT  |
| ANDROID\_LOG\_SILENT     | DLT\_LOG\_OFF      |
| ANDROID\_LOG\_FATAL      | DLT\_LOG\_FATAL    |
| ANDROID\_LOG\_ERROR      | DLT\_LOG\_ERROR    |
| ANDROID\_LOG\_WARN       | DLT\_LOG\_WARN     |
| ANDROID\_LOG\_INFO       | DLT\_LOG\_INFO     |
| ANDROID\_LOG\_DEBUG      | DLT\_LOG\_DEBUG    |
| ANDROID\_LOG\_VERBOSE    | DLT\_LOG\_VERBOSE  |


### Context Mapping

The json file dlt-logdctxt.json can be used to map android tags to dlt-contexts.

#### Example

Here is an example that sets a mapping from the tag "usbcore" to the DLT context "USBC",
and the tag "dummy_hcd.0" to the DLT Context "DUMM".
The description field is in the registration with libdlt.

```python
{
  "USBC": {
    "tag": "usbcore",
    "description": ""
  },
  "DUMM": {
    "tag": "dummy_hcd.0",
    "description": ""
  }
}
```
For the tags that are not listed, the mapping context ID should all be "OTHE".
