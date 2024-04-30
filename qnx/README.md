# Compile the port for QNX

**NOTE**: QNX ports are only supported from a Linux host operating system

- Setup your QNX SDP environment
- From the project root folder, type:
```
git submodule update --init
make -C qnx/build install JLEVEL=4 [INSTALL_ROOT_nto=PATH_TO_YOUR_STAGING_AREA USE_INSTALL_ROOT=true]
```

There are several CMAKE variables set for the QNX build as recommended by the dlt-daemon project maintainers:
* WITH_DLT_QNX_SYSTEM=ON
* WITH_DLT_CXX11_EXT=ON
* DLT_IPC=UNIX_SOCKET
* WITH_DLT_ADAPTOR=ON
* WITH_DLT_USE_IPv6=OFF
* WITH_LIB_SHORT_VERSION=ON

If you want to customize dlt-daemon features via CMAKE variables, you have to edit *qnx\build\common.mk* file and modify **CMAKE_ARGS** variable.
