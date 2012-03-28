DLT  - Automotive Diagnostic Log and Trace

Version: 2.6.0

This component provides a standardised log and trace interface, based on the
standardised protocol specified in the AUTOSAR standard 4.0 DLT.
This component can be used by GENIVI components and other applications as
logging facility providing
- the DLT shared library
- the DLT daemon
- the DLT daemon adaptors
- the DLT client console utilities
- the DLT test applications

The DLT daemon is the central component in GENIVI, which gathers all 
logs and traces from the DLT user applications. The logs and traces 
are stored optionally directly in a file in the ECU. The DLT daemon 
forwards all logs and traces to a connected DLT client.
The DLT client can send control messages to the daemon, e.g. to set 
individual log levels of applications and contexts or get the list of 
applications and contexts registered in the DLT daemon.


Homepage
--------
https://collab.genivi.org/wiki/display/geniviproj/Automotive+DLT+%28Diagnostic+Log+and+Trace%29


License
-------
Full information on the license for this software
is available in the "LICENSE.txt" file. 
The full MPL license is in "MPL.txt."


Contact
-------
Alexander Wenzel (Alexander.AW.Wenzel@bmw.de)
Christian Muck (christian.muck@bmw.de)


Compiling in Linux:
-------------------
- mkdir build
- cd build
- cmake ..
- make
- optional: sudo make install
- optional: sudo ldconfig


Compile options with default values
-----------------------------------
- WITH_DLT_SHM_ENABLE =        OFF
- WITH_CHECK_CONFIG_FILE =     OFF
- WITH_DOC =                   OFF
- WITH_TESTSCRIPTS =           OFF
- WITH_SYSTEMD =               OFF
- WITH_GPROF =  	       OFF
- WITH_MAN =		       ON
- WITH_DLTTEST =               OFF
- BUILD_SHARED_LIBS =          ON
- CMAKE_INSTALL_PREFIX =       /usr/local
- CMAKE_BUILD_TYPE =           RelWithDebInfo


In order to change these options, you can modify this values
with ccmake, do the appropriate changes in CmakeList.txt or via 
the commandline for cmake
- Change a value with: cmake -D<Variable>=<Value>
- Example: cmake -DCMAKE_INSTALL_PREFIX=/usr


Man pages
---------
With the compile option "WITH_MAN=ON" (default value) the man 
pages will generated. After the call "sudo make install" (see 
"Compilin in Linux") they are installed at <CMAKE_INSTALL_PREFIX>/share/man. 

To get more informations, call e.g.
- man dlt-daemon
- man dlt.conf
- man dlt-convert
- man dlt-receive
- man dlt-system
- man dlt-system.conf


Create doxygen documentation
----------------------------
- mkdir build
- cd build
- cmake -DWITH_DOC=ON ..
- make 
- (only DLT doc - optional )make doc
- (only DLT-Filetransfer - optional )make doc-filetransfer

You find the documentation know as HTML, RTF or LaTex in <project-root>/build/doc
