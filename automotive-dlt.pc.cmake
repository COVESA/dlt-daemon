prefix=@prefix@
exec_prefix=@prefix@
libdir=@libdir@
includedir=@includedir@

Name: DLT
Description: Diagnostic Log and Trace 
Version: @GENIVI_PROJECT_VERSION@
Requires: 
Libs: -L${libdir} -ldlt -lrt -lpthread
Cflags: -I${includedir}/dlt -DDLT_@PRINT_MAJOR_VERSION@_@PRINT_MINOR_VERSION@

