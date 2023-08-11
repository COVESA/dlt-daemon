# DLT Demo Setup
Back to [README.md](../README.md)

In this document you will run an instance of
dlt-daemon. It silently waits to collect and buffer log messages that are
produced by one or multiple DLT users. You will run one of those DLT users and
make it produce log messages that are sent to the daemon. Eventually, you launch
a client that collects and displays these messages.

*Note: We assume that you installed DLT (i.e. executed the [two optional steps
after build](../README.md#build-and-install)). Otherwise you have to take care
of the executable paths and explicitly state the library path.*

## Run the DLT Daemon
The DLT daemon is highly configurable but for this case the default settings are
okay. Don't be put off by warning messages:
```bash
$ dlt-daemon
[1886222.668006]~DLT~32290~NOTICE   ~Starting DLT Daemon; DLT Package Version: 2.18.0 STABLE, Package Revision: v2.18.1, build on Dec  8 2020 11:11:51
-SYSTEMD -SYSTEMD_WATCHDOG -TEST -SHM

[1886222.668651]~DLT~32290~INFO     ~FIFO size: 65536
[1886222.668764]~DLT~32290~INFO     ~Activate connection type: 5
[1886222.668897]~DLT~32290~INFO     ~dlt_daemon_socket_open: Socket created
[1886222.669047]~DLT~32290~INFO     ~dlt_daemon_socket_open: Listening on ip 0.0.0.0 and port: 3490
[1886222.669159]~DLT~32290~INFO     ~dlt_daemon_socket_open: Socket send queue size: 16384
[1886222.669355]~DLT~32290~INFO     ~Activate connection type: 1
[1886222.669509]~DLT~32290~INFO     ~Activate connection type: 9
[1886222.669644]~DLT~32290~INFO     ~Ringbuffer configuration: 500000/10000000/500000
[1886222.669924]~DLT~32290~NOTICE   ~Failed to open ECU Software version file.
[1886222.670034]~DLT~32290~INFO     ~Activate connection type: 6
[1886222.670188]~DLT~32290~INFO     ~Switched to buffer state for socket connections.
[1886222.670365]~DLT~32290~WARNING  ~dlt_daemon_applications_load: cannot open file /tmp/dlt-runtime-application.cfg: No such file or directory
```
The daemon opened a named pipe from which it is ready to read and buffer log
messages. It also accepts connections on TCP port 3490 by clients to collect
the messages.

## Produce Log Messages
A simulated ECU - a DLT user - will now use the DLT library to create log
messages and send them through the named pipe for the daemon to collect. Open a
second terminal and run
```bash
$ dlt-example-user -n 5 -l 3 "This is my first log message"
Send 0 This is my first log message
Log level changed of context TEST, LogLevel=4, TraceState=0
Log level changed of context TS1, LogLevel=4, TraceState=0
Log level changed of context TS2, LogLevel=4, TraceState=0
Send 1 This is my first log message
Client disconnected!
Send 2 This is my first log message
Send 3 This is my first log message
Send 4 This is my first log message
```
This will send 5 (```-n 5```) identical log messages of Log-Level
WARNING ```(-l 3)``` containing a string payload.

## Read logs
The DLT daemon now has the messages in its buffer and will keep them there until
they are fetched. A mighty tool for receiving and processing log messages is the
[DLT-Viewer](https://github.com/COVESA/dlt-viewer), which also provides a
graphical UI. For now, a simple command line client is absolutely sufficient:
```bash
$ dlt-receive -a localhost
2020/04/30 12:27:14.976731   17134987 000 ECU1 DA1- DC1- control response N 1 [service(3842), ok, 02 00 00 00 00]
2020/04/30 12:27:14.976779   17067139 000 ECU1 DA1- DC1- control response N 1 [service(3842), ok, 01 00 00 00 00]
2020/04/30 12:27:14.976787   17067139 004 ECU1 DLTD INTM log info V 1 [Client connection #7 closed. Total Clients : 0]
2020/04/30 12:27:14.976794   17104625 005 ECU1 DLTD INTM log info V 1 [ApplicationID 'LOG' registered for PID 5241, Description=Test Application for Logging]
2020/04/30 12:27:14.976802   17104625 000 ECU1 DA1- DC1- control response N 1 [get_log_info, 07, 01 00 4c 4f 47 00 01 00 54 45 53 54 ff ff 18 00 54 65 73 74 20 43 6f 6e 74 65 78 74 20 66 6f 72 20 4c 6f 67 67 69 6e 67 1c 00 54 65 73 74 20 41 70 70 6c 69 63 61 74 69 6f 6e 20 66 6f 72 20 4c 6f 67 67 69 6e 67 72 65 6d 6f]
2020/04/30 12:27:14.976823   17104625 000 ECU1 DA1- DC1- control response N 1 [get_log_info, 07, 01 00 4c 4f 47 00 01 00 54 53 31 00 ff ff 1b 00 54 65 73 74 20 43 6f 6e 74 65 78 74 31 20 66 6f 72 20 69 6e 6a 65 63 74 69 6f 6e 1c 00 54 65 73 74 20 41 70 70 6c 69 63 61 74 69 6f 6e 20 66 6f 72 20 4c 6f 67 67 69 6e 67 72 65 6d 6f]
2020/04/30 12:27:14.976844   17104625 000 ECU1 DA1- DC1- control response N 1 [get_log_info, 07, 01 00 4c 4f 47 00 01 00 54 53 32 00 ff ff 1b 00 54 65 73 74 20 43 6f 6e 74 65 78 74 32 20 66 6f 72 20 69 6e 6a 65 63 74 69 6f 6e 1c 00 54 65 73 74 20 41 70 70 6c 69 63 61 74 69 6f 6e 20 66 6f 72 20 4c 6f 67 67 69 6e 67 72 65 6d 6f]
2020/04/30 12:27:14.976866   17104588 000 ECU1 LOG- TEST log warn V 2 [0 This is my first log message]
2020/04/30 12:27:14.976872   17109592 001 ECU1 LOG- TEST log warn V 2 [1 This is my first log message]
2020/04/30 12:27:14.976880   17114599 002 ECU1 LOG- TEST log warn V 2 [2 This is my first log message]
2020/04/30 12:27:14.976884   17119607 003 ECU1 LOG- TEST log warn V 2 [3 This is my first log message]
2020/04/30 12:27:14.976889   17124611 004 ECU1 LOG- TEST log warn V 2 [4 This is my first log message]
2020/04/30 12:27:14.976894   17134988 006 ECU1 DLTD INTM log info V 1 [New client connection #8 established, Total Clients : 1]
2020/04/30 12:27:15.442016   17139641 000 ECU1 DA1- DC1- control response N 1 [service(3841), ok, 4c 4f 47 00 54 45 53 54 72 65 6d 6f]
2020/04/30 12:27:15.442044   17139642 007 ECU1 DLTD INTM log info V 1 [Unregistered ApID 'LOG']
```
The client connects to the default port 3490 of localhost to collect all
messages and interprets the payload as ASCII text (```-a```). You can see lots
of additional messages. These are control messages to control the flow between
client and daemon. You will learn about them later. For now, you have set up a
basic example have seen DLT in action.

You can now experiment with this setup. What happens if you start the DLT user
first and (while the DLT user is still running) the daemon?