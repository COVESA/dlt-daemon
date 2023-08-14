# Diagnostic Log and Trace  - Release Notes

Back to [README.md](../README.md)

## Version

2.18.10 STABLE

## Changes

### 2.18.10
## What's Changed
   * Update ReleaseNotes and version to v2.18.9 by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/468
   * Update status badges on README.md by @michael-methner in https://github.com/COVESA/dlt-daemon/pull/467
   * logstorage: Adds option to write logs in gzip format by @LiquidityC in https://github.com/COVESA/dlt-daemon/pull/442
   * Re-Initialize internal logging in daemon mode only by @lvklevankhanh in https://github.com/COVESA/dlt-daemon/pull/459
   * Remove use of DLT_LOG in signal handler by @michael-methner in https://github.com/COVESA/dlt-daemon/pull/472
   * Update gtest_dlt_daemon_multiple_files_logging.cpp by @LocutusOfBorg in https://github.com/COVESA/dlt-daemon/pull/481
   * Update AUTOSAR standard link by @lucafrance in https://github.com/COVESA/dlt-daemon/pull/480
   * cmake: set version to 2.18.9 by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/478
   * Fix macro code  to use boolean value in while instruction (false) by @michael-methner in https://github.com/COVESA/dlt-daemon/pull/469
   * dlt_user_shared: Add timeout to writev by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/385
   * build: add static lib only if necessary by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/479
   * Update CMakeLists.txt by @LocutusOfBorg in https://github.com/COVESA/dlt-daemon/pull/482
   * watchdog: improve dlt watchdog by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/470
   * log-level-config: add option to configure log levels by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/474
   * gtest_dlt_daemon_gateway: dlt-daemon run without dlt_passive.conf file by @lti9hc in https://github.com/COVESA/dlt-daemon/pull/487
   * dlt_unit_test: Fix and improve quality of unit tests by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/494
   * dlt-qnx-system improvement by @lvklevankhanh in https://github.com/COVESA/dlt-daemon/pull/495
   * gtest: Init submodule and update version by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/497
   * dlt-qnx-system: prevent message loss in high load situations by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/490
   * Fix compile error stringop-truncation with GCC 9.4 by @michael-methner in https://github.com/COVESA/dlt-daemon/pull/499
   * Update README.md by @lvklevankhanh in https://github.com/COVESA/dlt-daemon/pull/503
   * dlt-coverage: Add coverage report generator for dlt by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/501
   * dlt-system: move journal reading to its own thread by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/471
   * Switch from GENIVI to COVESA by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/511
   * Update CMakeLists.txt: set required std version to gnu++14 by @LocutusOfBorg in https://github.com/COVESA/dlt-daemon/pull/504
   * cmake: Policy CMP0115 set to OLD behavior for dlt-daemon with cmake >= 3.20 by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/510
   * fix usage of pthread_cond_timedwait by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/491
   * DLT Upstream for minor release by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/515
   * cmake: disable network trace by @alexmohr in https://github.com/COVESA/dlt-daemon/pull/477
   * doc: add COVESA logo image by @minminlittleshrimp in https://github.com/COVESA/dlt-daemon/pull/516

   **Full Changelog**: https://github.com/COVESA/dlt-daemon/compare/v2.18.9...v2.18.10

