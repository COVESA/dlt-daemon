Name: @PROJECT_NAME@
Summary: %{name} - Diagnostic Log and Trace
Version: @PRINT_MAJOR_VERSION@.@PRINT_MINOR_VERSION@.@PRINT_PATCH_LEVEL@
Release: @GENIVI_RPM_RELEASE@
License: @LICENSE@
Group: System Environment/Base
Vendor: BMW Group AG
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: %{name} = %{version}-%{release}, pkgconfig

%description
This component provides a standardised log and trace interface, based on the
standardised protocol specified in the AUTOSAR standard 4.0 DLT.
This component can be used by GENIVI components and other applications as
logging facility providing
- the DLT shared library
- the DLT daemon, including startup scripts
- the DLT daemon adaptors
- the DLT client console utilities
- the DLT test applications

%package doc
Summary:        %{name} - Diagnostic Log and Trace: Documentation
Group:          Documentation

%description doc
This component provides the documentation for %{name}.

%package devel
Summary:        %{name} - Diagnostic Log and Trace: Development files
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}, pkgconfig

%description devel
This component provides the development libraries and includes for %{name}.

%prep
%setup
echo "building package automotive-dlt"

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc/init.d
/usr/bin/install -c -m 755 testscripts/Ubuntu/dlt-daemon $RPM_BUILD_ROOT/etc/init.d

%clean
rm -rf $RPM_BUILD_ROOT

%files
#/etc/init.d/dlt-daemon
/etc/dlt-system.conf
/etc/dlt.conf
/usr/share/dlt-filetransfer/dlt-test-filetransfer-file
/usr/share/dlt-filetransfer/dlt-test-filetransfer-image.png
%{_libdir}/libdlt.so.@PRINT_MAJOR_VERSION@
%{_libdir}/libdlt.so.@GENIVI_PROJECT_VERSION@
%{_libdir}/libdlt.so
%{_bindir}/dlt-system
%{_bindir}/dlt-convert
%{_bindir}/dlt-receive
%{_bindir}/dlt-adaptor-stdin
%{_bindir}/dlt-adaptor-udp
%{_bindir}/dlt-test-client
%{_bindir}/dlt-test-user
%{_bindir}/dlt-test-stress
%{_bindir}/dlt-test-internal
%{_bindir}/dlt-test-filetransfer

%attr(0755,root,root) 
%{_bindir}/dlt-daemon
%{_bindir}/dlt-example-user
%{_bindir}/dlt-example-user-func
%{_bindir}/dlt-example-filetransfer

%files devel
%{_libdir}/pkgconfig/*.pc
%{_includedir}/dlt/*.h
%{_libdir}/pkgconfig/automotive-dlt.pc

%pre

%post

%changelog
* Wed Nov 24 2010 dlt_maintainer <dlt_maintainer@genivi.org> 2.2.0
- Creation
