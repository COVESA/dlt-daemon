# Compile the port for QNX

**NOTE**: QNX ports are only supported from a Linux host operating system

## Build

- Setup your QNX SDP environment:
```
source <SDP_PATH>/qnxsdp-env.sh
```
- From the project root folder, type:
```
make -C qnx/build install JLEVEL=4 [INSTALL_ROOT_nto=PATH_TO_YOUR_STAGING_AREA USE_INSTALL_ROOT=true]
```

Build a single architecture only (e.g. aarch64):
```
make -C qnx/build/nto-aarch64-le install JLEVEL=4
```

Clean:
```
make -C qnx/build clean
```

## Default CMake variables

| Variable | Value |
|---|---|
| WITH_DLT_QNX_SYSTEM | ON |
| WITH_DLT_CXX11_EXT | ON |
| DLT_IPC | UNIX_SOCKET |
| DLT_USER_IPC_PATH | /tmp |
| WITH_DLT_ADAPTOR | ON |
| WITH_DLT_USE_IPv6 | OFF |
| WITH_LIB_SHORT_VERSION | ON |

To customize features, edit **CMAKE_ARGS** in *qnx/build/common.mk*.

## QNX build constraints

- QNX has no `timerfd`. The daemon uses pipe+thread timers instead
  (`__QNX__` code paths in `src/daemon/dlt-daemon.c`).
- `DLT_USER_IPC_PATH` is pinned to `/tmp` so the daemon and the application
  library agree on the IPC endpoint location.
- `DLT_IPC` defaults to `UNIX_SOCKET`. FIFO also builds and works on QNX
  (verified with QNX SDP 8.0.3, aarch64). UNIX sockets are preferred because
  FIFO needs `DaemonFifoGroup` provisioning so non-root applications can open
  the daemon FIFO (see *doc/dlt_design_specification.md*).
- The daemon and the application library must be built with the same
  `DLT_IPC` value. `qnx/build/common.mk` rejects values other than
  `UNIX_SOCKET` or `FIFO`.
