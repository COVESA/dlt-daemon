/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * For further information see http://www.genivi.org/.
 * @licence end@
 */
 
/** \mainpage 

\image html genivilogo.png

\par More information
can be found at https://collab.genivi.org/wiki/display/genivi/GENIVI+Home \n

\par About DLT
The DLT is a Deamon that enables diagnostic log and trace in a GENIVI headunit and is based on AUTOSAR 4.0.

DLT Filetransfer Main documentation page

\section Introduction Introduction
DLT Filetransfer enables the feature to store the binary data of a file to the automotive dlt log. The file will
be read in binary mode and put as payload to a DLT_INFO log. With a special plugin of the dlt viewer, you can
extract the file from the trace and save it to your hard disk.


\section Protocol Protocol
The file transfer is at least one single transaction. This transaction consist of three main types of packages:
	\li header package
	\li one or more data packages
	\li end package

The following section explains more about the content of each package.

\subsection Header Header Package
Every filetransfer must begin with the header package using:
\code
int dlt_user_log_file_header(DltContext *fileContext,const char *filename)
\endcode
Here's the conent of the header package:
\code
|----------------------------------------------------|
|                      FLST                          | Package flag
|----------------------------------------------------|
|                fileserialnumber                    | Inode of the file used as file serialnumber
|----------------------------------------------------|
|                    filename                        | Use the absolute filepath to the file
|----------------------------------------------------|
|                    filesize                        | Filesize of the file
|----------------------------------------------------|
|               file creation date                   | Creation date of the file
|----------------------------------------------------|
|               number of packages                   | Counted packages which will be transferred in the data packages
|----------------------------------------------------|
|                   BUFFER_SIZE                      | Defined buffer size to reconstruct the file
|----------------------------------------------------|
|                      FLST                          | Package flag
|----------------------------------------------------|
\endcode

\subsection Data Data Package
After the header package was sent, at least one or more data packages can be send using:
\code
int dlt_user_log_file_data(DltContext *fileContext,const char *filename, int packageToTransfer, int timeout)
\endcode
Here's the conent of the data package:
\code
|----------------------------------------------------|
|                      FLDA                          | Package flag
|----------------------------------------------------|
|                fileserialnumber                    | Inode of the file used as file serialnumber
|----------------------------------------------------|
|                  PackageNumber                     | Transferred package
|----------------------------------------------------| 
|                      Data                          | Payload containing data
|----------------------------------------------------|
|                      FLDA                          | Package flag
|----------------------------------------------------|
\endcode


\subsection End End Package
After all data packages were sent, the end package must be sent to indicate that the filetransfer is over using:
\code
int dlt_user_log_file_end(DltContext *fileContext,const char *filename,int deleteFlag)
\endcode
Here's the conent of the end package:
\code
|----------------------------------------------------|
|                      FLFI                          | Package flag
|----------------------------------------------------|
|                fileserialnumber                    | Inode of the file
|----------------------------------------------------|
|                      FLFI                          | Package flag
|----------------------------------------------------|
\endcode
\subsection File information
The library offers the user the possibility to log informations about a file using the following method without transferring the file itself using:
\code
dlt_user_log_file_infoAbout(DltContext *fileContext, const char *filename)
\endcode
Here is the content of the information package:
\code
|----------------------------------------------------|
|                      FLIF                          | Package flag
|----------------------------------------------------|
|                fileserialnumber                    | Inode of the file used as file serialnumber
|----------------------------------------------------|
|                    filename                        | Use the absolute filepath to the file
|----------------------------------------------------|
|                    filesize                        | Filesize of the file
|----------------------------------------------------|
|               file creation date                   | Creation date of the file
|----------------------------------------------------|
|               number of packages                   | Counted packages which will be transferred in the data packages
|----------------------------------------------------|
|                      FLIF                          | Package flag
|----------------------------------------------------|
\endcode

\subsection File transfer error
\code
//! Error code for dlt_user_log_file_complete
#define ERROR_FILE_COMPLETE -300
//! Error code for dlt_user_log_file_complete
#define ERROR_FILE_COMPLETE1 -301
//! Error code for dlt_user_log_file_complete
#define ERROR_FILE_COMPLETE2 -302
//! Error code for dlt_user_log_file_complete
#define ERROR_FILE_COMPLETE3 -303
//! Error code for dlt_user_log_file_head
#define ERROR_FILE_HEAD -400
//! Error code for dlt_user_log_file_data
#define ERROR_FILE_DATA -500
//! Error code for dlt_user_log_file_data
#define DLT_FILETRANSFER_ERROR_FILE_DATA_USER_BUFFER_FAILED -501
//! Error code for dlt_user_log_file_end
#define ERROR_FILE_END -600
//! Error code for dlt_user_log_file_infoAbout
#define ERROR_INFO_ABOUT -700
//! Error code for dlt_user_log_file_packagesCount
#define ERROR_PACKAGE_COUNT -800
\endcode
If an error happens during file transfer, the library will execute the mehtod:
\code
void dlt_user_log_file_errorMessage(DltContext *fileContext, const char *filename, int errorCode)
\endcode
Here is the content of the error package:
\code
|----------------------------------------------------|
|                      FLER                          | Package flag
|----------------------------------------------------|
|                   error code                       | see error codes above
|----------------------------------------------------|
|                linux error code                    | standard linux error code
|----------------------------------------------------|
|                fileserialnumber                    | Inode of the file used as file serialnumber
|----------------------------------------------------|
|                    filename                        | Use the absolute filepath to the file
|----------------------------------------------------|
|                    filesize                        | Filesize of the file
|----------------------------------------------------|
|               file creation date                   | Creation date of the file
|----------------------------------------------------|
|               number of packages                   | Counted packages which will be transferred in the data packages
|----------------------------------------------------|
|                      FLER                          | Package flag
|----------------------------------------------------|
\endcode
If the file doesn't exist, the conent of the error package is a little bit different:
\code
|----------------------------------------------------|
|                      FLER                          | Package flag
|----------------------------------------------------|
|                   error code                       | see error codes above
|----------------------------------------------------|
|                linux error code                    | standard linux error code
|----------------------------------------------------|
|                    filename                        | Use the absolute filepath to the file
|----------------------------------------------------|
|                      FLER                          | Package flag
|----------------------------------------------------|
\endcode

\section Using Using DLT Filetransfer
There are two ways to use the filetransfer
	\li Automatic filetransfer in one step
	\li Header, data and end package order handeld by the user

\subsection Automatic Automatic 
Call
	\li dlt_user_log_file_complete
	
The method needs the following arguments:
	\li fileContext -> Context for logging the file to dlt
	\li filename -> Use the absolute file path to the file
	\li deleteFlag -> Flag if the file will be deleted after transfer. 1->delete, 0->notDelete
	\li timeout -> Deprecated.
	
The order of the packages is to send at first the header, then one or more data packages (depends on the filesize) and in the end the end package.
The advantage of this method is, that you must not handle the package ordering by your own.

Within dlt_user_log_file_complete the free space of the user buffer will be checked. If the free space of the user buffer < 50% then the
actual package won't be transferred and a timeout will be executed.

If the daemon crashes and the user buffer is full -> the automatic method is in an endless loop.

\subsection Manual Manual
Manual starting filetransfer with the following commands:
	\li dlt_user_log_file_head | Transfers only the header of the file
	\li dlt_user_log_file_data | Transfers only one single package of a file
    \li dlt_user_log_file_end | Tranfers only the end of the file

This ordering is very important, so that you can save the transferred files to hard disk on client side with a dlt viewer plugin.
The advantage of using several steps to transfer files by your own is, that you are very flexible to integrate the filetransfer
in your code.

An other difference to the automatic method is, that only a timeout will be done. There is no check of the user buffer.

\subsection Important Important for integration
You should care about blocking the main program when you intergrate filetransfer in your code.
Maybe it's useful to extract the filetransfer in an extra thread.
Another point is the filesize. The bigger the file is, the longer takes it to log the file to dlt.

\section Example Example dlt filetransfer
For an example file transfer you can use
\code
Usage: dlt-example-filetransfer [options] <command>
Filetransfer example with DLT Package Version: 2.2.0 , Package Revision: 1666, build on May 28 2011 02:18:19
 
Command:
-f file      - File to transfer (absolute path)
Options:
-a apid      - Set application id to apid (default: FLTR)
-c ctid      - Set context id to ctid (default: FLTR)
-t ms        - Timeout between file packages in ms (minimum 20 ms)
-d           - Flag to delete the file after the transfer (default: false)
-i           - Flag to log file infos to DLT before transfer file (default: false)
-h           - This help
\endcode

\section Test Testing dlt filetransfer
When you call "sudo make install", some automatic tests will be installed. Start the test using the following command from bash:
\code
dlt-test-filetransfer
\endcode
It's important that the dlt-filetransfer example files are installed in /usr/share/dlt-filetransfer which will be done automatically by using "sudo make install".
\subsection testFile1Run1
Test the file transfer with the condition that the transferred file is smaller as the file transfer buffer using dlt_user_log_file_complete.
\subsection testFile1Run2
Test the file transfer with the condition that the transferred file is smaller as the file transfer buffer using single package transfer 
\subsection testFile2Run1
Test the file transfer with the condition that the transferred file is bigger as the file transfer buffer using dlt_user_log_file_complete.
\subsection testFile2Run2
Test the file transfer with the condition that the transferred file is bigger as the file transfer buffer using single package transfer
\subsection testFile3Run1
Test the file transfer with the condition that the transferred file does not exist using dlt_user_log_file_complete.
\subsection testFile3Run2
Test the file transfer with the condition that the transferred file does not exist using single package transfer
\subsection testFile3Run3
Test which logs some information about the file.




\section Requirements Requirements
 \code
 automotive-dlt
 \endcode
 <hr>

\section Licence Licence
Copyright 2011 - BMW AG, Christian Muck <christian.muck@bmw.de>

* */