### 2.18.9

   * dlt-user: fix crash with certain strings (#463)
   * dlt_multiple_files: remove superfluous mode bits and add header file to header list (#462)
   * Android: Add new feature in Android bp (#461)
   * cmake: remove duplicated option message (#454)
   * house-keeper: remove infinite wait (#438)
   * dlt-logd-converter: Fix getting log level from log msg (#456)
   * dlt-logd-converter: fixes android 12 compilation (#445)
   * logfile: exhance internal dlt logging by introducing size limits (#369)
   * This changes a mispatch from fcb676a7 to install the udp binary correctly. (#449)
   * Installs dlt.conf on android (#446)
   * dlt-connection: add socket timeout (#439)
   * Fix memory leak (#441)
   * Check for negative index in dlt_file_message (#437)
   * dlt-user: fix potential non closed socket in init/free (#435)
   * dlt-convert: Fix memory leak by calling dlt_file_free (#434)
   * dlt-user: Fix crashes in dlt_free during dlt_init (#362)
   * Update contacts and removed mailing lists (#431)
   * Updates for Coding Styles (#425)
   * gateway: Fix Node handling and ECUid checks (#429)
   * filetransfer: fix filesize divisible by blocksize case (#383)
   * client: Fix Get Log Info response conversion method (#422)
   * cmake: network trace enable toggle (#424)
   * dlt-system: Fix buffer overflow detection on 32bit targets (#398)
   * dlt-receive: set host interface and allow multiple udp multicast addresses (#420)
   * Fix for Resource and Memory Leak (#418)
   * dlt_daemon_client: Fix Control Msg ECUId comparison with active Gateway (#414)
   * Avoid memory corruption behind buffer wp in function dlt_getloginfo_conv_ascii_to_id (#411)
   * dlt_common: change output of message for log initialization (#412)
   * internal-logging: Fix issues with file logging (#378)
   * systemd: add support for socket activation via systemd (#401)
   * Update maintainer (#410)
   * dlt_daemon_client: Fix change loglevel of application (#408)
   * dlt_client:Block in connect() (#409)
   * dlt-gateway: Fix crash on invalid ip (#381)
   * Update dlt_for_developers.md (#405)
   * logstorage: Truncate ECUid in Logstorage filter to prevent crash (#402)
   * dlt_common.c: Change default logging_mode (#406)
   * dlt-daemon-connection: Start up even if not all bindings are valid (#380)
   * enforce-trace-limit: ContextLogLevel is now enforced in the daemon (#382)
   * automotive-dlt.pc: add the path to find the static library (#387)
   * systemd: install adaptor-udp service for adaptor=on (#393)
   * Fix handle returned value (#384)
   * README: Update link to github actions (#392)
   * Update for CI (#389)
   * Fix a double-free bug. (#376)
   * Issue-ID: make-adaptor-configurablecmake: Add option to enable each adaptor by itself (#364)
   * Fix the target name in documentation (#372)
   * cmake: Add options to enable/disable each dlt console tool (#363)
   * filetransfer: Fix getFileCreationDate2 stat check (#361)
   * tests: Deplicate unused files and variables (#359)
   * Fix DLT User/Client tests (#357)
   * lib: Correct VARI usage in dlt_user_log_write_uint (#356)
   * filetransfer: Return error if no free space (#354)
   * Support for Cygwin toolchain. (#351)
   * dlt-system: fix invalid free by removing unused TempDir (#350)
   * fix -Wformat issues reported by clang (#349)
   * Forcibly the severity level set (#346)
   * daemon: Do not exit when accept returns ECONNABORTED (#347)
   * dlt-system : fix invalid free with ConfigurationFileName (#342)
   * dlt-daemon: Only create directories if they do not exist yet (#340)
   * fixes compilation issue with clang (#339)
   * dlt-daemon: create sockets using "android way" (#333)
   * dlt-system: fix a libc buffer overflow detection on 32bit targets (#337)

### 2.18.8

   * lib: Fix wrong type alert from lgtm
   * gtest_dlt_daemon_gateway: fix gtest build failed
   * lib: generate dlt library internal log file
   * tests: add stdlib to dlt_cpp_extension
   * dlt_user: Make dlt_init thread safe
   * remove clang-tidy analyzer warnings: incompatible pointer type
   * debian: improve debian build package
   * dlt-control-common: shutdown and close socket
   * dlt_common: improve function description
   * gtest: Bring-in changes
   * gtest: Rework WORKING_DIRECTORY
   * gtest: Correct data amount in gtest_dlt_common
   * cmake: Set empty to systemd_SRCS
   * gtest: Refactor tests/CMakeLists.txt
   * lib: Add SOCK_CLOEXEC to socket
   * daemon: Create parent directory for unix socket
   * cmake: Correct added subdirectories
   * console: Add cmake options for control and timestamp
   * tests: Adapt to DLT_DISABLE_MACRO
   * header: Adapt to DLT_DISABLE_MACRO
   * include: Refactor CMakeLists.txt
   * gtest: Change script name to gtest_dlt_daemon_offline_log.sh
   * daemon: Enable to use FIFO on QNX
   * tests: Add new test case with given buffer
   * tests: Enable macro disabling
   * lib: Add new interfaces with given buffer
   * Implemention of tests for the dlt-qnx-system module
   * lib: Add MaxFileSize handling
   * client: pthread_join for deinit
   * doc: update initial log level document
   * dlt-system: Fix memory leak in dlt-system config
   * dlt_common: remove duplicate stdbool header
   * dlt-control: Add option to config port
   * system: use signalfd for dlt-system
   * console: provides args option to enable send/receive serial header
   * fix malformed printf format strings (#295)
   * cmake: Set WITH_LEGACY_INCLUDE_PATH to ON as default (#334)
   * Make the legacy include path a CMake option (#332)
   * daemon: Call dlt_daemon_configuration_load() properly (#330)
   * dlt_user: Use pthread_setname_np() if available (#326)
   * libdlt: Add legacy include path in exported CMake config file (#327)
   * lib: Set TYLE to 1 for BOOL type (#320)
   * file-transfer: Abort file transfer if get serial number failed
   * dlt_user.c: fixing casting wrong type
   * dlt-sortbytimestamp: Remove duplicated conditional statements code
   * dlt-convert: Remove duplicated conditional statements code
   * doc: Minor fix in dlt_for_developers.md (#321)
   * dlt-control-common.c: Fix build failure due to out-of-bound write -Werror=stringop-truncation
   * Extend include path in *.pc file (#319)

### 2.18.7

   * dlt_common: correct read/write position
   * Update document and fix conversion warning
   * dlt_common: Increment size after memcpy()
   * Using dlt_vlog in verbose option in dlt_client.c
   * dlt-logstorage-ctrl: Improve option force to sync
   * Do not use Cpack
   * Debian packaging
   * Update dlt-system.conf documentation
   * dlt_user: correct handling return value
   * Update dlt-system-syslog with IPv6
   * Alternative solutions for json-c dependency
   * dlt-receive:Fix compiler warning
   * daemon: Improve signal handler for timers
   * dlt-doc:Fix generating HTML documentation and man pages
   * Apply uncrustify for src/daemon/* and src/lib/*
   * Update uncrustify
   * dlt-receive: Implemented gtest for extended filtering
   * dlt-receive: Enabled more filtering by using json filter files
   * dlt_receive: handle exception in signal handler.
   * dlt message header broken
   * Add missing string functions (#309)
   * gtest: Find system-provided external gtest (#301)
   * Make nonverbose mode non exclusive (#300)
   * Merge pull request from GHSA-7cqp-2hqj-mh3f
   * daemon: check the conf inputs
   * Export cmake config file (#289)
   * Better formatting of RAWD (#291)
   * fix bad funcion cast in dlt_user_log_out_error_handling (#294)
   * Contact information update (#299)
   * dlt-receive: Add option to configure port of dlt receive (#293)
   * Add verbose mode attribute handling (#292)
   * Resolving broken link in README (#297)
   * fscanf() uses dynamic formatting to prevent buffer overflow (#288)
   * Make dlt pipe only readable by user (#285)
   * dlt_client_main_loop running in an infinite loop restricts graceful exit of DLT Client code (#284)

### 2.18.6

   * Update releaseNotes and version to v2.18.6
   * doc: Limitation to SIGUSR1 usage on Android
   * gtest_dlt_common: fix seg fault when using memcpy().
   * example: Added customization of CxtID and AppID
   * other: fix remaining conversion warnings
   * library: fix conversion warnings
   * tests: fix conversion warnings
   * daemon: fix conversion warnings
   * shared: fix conversion warnings
   * logstorage: fix conversion warnings
   * console :fix conversion warnings
   * gateway: fix compile warnings
   * readme: cpack document for debian package
   * CMakeList: implement cpack for debian package
   * dlt-receive: flush stdout buffer by signal
   * gtest: Include necessary header
   * doc: Update doc for the maintain logstorage loglevel implementation
   * logstorage: Implement general config to maintain logstorage loglevel
   * daemon: Correct order of runtime config load.
   * libdlt: Use SIGUSR1 for thread on Android
   * dlt-daemon: log levels are not controlled
   * dlt-daemon: complete logstorage path with '/'
   * libdlt: Use poll to avoid CPU high load
   * libdlt: Flush all data in atexit_handler
   * dlt-convert: replace system() by dlt_execute_command()
   * dlt_common: Execute system command using execvp
   * logstorage: snprintf return check
   * logstorage: Update unit test
   * logstorage: Add debug logs
   * logstorage: Modify rearranging file
   * logstorage: Handle wrap-around
   * libdlt: support short version
   * design doc: update spec for new environment variable
   * doc: update disable injection msg for developer
   * libdlt: disable injection msg via env var
   * doc: Rewrote the DLT user documentation.
   * daemon: Alternative of timerfd in QNX
   * bug-fix: fix invalid file descriptor check
   * dlt_user: fix invalid poll timeout
   * README: Switch from travis-ci.org to travis-ci.com
   * Add support for logging with VSOCK (#255)
   * Use dlt_defaults for dlt-logd-converter in Android.bp (#271)
   * dlt-control.c: return -1 on error (#259)
   * Check size of ring buffer (#269)
   * dlt_common: Fix buffer overflow in dlt_filter_load (#275)
   * dlt-daemon: Adds an option to disable injection mode (#266)
   * Add message length check
   * Init logd crash buffer
   * Avoid memory access errors with 4-chars context ids (#250)
   * dlt_client.c: remove misleading error message (#258)
   * Fix overflow for -d argument in dlt-example-user (#270)
   * dlt-daemon.c: exit early on error (#261)
   * Remove nonexistent file from Android.bp (#264)
   * dlt-control.c: initialize dltdata via struct literal (#257)
   * dlt-common.c: ensure null terminated string (#256)
   * Make it easier to use libdlt when building DLT as a CMake subproject (#254)
   * Simplify setting of CONFIGURATION_FILES_DIR in CMakeLists.txt (#247)
   * Create codeql-analysis.yml (#252)
   * Modify CMAKE_C_COMPILER check for QNX to accept "qcc" (#246)
   * dlt_offline: fix build failures with gcc-10 (#245)
   * Implement DLTClient for UDP multicast (#240)
   * Revert "dlt_offline: fix build failures with gcc-10"
   * dlt_offline: fix build failures with gcc-10
   * sd-daemon.c: Fix build with newer glibc and musl libc
   * dlt_user.c: fix the lack of DLT_NETWORK_TRACE_ENABLE definition

### 2.18.5

   * Update releaseNotes and version to v2.18.5
   * slog2 adapter on QNX
   * Update gtest_dlt_all.sh to detect core dump
   * dlt-daemon.c: fix printf format %d to %ld that formats a long
   * doc/CMakeList.txt: replace dlt_design_specification.txt to .md
   * network trace: Fix macro usage
   * limit logspam in gateway on client overflow
   * Android.bp: fix dlt_user.h genrule
   * logstorage: Issue with more than 2 filters
   * remove unused feature
   * gtest_dlt_daemon_gateway: correct comparison operator syntax
   * set DLT_NETWORK_TRACE_ENABLE by cmakedefine
   * doc: Update dlt_offline_logstorage.md
   * UT: Fix segfault in logstorage test
   * logstorage: support all stragegies
   * update cmake VERSION variables
   * simplify and fix android version script
   * execute unittests with cmake
   * dlt_design_specification: update content
   * dlt_design_specification: convert from .txt to .md format
   * logstorage: fix syncbehaviour
   * daemon: Fix smoketest
   * unittest: Update according to API's changes
   * offline storage: Improvement log messages at bottom
   * common: Isolate FIFO/Unix socket
   * libdlt: Relocate dltFifoBaseDir setting
   * doc: md file for dlt_gateway.conf
   * network trace: Include necessary headers
   * network trace: Add mqueue verification
   * Unittest for new api
   * Remove duplicate definition of DLT_CONVERT_TEXTBUFSIZE
   * Improve performance of DLT file parsing
   * relocation dlt_check_envvar() and update dlt_init() functions
   * Redirect stdout to stderr
   * Unittest: Update testcase
   * gateway: Improvement of handling Gateway config
   * daemon: Avoid spamming message buffer overflow
   * gateway: Support infinite loop of retry
   * gateway: Configurable interval time
   * tests: remove unused zlib include
   * disable android services by default
   * logstorage: Add NULL check of IDs
   * snprintf ret > 0 is not always an error
   * fix some gcc9 compiler warnings
   * fix clang warnings about GNU stuff
   * android logd forwarder
   * fix gateway config element search
   * doc: update a note for logstorage with wildcard
   * daemon: logstorage with wildcards
   * Limit log messages on full buffer
   * Android: Enable Android build
   * logstorage: Filter section handling
   * doc: Logstorage non-verbose filter
   * correct errno usage in dlt_stop_threads
   * shared: Read DLT header until it's found
   * Use ssize_t for bytes_written
   * Fix dlt-sortbytimestamp
   * Remove DLT_PACKED redefinition guard in dlt_common.h
   * rename definition PACKED to DLT_PACKED
   * daemon: Remove empty line in daemon log
   * Revert "dlt_common: Fix buffer overflow in dlt_buffer_get (#215)"
   * FIX: prevent usage of uninitialized message queue handle
   * Add unit tests for new sized string functions
   * Add documentation for new sized string functions
   * Add macro wrappers for new sized string functions
   * Minor optimization
   * Add functions for writing strings with known sizes
   * Add helper function for writing strings with known sizes
   * Fixed empty internal message in dlt-daemon.c (#225)
   * common: Fix uint64 type (#217)
   * dlt_common: Fix buffer overflow in dlt_buffer_get (#215)
   * FIX: Check validity of `file` pointer before usage.
   * doc/dlt_for_developers.md: Fix wrong DLT include directive
   * Remove naming of variadic macro parameters
   * sys/poll.h: deprecate old sys/poll.h include header, now glibc/musl wants poll.h being included directly. This fixes a build failure on musl systems with strict c hardening flags
   * dlt-test-init-free: fix build failure with strict compiler flags, due to uint being undefined. This is actually an "int" type, looking at the test implementation
   * dlt_user.h: fix build when musl is the libc implementation, by adding a missing include for pthread_t reference:
   * dlt.conf: suppress the warnings udp multicast (#197)
   * dlt_daemon_socket: leave while socket binds fails
   * dlt_user: init DltContextData before use
   * dlt-daemon: fix resource leak
   * console: fix memleak of dlt-logstorage-list
   * BugFix: SEGFAULT when using AppArmor (#192)
   * Fix compiler warnings: pointer of type ‘void *’ used in arithmetic (#196)
   * Change the DLT_CHECK_RCV_DATA_SIZE macro to an internal function (#191)
   * Fix a potential memory leak in file transfer (#126)
   * Provide DLT_GET_APPID macro (#187) (#188)
   * dlt-offline-trace: fix bug and hardcode (#174) (#186)
   * Avoided Seg fault in dlt_message_payload (#179) (#181)
   * Improvement: Make ZLib dependency optional (#182)
   * fix the dlt offline trace file name creation (#178)
   * libdlt: fix memory leak
   * Removed unused headers(epoll) in UDP connection
   * Fix: Propper usage of LoggingMode: "uncrustification"
   * Fix: Propper usage of LoggingMode in ".../dlt-runtime.cfg"
   * libdlt: reattachment and improvement in dlt thread (#171)
   * dlt-system: Call tzset before localtime_r (#165)
   * Update cmakelist, fix build due to copy-paste error (#170)
   * Bugfix: dlt-system-journal
   * Proper setup and error checking of pthread_create
   * udp: Disable WITH_UDP_CONNECTION as default
   * doxygen improvement
   * doc: Modify markdown doc generation
   * doc: Update dlt_for_developers.md
   * cmake improvement for Logstorage console
   * cmake add component for libdlt.so
   * parser: Change maximum number of config section
   * common: Remove unused structure
   * dlt_common: Use defined macro
   * libdlt: calculate resend buffer memory
   * lib: Disable extended header in non verbose mode by env var
   * shm: Resend dlt msg when client connect
   * lib: Remove else nothing
   * network trace: Do not allow DLT usage in forked child
   * lib: Assign fd after it's closed
   * console: Add get sw version control msg
   * console: Tool to merge multiple DLT files
   * dlt-test: Add message length option
   * Add user custom timestamp interface

### 2.18.4

   * Update ReleaseNotes and version to v2.18.4
   * dlt-daemon: correct errno usage
   * dlt-daemon: fix bug binding invalid ipv6 address as default
   * Add option in dlt.conf for bindAddress to specific IPs (#130)
   * protocol: Remove non supported user service ID (#159)
   * libdlt: truncate the log message if it is too long (#156) (#157)
   * UDP Multicast implementation (#155)
   * doc: Remove unused images
   * daemon: fix compile error with DLT_IPC="UNIX_SOCKET" (#153)
   * using POSIX shared memory APIs (#90) (#151)
   * Revert "Add option in dlt.conf for bindAddress to specific IPs (#130)"
   * Add option to set owner group of daemon FIFO (#122)
   * Add option in dlt.conf for bindAddress to specific IPs (#130)
   * dlt-system-journal: fixed localtime compile error
   * Correct sa findings
   * logstorage: fix compile error
   * doxygen: Align variable for apid and ctid
   * doxygen: Remove licence
   * doxygen: Get rid of warnings
   * doc: Use pandoc to generate HTML from markdown
   * doc: Improve README.md
   * doc: Create missing markdown documents
   * doc: Documentation update
   * cmake: Allow build as a subproject (#145)
   * fix config path for dlt-dbus
   * define DLT_PATH_MAX for max path buffer length
   * cmake-improvements (#135)
   * libdlt: Use posix nanosleep (#144)
   * doc: Improve markdown documents
   * doc: Improve dlt_for_developers.md
   * fix the warning of strncat size
   * fix warning of self assign
   * dlt-convert: fix warning of wrong conversion
   * Travis: Run Travis on Xenial 16.04
   * Travis: Modify install package
   * cleanup: Use dlt_vlog()
   * Fix alerts from lgtm
   * lgtm: Add code analysis platform
   * dlt-test: Improve context ID
   * dlt-test: Add options
   * libdlt: Remove commented out code
   * Remove dlt_forward_msg
   * libdlt: compare dlt_ll_ts to NULL
   * network trace: Define package ID macro
   * daemon: Loop for client fds
   * daemon: Remove bytes_sent
   * daemon: Don't remove unregistered context
   * daemon: Don't assign fd after free
   * test: Add manual interruption in dlt-test-stress
   * gtest: Logstorage unit test update
   * Logstorage: Correct behavior in sync message cache
   * Logstorage: Sync behavior bug fix
   * Logstorage: Fix write msg cache

### 2.18.3

   * Update ReleaseNotes and version to v2.18.3
   * Travis CI: Run unit test (#132)
   * libdlt: Fix compiler warnings
   * Unit test: Fix compiler warnings
   * Unit test fix
   * Do not install systemd service files for binaries that are not built (#129)
   * lib: unlock buffer on termination
   * dlt-receive: Fix crash without arguments
   * dlt-control: Bug fix for broken get log info
   * Logging: Error message modification
   * dlt-daemon: fix internal logging to file after daemonize
   * Offline logstorage: Fix storage handle NULL check during cleanup
   * Fix compiler warnings
   * POSIX: Replace usleep with nanosleep
   * unix socket: IPC code isolation
   * lib: daemon: Fix sem lock potential issue
   * socket: Remove unnecessary header
   * dlt-daemon: unlink application socket
   * ipc: close socket if connect failed

### 2.18.2

   * Update ReleaseNotes and version to v2.18.2
   * Size of Resend buffer less than or equal to DLT_USER_BUF_MAX_SIZE res… (#116)
   * Fixed memory leak when receiving network traces of 0xFFFF length
   * Contact information update (#118)
   * lib: Add mq_close/mq_unlink conditions
   * doc: Do not allow DLT usage in forked child (#95)
   * doc: Raise an awareness of log level sync
   * dlt_offline_logstorage: fix multiple file creation error (#85, #94)
   * doc: Fix PANDOC_TOOL condition
   * Travis CI: Fix link in README.md (#106, #108)

### 2.18.1

   * doc: Move all man pages to markdown files (#102)
   * Fix linking problem with tests when systemd enabled (#103)
   * libdlt: Do not allow DLT usage in forked child (#95)
   * Travis CI: build with systemd enabled (#97)
   * Make dlt-convert more responsive when watching a file
   * Travis CI: fix - add new line to .travis.yml
   * Build and test status added
   * Add the Travis CI script

### 2.18.0

   * fix broken/missing links in documentation
   * CMake: Set default configuration
   * Code beautification using uncrustify
   * Documentation update
   * dlt-daemon: Output current number of connections as default
   * Remove unnecessary reference to zlib in .pc file
   * Cleanup of unit test fixes
   * gtest: Modification to offline logstorage
   * libdlt: Add error handling
   * exit DLT daemon if /dev/null open fails during fork
   * Improvement - use dup2 in place of dup in daemon fork
   * Remove one-instance-lock mechanism
   * daemon: Add exit trigger
   * UnitTest: Updates
   * Made socket send reliable
   * lib: socket: Flush all data before closing socket
   * buffer: Code cleanup
   * buffer: Improve logging
   * Removed log level change callback notification while context register done with ll_ts API
   * Logging: avoided missing of log level change callback
   * Injection: New callback with private data Added new injection callback with private data as argument
   * Fixed compiler error with previous commit
   * Dynamic allocation of msg buffer
   * cmake: systemd: fix hardcoded user in dlt-dbus.service (Issue #36)
   * rename #define STATIC to DLT_STATIC
   * Use poll in the dlt-daemon for POSIX compliance
   * dlt-client: logging: Extended the receiver buffer size
   * dlt-control: update get log info
   * Protocol: DLT Service ID Enum instead of defines
   * Gateway Improvements
   * Log storage - Updates (#82)
   * Fflush stdout in the intenal logger (#81)
   * dlt-daemon: per ECU list of user information (#80)
   * Add dlt-sortbytimestamp utility plus documentation (#73)
   * Fix compiler warning PR #77
   * Fix compilation with glibc 2.28 (#77)
   * Fix gcc 8 build (#74)
   * dlt-daemon: fixed linked-list element remove (#71)
   * Update dlt_user.c (#66)
   * dlt-daemon: Fix no state transition to BUFFER state (#65)
   * file parser: Replace hash functions with list (#67)
   * libdlt: Avoid busy loop in error case of mq_receive() (#59)
   * dlt-daemon: Output signal number at exit (#68)
   * dlt-daemon: Improve error logging on accept() failure (#69)
   * dlt-daemon: Avoid to output duplicated application registration message (#63)
   * dlt-daemon: Not output Context un-/registration DLT message by default (#62)
   * dlt-daemon: Continue to send log level / connection status even if error occurs (#61)
   * IPC: Unix socket added (#43)
   * Introduce controlling entire system trace status feature from dlt-control (#57)
   * dlt-daemon: Lower log level of logs not to output unintentional warning (#58)
   * dlt-daemon: Fix infinite loop on set log level using wildcards (#55)
   * dlt-daemon: Fix repeated output of marker message (#54)
   * dlt-control: Fix Setting default trace status issue (#53)
   * Fix ForceContextLogLevelAndTraceStatus handling in dlt_daemon_client.c (#50)
   * minor compiler warning gcc 7.x (#30)
   * improve error reporting in dlt_daemon_socket (#41)
   * Fix SEGV dlt_offline_trace.c (#32)
   * fix PR #26 socket_sendreliable data_send update
   * Prevention for occasional corrupted messages  (#26)
   * README: Update contact information

### 2.17.0
  * Fix for initialization of buffer settings in DLT user library.
  * fix various memory leaks
  * some-minor-fixes
  * Minor fixes: corrected typo in CMakeLists.txt - WTIH_DLT_ADAPTOR, removed character from merge
  * Data stuck in receiver buffer when dlt_daemon_user_send_log_level() fails
  * Update dlt_user.h
  * Add short explanation for DLT log level
  * Prevent buffer overflow for mount point path in dlt_logstorage_open_log_file
  * journald adaptor: test with sudo privileges
  * cmake: fix unit tests compilation with systemd
  * Input parameter check & Error message modification
  * Tell cmake to use [README.md](README.md) instead of README to fix doc generation
  * dlt-system-process-handling: fix warning
  * dlt_daemon_connection_types: fix build warnings
  * Updated README
  * dlt-adaptor-udp, dlt-adaptor-stdin: implement get of verbosity level from input
  * Added Description in dlt-system.conf
  * dlt-client: fix dlt_client_cleanup memory handling
  * CMake Option: Trigger segmentation fault in case of FATAL log
  * Daemon connection handling fixes
  * Added Description in dlt-system.conf
  * dlt-client: fix dlt_client_cleanup memory handling
  * CMake Option: Trigger segmentation fault in case of FATAL log
  * dlt-daemon: Fix use after free potential issue
  * Event handling: Fix connection destroy bug
  * Remove duplicate README
  * daemon: check payload length before cast to struct
  * Added missing build steps to INSTALL
  * pkg-config: fix library directory.

### 2.16.0
  * doc: Documenatation update
  * Systemd-journal-test: Add WITH_DLT_UNIT_TEST flag when building sources
  * Smoketest: Offline Logstorage
  * Smoketest: Multinode
  * Unit Test: Event handling
  * Provision to test static function
  * Unit Test: MultiNode
  * Unit Test: Multinode Unit test preparation script
  * CMake: Add option to build unit test binaries
  * dlt-system-filetransfer: fix bug caused by malloc assert
  * Environment variables for library ringbuffer
  * DLT_PTR macro: Improve implementation and function API added
  * MultiNode: Specify config file location in dlt.conf
  * dlt-client: Use correct port on connect
  * process user message: Fix bound handling
  * dlt-system-filetransfer: Fix compiler warnings
  * Remove C99 style comments in include directory
  * Dlt-Receive: Use PRIxxx macros for printf variables
  * Offine logstorage: Remove duplicated source file
  * Fix: Memory for context description is not freed
  * Fix: dlt-daemon overwrites ECU ID even if user log message already has the ECU ID that is not default value
  * Add: Configuration of option of get log info response during context registration
  * Add: Configuration of daemon FIFO size
  * Fix: Handle of /tmp/dlt never reset if dlt-daemon is killed during output user buffer
  * Add: Debug log for file transfer feature of dlt-system.
  * Fix: Segfault in checking buffer usage
  * Fix: File Transfer acceleration
  * Fix: File name is broken when file is transferred on 64 bit OS.
  * Fix: Memory leak issue in dlt-dbus
  * Add dlt_user_is_logLevel_enabled API
  * [README.md](README.md) formatting changes
  * Adding GitHub flavored markdown for README.
  * Fixed D-Bus tracing not working anymore
  * Fixed not working default log level.
  * Fixed not returning the correct number of lost messages at exit.
  * dlt-daemon: Free DltDaemon structure on exit
  * CommonControl: Fix for commands not working with unix socket
  * CommonControl: Unix socket path and ecuid parsing for control applications
  * dlt-control: Provision to control entire system log level
  * DLT_PTR: User macro to print pointers
  * dlt-daemon: Fix user log handler return value
  * dlt-daemon: Connection activation rework
  * dlt-daemon: receiver rework
  * Fix connection handling of serial interface
  * Offline trace: Make search more precise
  * MultiNode: Add support for mandatory configurations
  * MultiNode: Add support for SerialHeader conf
  * MultiNode: Add support for port configuration
  * MultiNode: Send control messages after connection
  * MultiNode: Send serialheader if specified in dlt.conf
  * Offline logstorage: On Demand triggering for syncing Logstorage cache and support long options
  * Offline logstorage: Fix to resetting of Syncbehavior value
  * Offline logstorage: Refactor filter storage functionality to support general properties
  * Offline logstorage: Fixed extended header size check condition
  * Offline logstorage: Fix invalid filter configuration handling
  * Added abnormal unit tests to check DLT_RETURN_USER_BUFFER_FULL
  * DLT_RETURN_USER_BUFFER_FULL is returned when user buffer full
  * Revert truncation of string or raw block

### 2.15.0

  * Fixed bug with truncation of string or raw block
  * Updated man pages and README
  * Truncate string or raw block if length exceeds maximum message length
  * Fixed a bug in dlt-system filetransfer
  * dlt-system filetransfer waits for a client to connect
  * Fixed compilation for older versions of gcc
  * Fixed core pattern to use correct dlt-cdh install path
  * Fixed CMakeLists to build core dump handler
  * Replaced Type=Simple with Type=simple in cmkake files for .service files
  * Added systemd install dir parameter
  * Added option to specify user for non-root processes
  * Added dlt-kpi component to log various KPI information to dlt-daemon

### 2.14.1

  * MultiNode: Reconnection after connection loss
  * Fix injection message handling

### 2.14.0

  * Fix fork()-handler in libdlt
  * Set default log-levels in dlt.conf
  * Fix register context before application is registered
  * Fixed compiler warnings about format issues in dlt-system-journal.c by replacing llu with PRIu64
  * Make IP version compile time configurable
  * Implemented Dlt MultiNode to connect DLT Daemons running on different operating systems
  * Daemon shutdown: fixed memory leaks and missing removal of created sockets
  * DltLogstorage: reduce writing to internal storage device as much as possible
  * Control appliction to support offline log storage trigger implemented
  * Offline log storage to internal and external devices implemented
  * Unix socket control interface implemented
  * Parse INI files for Offline Logstorage, Multinode and potentially other DLT extensions implmented
  * Linking library systemd instead of systemd-journal systemd-id128 if systemd version >= 209
  * Event handling has been reworked in order to use epoll and restructure the code
  * Fixed include paths in dlt_user_manual.txt and dlt_cheatsheet.txt

### 2.13.0

  * Added core dump handler code
  * Purged all warnings for -Wall -Wextra with gcc 4.9.1
  * Set DLT_USER_BUF_MAX_SIZE to 1390 to prepare UDP message transport
  * dlt-test-client and dlt-test-filetransfer have global failed test counter so they can return 1 on failure
  * Using DLT_USER_BUF_MAX_SIZE in dlt-test-client.c truncated check
  * Set path to /usr/local/share/ in dlt-test-filetransfer.c
  * Added programme to test repeated calls of dlt_init and dlt_free
  * DLT daemon improvement - dlt_init()-check
  * DLT daemon improvement - parameter value range check
  * Adapt unit tests to check for enum return values
  * Changed C version to gnu99 and C++ version to gnu++0x
  * Fixed bug in INTERNAL-mode connection
  * Use the best possible timestamp for all system journal entries
  * Make timeout in at_exit handler configurable
  * Allow multiple instances of dlt-daemon
  * Add C++ extension which uses variadic templates from C++ 11 (disabled by default)
  * Allow registration of contexts before application is registered
  * Add env-var to set initial log-levels
  * Allow applications to fork()
  * Fixed file permissions
  * Added offline logstorage implementation which can be used instead of the already available offline trace functionality

### 2.12.1

  * Removed all trailing whitespaces
  * Replaced all tabs with spaces in all files in include folder
  * Rework of addon tests filtetransfer, systemd-journal and system-logger
  * Fix compilation warnings and possible misuse of snprint
  * Rework of unit tests
  * Fix installation paths on x86_64 (lib64 instead of lib)

### 2.12.0

   * Added unit and functional tests
   * Fixed copyright doxygen comments
   * Updated license headers to latest COVESA license policy
   * Renamed and cleanup further files to comply with licensing requirements
   * dlt-control: Check for return values
   * dlt-daemon: Explicitly set the default loggingLevel to LOG_INFO
   * dlt-daemon: Explicitly set the default loggingMode to DLT_LOG_TO_CONSOLE
   * Fixing MIN and MAX defines for base integer types
   * Unified all line endings to UNIX style
   * Adding gtest framework v1.7.0
   * Remove absolute installation paths so that DLT can be installed at any location (not only "/usr").
   * Add Service ID Last entry to avoid further modifications in dependent code
   * Change daemon state handling to have all traces in online trace even when offline trace is active
   * Fix content of offline trace
   * Open daemon connection in atexit function
   * Change loglevel of Request-Resend message
   * Fix daemon state handling with offline trace
   * Fix watchdog timeout
   * Reworked internal output
   * DLT MISRA conform changes
   * Made zlib dependent on DLT_SYSTEM
   * Doxygen paths are now determined by CMake.
   * Add the IPv6 support
   * Workaround for duplicated log messages in offline trace file issue
   * Fix PREFIX. Works now with the default PREFIX (/usr/local/) and with the user PREFIX (e.g. /temp/test_with_pref). PREFIX Fix for filetransfer directory (PREFIX/share/)
   * Fixed typo in include guard
   * Resolves BUG-206: Install prefix should be configurable
   * Adding support for new macros to the daemon. New macros: DLT_HEX8(VAR) 8bits variable displayed in hexadecimal with "0x" prefix DLT_HEX16(VAR) 16bits displayed
     in hexadecimal with "0x" prefix DLT_HEX32(VAR) 32bits displayed in hexadecimal with "0x" prefix DLT_HEX64(VAR) 64bits displayed in hexadecimal with "0x" prefix DLT_BIN8(VAR)
     8bits variable displayed in binary with "0b" prefix DLT_BIN16(VAR) 16bits variable displayed in binary with "0b" prefix
   * Fixed network trace test
   * Fix dlt_user_log_write_start_id return value
   * Added new API to send marker message from application.
   * New Callback function in DLT library, called when log level of context is changed

### 2.11.0

   * New macros for Format of Hex and Binary.
   * Enable dbus trace when adaptor starts up.
   * Added configuration of dbus filter.
   * Fixed segmented messages arguments to standard.
   * First implementation of DLT DBus adapter.
   * DLT_CSTRING implementation non verbose mode.
   * Added new examples which can be manually build against DLt library.
   * Send ECU Id if enabled and added library API to change.
   * Send timestamp can be disabled by new API.
   * Send session/process id by default and add configuration API.
   * Send extended header in non verbose mode by default and add new API to change setting.
   * Make daemon buffer size configurable

### 2.10.0

   * Bug 184 - /tmp/dltpipes directory does not exist before dlt-daemon is started, logging disabled
   * Updated authors information.
   * Fixed missing variable declaration when systemd not enabled.
   * Fixed: all possible malloc, sprintf and strcpy problems
   * Fixed: Creation of dltpipes directory is too late.
   * Cygwin port: cygwin patch, signal handling patch and cppcheck and install lib dll to correct location on Windows.
   * Fixed compiler warnings with 32Bit gcc compiler.
   * Fixed: Fixed offline trace and new send functions issues
   * Fixed: Bug 172 - DLT system crashes because of wrong journald adaptor implementation
   * DLT Common API Wrapper.
   * Removed dlt_free from example and test applications, already called from exit handler.
   * Fixed missing dlt_receiver_remove in dlt_daemon_process_user_xxx functions.
   * Use LIB_SUFFIX as lib installation path.
   * Fixed serial port not working anymore.
   * Added log output of created socket/port In init phase 2 - socket creation
   * Defined return value for dlt_message_read().
   * Cleanup of send return values.Further cleanup of send restructure.
   * Moved daemon client functions to new source file.
   * Centralised send function to client.Introduced connection state to dlt daemon.
   * Removed check of double registration of contexts in user library, already checked by daemon.
   * When using DLT in console mode on a 64-bit machine, timestamps are corrupted due to an address of a 32-bit value being cast to a 64-bit pointer.
   * Bug 3 - Cmake does not check for zlib for dlt-daemon compilation.
   * Added new control message timezone.
   * Fixed deadlock after wrong merge..
   * Fix potential buffer overflow in offline trace.
   * Fix deadlock in dlt_user_log_reattach_to_daemon(void).
   * Fixed possible crash when runtime configurations files are corrupted.
   * Environement variables added to configure internal logging in library.
   * Reduce Timeout between filetransfer packets.
   * Close socket when send fails.
   * Replace threads by timing fds for ecu version, timing packets and watchdog.
   * Added conntection info and unregister context control messages.
   * Configurable Timeout on send.
   * Added further checks to dlt_buffer.
   * atexit handler fix.
   * Add threadnames to libdlt threads.
   * Security fix on DLT pipes.
   * Reduce usage of SEM_LOCK in application library and reset pointers.
   * Fix: Systemd Journal Adapter provides corrupted output.
   * Fix: Install Example service file only when example enabled

### 2.9.1

   * Implementation of command line tool dlt-control.
   * Fix file transfer bug.
   * Bug 44 - Don't print "Buffer full" message from DLT daemon for each trace.
   * Yocto fix in build builds.
   * Fixed: security issue in dlt-system-shell regarding strncpy.
   * Fixed: Security Issue by Command Injection in DLT System.
   * systemd/CMakeLists: Remove SYSTEMD_CONFIGURATIONS_FILES_DIR existance check.
   * Bug 85 - Include of dlt.h leads to compiler warning.
   * Bug 84 - Adding utf8 support to dlt-daemon, dlt-viewer. Modified patch, originally provided by Stefan Vacek.
   * systemd journal support added.
   * spec file does not package man files when cmake is run with -DWITH_DOC=OFF
   * added length check for paths of files to be transferred
   * Semaphores and Pointer passing insteasd by value and otehr coverity issue fixes
   * Fixed several issues in DLT filetransfer.
   * added creation date and a simple hash on the file name for to improve the uniqueness of getFileSerialNumber
   * modified filetransfer to be more robust in restarting transfers
   * Remove dangling DLT_SEM_FREE from dlt_user_queue_resend
   * Unifed ECU version sending functions
   * Refinements due to problems reported by static code analysis
   * Spec file does no more package man files when cmake is run with -DWITH_DOC=OFF
   * Made the APID strings in dlt-test-multi-process counting from 00-99
   * Added creation date and a simple hash on the file name for to improve the uniqueness of getFileSerialNumber
   * File Transfer: improved robustness in case of restarted ECU/dlt-system with interrupted transfers

### 2.9.0

   * Changed documentation and man pages into asciidoc format.
   * Increased buffer sizes for DLT user library and DLT daemon
   * [GDLT-120]: truncated and Segmented network tracing
   * [GDLT-137]: Automatically try resending of user buffer after FIFO full
   * [GSWD-85]:  Added authors file

### 2.8.0

   * [GDLT-115]: Encapsulate user macros
   * Fix register app and register context was not stored in buffer when FIFO is full. Other controll messages still not saved in buffer.
   * Create new fifo only when same application registers with different pid.'
   * Do not register appliction again, if already registered.
   * Fixed filetransfer not checking buffer fill level.

### 2.7.0

   * [GDLT-24] Fixing compiler warnings
   * [GDLT-94] Optional sending periodic software version messages. See man pages for more informations
   * [GENDLT-26] Check for description length sanity
   * [GENDLT-24] Crash on invalid injection message fixed
   * [GDLT-93] Add -Wextra flags for compilation Fixed all the warnings that
   * [GDLT-90] Optional: systemd watchdog concept in dlt-system and dlt-daemon
   * [GDLT-67] Re-implemented dlt-system. Read full commit message for more information

### 2.6.2

   * [GDLT-89] Fixed daemon doesn't sent the persistent log level
   * [GDLT-88] Fixed wrong initalization order using offline trace function

### 2.6.1

   * Add _GNU_SOURCE Definition to be able to use O_CLOEXEC
   * Added important SEM_FREE in the daemon and closing fd in the filetransfer
   * [GDLT-3] Fixed missing semaphore around dlt_buffer_push3
   * [GDLT-86] Fixed dlt_free uses absolute file path /tmp and not DLT_USER_DEFINE define
   * [D4099] Check for duplicate file handles, and clean them up if found
   * [GDLT-85] Pipes opened multiple times for the same application pid fixed
   * [GDLT-82] Child process inherits file descriptors openend by their parent fixed
   * [GDLT-84] Instead of calling the injection callback, store a pointer to it and the required parameter data fixed
   * [GDLT-70] Check for malloc failures and return errors where applicable
   * [GDLT-47] Avoid discarding old contexts if no new memory can be allocate
   * [GDLT-69] Fixed bug in dlt-test-multi-process shares context between threads

### 2.6.0

   * [GDLT-75] Use old style directory check on startup
   * [GENDLT-21] Move mcnt from DltContextData to DltContext
   * [GENDLT-15] Fixes to previous integrations from review
   * [GENDLT-15] Safe re-allocations for databuffer
   * [GENDLT-15] use the correct TEXTBUFSIZE
   * [GENDLT-15] Optimize usege of strlen. Improved log level handling
   * [GENDLT-15] Avoid buffer overrun with snprintf()
   * [GENDLT-15] Check return value of dlt_user_log_write_start(_id) correctly
   * [GENDLT-15] Reduce the number of applications if allocation fails
   * [GENDLT-15] Make dlt_user_log_write_start inline
   * [GENDLT-15] Improve errore checking in dlt_user_log_write_start_id
   * [GENDLT-15] Use databussersize to avoid reallocations
   * [GENDLT-15] Rename buffer size constant to avoid confusion
   * [GENDLT-15] Better error handling when writing to FIFO
   * [GENDLT-15] Remove duplicate msg initialization.
   * [GENDLT-15] Optimize away multiple uses of strlen for one check
   * [GDLT-4] Improve queue handling, allow for other messages while transferring a large file
   * [GDLT-4] Limit maximum file queue to 256 files
   * [GDLT-4] First working version of inotify for file transfer
   * [GDLT-2] First test for filetransfer change
   * [GDLT-2] Change to gzip wrapper format. Change file signature creation to account for file size, as inode number maybe duplicate when deleting and creating new files
   * [GDLT-2] Fix bug while reading the options
   * [GDLT-2] Allow for enabling/disabling compression for the separate directories
   * [GDLT-2] Check if the file is already compressed
   * [GDLT-2] Link with libz, fix a typo
   * [GDLT-2] zlib based compression for dlt-system
   * Cleaned some warnings generated from removing stale old code

### 2.5.2

   * Change to Mozilla Public License Version 2.0

### 2.5.1

   * Fixed bug with comparinson between signed and unsigned integer and protection for a buffer overflow.
   * Modified library for new test cases to corrupt data - related to the bug fix for testing signed and unsigned integer
   * [GENDLT-20] Fixed bug to use old cmake version for copy file
   * Replaced dlt-test-filetransfer-image.png with an own created image
   * [GENDLT-21] Fixed bug: Message Counter (MCNT) should be increased but is always 0

### 2.5.0

   * [GDLT-53] Man pages installation included
   * .cproject and .project file for Eclipse included
   * Update of doxygen documentation and generation
   * Rework of root CMake project file, e.g. structure and compile options
   * [GENDLT-16] Create variable in dlt-system.conf to configure the timeout of the filetransfer
   * [GDLT-37] Extend automated test tools for parallel process/threads tests
   * [GSW-138] API Extension to resend the log messages in the user buffer
   * [GDLT-36] Prefixing of dlt_version.h fixed
   * [GDLT-31] Tracefile content stored different under Ubuntu 64 bit version compared to Ubuntu/Win 32 Bit version fixed
   * [GDLT-35] Compile warnings fixed
   * [GSW-137] Wrong include gives error on compailing against dlt fixed

### 2.4.2

   * Added dynamic increasable ringbuffers to user lib and daemon.
   * dlt-system filetransfer now recovers when file is deleted during filetransfer.
       * Added check of file size when starting and deleting files during filetransfer
       * Added chekc of shm buffer availability when push to shm
   * Create abstraction of shm buffer management.
   * Fixed buffer overflow problem in buffer library.
   * Disabled share memory by default - disabled completely shared memory if not enabled.

### 2.4.1

   * Added dynamic increasable ringbuffers to user lib and dlt-daemon.
    * Created abstraction of shm buffer management.
    * dlt-system filetransfer now recovers when file is deleted during filetransfer.
    * Added check of file size when starting and deleting files during filetransfer.
    * Added check of shm buffer availability when push to shm.

### 2.4.1

   * Added internal logging facility to stdout, syslog or local file, configurable in configuration file.
  * Added deamonise and signal handlers to dlt-system.
  * Added manual pages.
  * Added new API dlt_check_library_version() function.
  * Fifo or SHM mode can be changed by compiler switch.
  * Replaced SHM implementation.
  * Fixed shared memory problem in DLT library during startup, if application is started before daemon.
  * Fixed syslog adapter in dlt-system.
  * Reverted API changes in dlt_register_app() function.
  * DLT user library does not set the stack size of the receiver thread anymore.

### 2.4.0

  * New config files /etc/dlt.conf and /etc/dlt-system.conf must be adapted to the needs
  * New DLT user lib API dlt_get_log_state() to get DLT client state
  * New DLT user lib API to manage flow control (needed for bulk data logging)
  * New DLT user lib API dlt_set_log_mode() to enable/disable internal/external trace
  * New application dlt-system (filtransfer, proc file system logger,syslog udp adapter included)
  * [GSW-66] File transfer over DLT.
  * [GSW-43] Performance improvement for bulk data over DLT.
  * [GSW-61] Replace command line parameter by configuration file
  * [GSW-13] Support for keep-alive messages as configuration parameter
  * [GSW-60] Extended offline DLT Trace memory handling.
  * Removed filter implementation

### 2.3.0

  * [GSW-16] Systemd configuration for syslog to DLT dapater
  * [GSW-62] DLT Library version check
  * [GSW-28] Directory where persistent data is stored is not configurable
  * [GSW-59] Statically allocated large array
  * Added init script for Ubuntu
  * Optional adding of gprof compile flags
  * sprintf with float64 fails on ARM platform; disabled this function on QRM platform.

### 2.2.0

  * Moved build process completely to cmake
  * Added commandline parameter -u to set ring buffer size
  * Reduced cpu consumption needed by applications using DLT library
  * Increased default ringbuffer size to 10024 bytes
  * Changed delay in receiver routine to 100ms

### 2.1.0

 * DLT Viewer (QT)
    * New dlt viewer (QT-based) implementated
    * Moved to seperate project, see extra Release Notes for DLT Viewer (QT)
 * DLT Viewer (WX) - Deprecated
    * Old dlt viewer (WX) is removed now from package generation
    * Moved to seperate project
    * Removed filtering of messages during writing to a file
 * DLT library:
    * Functions dlt_file_read_raw() and dlt_file_read_header_raw() added
    * Added support for raw messages in nonverbose mode
    * Injection tables are now dynamically allocated
    * Contexts are now dynamically allocated
    * Added seperate file for platform float types (dlt_float_types.h)
      and used this types.
      Attention: This file must be adapted to each target platform.
    * Removed signal handlers from dlt_user.c; SIGPIPE signal is ignored; atexit() handler still exists
    * Function dlt_forward_msg() added
 * DLT daemon:
    * Small optimization in get_log_info() for one searched application with one searched context,
      which is existing in the context table of the dlt daemon
    * Optional syncing to serial header added
    * Support for keep-alive messages, realized as seperate thread
 * General:
    * Combined dlt-test-user-multi and dlt-test-many to dlt-test-stress
    * Extended dlt-test-client
    * Added stress test3 to dlt-test-stress
    * Added help to dlt-test-stress, printed if no test was selected
    * Added dlt-test-internal
    * Removed plugin support from dlt_receive and dlt_convert
    * Extended documentation
    * dlt viewer (wx): Fixed minor bug, it's possible now to compile the dlt viewer (wx) again under mingw under Windows
    * DLT test programs: Fixed minor bug in dlt-test-user, test3f: Wrong counter was used
    * Removed DLT_LOG calls in injection functions due to problems (application hangs)

### 2.0.5

 * DLT viewer:
	* The default log level is now shown, if already known
	* Renamed Filter->New.. to Filter->Delete all filter
	* Enhanced performance
 * DLT library:
	* On crash or termination of application using the DLT library,
	  the registered context and application IDs are removed properly
	  (and are deregistered from DLT daemon)
	* dlt_register_context_ll_ts() and Macro therefore added
	* dlt_message_payload() has now additional type DLT_OUTPUT_ASCII_LIMITED
	* dlt_message_header_flags() added
 * DLT daemon:
	* Support for dlt_register_context_ll_ts() added
	* Enhanced support for get_log_info (all modes, 1 app all contexts, 1 app 1 context, all apps all contexts)
	* Added -r option, for automatic sending context information to dlt client; if no client connection is available,
      this information is stored in history buffer in dlt daemon
	* Several internal performance optimizations:
		* dlt_daemon_context_find(), dlt_daemon_application_find(): Now O(log n) instead O(n)
		* Several functions optimized
		* Unnecessary functions removed
 * General:
	* Moved definition of struct DltUser from dlt_user_private.h to dlt_user.h
	* dlt.h includes now dlt_common.h
	* Extended dlt-test-user and dlt-test-client applications
 * DLT daemon/DLT library: Fixed bug in Filter Delete
 * DLT daemon: Fixed bug in dlt daemon which leads to a crash, when starting/stoping application, then sending
     new log level to context of this (now not running) application.
 * DLT daemon: Fixed bug in unregister application
 * DLT daemon: Fixed bug in reattach to daemon
 * DLT library: Fixed bug in send function
 * DLT viewer: Fixed bug in set default log level

### 2.0.4

 * License has changed from ADRLPD to ADRLRM
 * DLT viewer:
	* Support for non-verbose mode (as FIBEX plugin)
 * DLT library:
	* Support for non-verbose mode (as FIBEX plugin)
	* dlt_message_print_* functions added
	* Semaphore calls added to enable multi-threading
	* Changed injection interface from direct usage to callback
	* Requested log level and trace status is set immediately
          in dlt_set_application_ll_ts_limit()
	* Implemented receiver thread in DLT library
          (used for setting of log level/trace status and for injection handling)
        * Added signal-handler and atexit-handler for cleanup (calls dlt_free())
 * General:
	* Added implementation of clientlib and testclient for Windows
 	* Both adaptors sends now log messages with log level DLT_LOG_INFO
	* Multi-threading example in src/tests/dlt-test-user-multi added
 * DLT viewer: Right mouse button for loading plugin descriptions (MOST-/Fibex-XML File)
     is now working (also in Windows)
 * DLT library: Fixed bug in dlt_print_mixed_string()
 * DLT library: Fixed bug in dlt_daemon_contexts_get_next_con_id()
 * DLT daemon: dlt_daemon_process_user_message_unregister_application() also removes now
     all corresponding contexts
 * DLT daemon: Added security check to dlt_daemon_control_get_log_info() in order to avoid crash
     which occured under special circumstances
 * DLT daemon: Register app now opens the connection to the DLT library,
     unregister app closes the connection (was before in register context)
 * Added -lrt to package config file
 * Resolved dependency from dlt_client.h to dlt_common.h -> dlt_common.h is now public

### 2.0.3

 * DLT viewer:
	* Reduced load if idle
	* Modified behaviour of settings in dlt-viewer
	* Always open tmpfile in dlt-viewer if nothing other is specified
	* File->Clear added
 * DLT daemon:
	* Added several checks within code
 * DLT library:
	* Added several checks within code
	* Enhanced local print modes:
	  a environment variable now can be used to control local print mode:
	  Variable: DLT_LOCAL_PRINT_MODE
	  Values  : "AUTOMATIC"             (local print only, if dlt-daemon connection is not
		                             available at startup of program using DLT library)
		    "FORCE_ON"              (always do local print)
		    "FORCE_OFF"             (never do local print)
	* A client library for writing console client applications (Linux) is now available.
	  dlt-receiver and dlt-test-client uses this new library code
 * General:
	* Added seperate file for DltMostMessageHeader type
	* Added seperate file for DLT protocol values
	* Relaxed checks for passing trace messages to plugin handler
	* Tested and improved MOST plugin
	* Support for float (32 Bit) and double (64 Bit) values
	* Code fragments for winclientlib and wintestclient added
 * DLT library: Fixed bug in DLT_IMPORT_CONTEXT
 * DLT library: Fixed bug in dlt_plugin_print() and dlt_most_payload()
 * DLT daemon and library: Fixed bug in handling of description strings
 * DLT viewer: Fixed bug in RMB Click for loading plugin description
 * General: Fixed parsing and printing of MOST messages
 * Several small bugs fixed

### 2.0.2

 * DLT viewer:
	* Showing timestamp
	* Compiles now with MS Visual C++
	* Support for loading multiple descriptions
	  of plugins is now possible (*)
	* Plugin description can be loaded individually
	  by Right-mouse-button (*)
 * DLT daemon:
	* Overflow message is now stored in history buffer,
	  if necessary
 * DLT library:
	* Ring-buffer for injection messages implemented
	* History Buffer for Startup + Overflow implemented
	* Setting of maximum logged log level/trace status for
          application triggered by application is now possible
	* Optional local output of Log message is now possible
 * General:
	* Support for ARTIS Box implemented (all, without GUI)
	* Support for timestamp in standardheader extras added
	* Support for ECU ID in standardheader extras added;
	  this value can be overwritten by the DLT daemon
 * DLT viewer:
	* Store and load application and context description fixed
	* Fixed crash on termination of Windows version
 * DLT console utilities:
	* Fixed printing of filter ids
 * General:
	* Big Endian/Little Endian support tested and fixed
	* Fixed writing and reading of locally created dlt files
	* Several smaller bugs fixed

### 2.0.1

 * Full support for serial connection between DLT daemon and DLT Viewer
 * Several small bugs fixed in DLT Viewer

### 2.0.0

 * Initial Release of new DLT daemon Version 2 including the new DLT Client DLT Viewer
 * Initial Release
