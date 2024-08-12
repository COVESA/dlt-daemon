- [Trace load limits](#trace-load-limits)
  - [Overview](#overview)
  - [Technical concept](#technical-concept)

# Trace load limits

## Overview

In a shared system, many developers log as much as they want into DLT. It can overload the logging system, resulting in poor performance and dropped logs. This feature introduces trace load limits to restrict applications to a specified log volume measured in bytes/s. 

Trace load limits are configured via a space-separated configuration file.
The format of the file follows:

> APPID [CTXID] SOFT_LIMIT HARD_LIMIT

The entry with the most matching criteria will be selected for each log, meaning the app and context or the app ID must match. For non-configured applications, default limits are applied.
It allows configuring trace load for single contexts, which can be used, for example, to limit different applications in the QNX slog to a given budget without affecting others or to provide a file transfer unlimited bandwidth.
You should always specify a budget for the application ID without the contexts to ensure that new contexts and internal logs like DLTL can be logged.

Applications start with a default limit defined via CMake variables TRACE_LOAD_USER_HARD_LIMIT, TRACE_LOAD_USER_SOFT_LIMIT. 
Once the connection to the daemon is established, each configuration entry matching the app ID will be sent to the client via application message. If no configuration is found, TRACE_LOAD_DAEMON_HARD_LIMIT and TRACE_LOAD_USER_SOFT_LIMIT will be used instead.
The two-stage configuration process ensures that the daemon default can be set to 0, forcing developers to define a limit for their application while ensuring that applications can log before they receive the log levels.

Measuring the trace load is done in the daemon and the client. 
Dropping messages on the client side is the primary mechanism and prevents sending logs to the daemon only to be dropped there, 
reducing the IPC's load. Measuring again on the daemon side ensures that rouge clients are still limited to the defined trace load.

Exceeding the soft limit will produce the following logs:

```
ECU1- DEMO DLTL log warn V 1 [Trace load exceeded trace soft limit on apid: DEMO. (soft limit: 2000 bytes/sec, current: 2414 bytes/sec)] 
ECU1- DEMO DLTL log warn V 1 [Trace load exceeded trace soft limit on apid: DEMO, ctid TEST.(soft limit: 150 bytes/sec, current: 191 bytes/sec)]
```

Exceeding the hard limit will produce the same message but with the appended text '42 messages discarded.' And (obviously) messages will be dropped. Dropped messages are lost and cannot be recovered, which forces developers to get their logging volume under control.

As debug and trace load are usually disabled for production and thus do not impact the performance of actual systems, these logs are not accounted for in trace load limits. In practice, this was very useful for improving the developer experience while maintaining good performance, as developers know that debugging and trace logs are only available during development and that essential information has to be logged at a higher level.

To simplify creating a trace limit baseline, the script 'utils/calculate-load.py' is provided, which suggests limits based on actual log volume.

## Technical concept

The bandwidth is calculated based on the payload of each message of every application or context. Each limit gets assigned several slots (default 60s) with a width of 1s, called a window. The sum of all slots represents the current bandwidth. 

The `dlt_check_trace_load` function is the main entry point for handling trace load. It receives the payload size and the trace load configuration (among other parameters, but these are the most important) and returns whether the message should be kept. A return value of `true` means the message can be logged, while `false` means it should be dropped. This method is called for each message received by the daemon and for each message sent by the client. Doing this on the client side ensures that the daemon is not overloaded by clients sending too much data. Checking it on the daemon side ensures that rouge clients are still limited to the defined trace load.

Within the `dlt_check_trace_load` function, the first step is to check if the current slot is still valid by checking its time frame. It is done in the `dlt_switch_slot_if_needed` function, which also does sanity checks and cleans the window if necessary.

Cleaning does the following things:

1. Check if a timestamp overflow occurred, which will happen after ~119 hours. In this case, the time calculation will be reset.
2. Calculate the number of elapsed slots since receiving the last message.
3. The window is reset if the calculated amount of elapsed slots is larger than the window size (`total_bytes_of_window`).
4. Otherwise, remove data from slots that are not in the current window anymore.
5. Output errors when the limits are exceeded.

After the preparation, the actual recording of the trace load is calculated. The calculation is done in the following way:

1. Add the payload size to the current slot.
2. Increment the total size of the window.
3. Save the current slot to the window so that we can use this in the next iteration.
4. Calculate the average trace load of the window. It is an average in byte/s over the window size. The average value is used to ensure that short spikes do not immediately lead to dropped messages.

The trace load configuration contains two flags, `is_over_soft_limit` and `is_over_hard_limit`. When the limits are exceeded, they will be set.
If a new message exceeds the hard limits, it will be dropped, and its size will be removed from the window as the message is dropped.
It will also increment the `hard_limit_over_counter`, which counts the number of dropped messages. `hard_limit_over_counter` is a part of the message logged in case of exceeding the hard limit.