# DLT Filetransfer

Back to [README.md](../README.md)

## Overview

DLT is a reusable open source software component for standardized logging and
tracing in infotainment ECUs based on the AUTOSAR 4.0 standard.

The goal of DLT is the consolidation of the existing variety of logging and
tracing protocols on one format.

## Introduction to DLT Filetransfer

With DLT Filetransfer it is possible store the binary data of a file to the
automotive dlt log.

The file will be read in binary mode and put as several chunks to a DLT\_INFO
log. With a special plugin of the dlt viewer, you can extract the embedded files
from the trace and save them.

It can be used for smaller files, e.g. HMI screenshots or little coredumps.

## Protocol

The file transfer is at least one single transaction. This transaction consist
of three main types of packages:

- header package
- one or more data packages
- end package

## Header Package

Every filetransfer must begin with the header package using:

` int dlt_user_log_file_header(DltContext *fileContext,const char *filename) `

Header Header Package Protocol:

Value | Description
:--- | :---
FLST | Package flag
fileserialnumber | Inode of the file used as file serialnumber
filename | Use the absolute filepath to the file
filesize | Filesize of the file
file creation date | Creation date of the file
number of packages | Counted packages which will be transferred in the data packages
BUFFER_SIZE | Defined buffer size to reconstruct the file
FLST | Package flag

## Data Package

After the header package was sent, at least one or more data packages can be
sent using:

` int dlt_user_log_file_data(DltContext *fileContext,const char *filename,int packageToTransfer, int timeout) `

Data Data Package Protocol:

Value | Description
:--- | :---
FLDA | Package flag
fileserialnumber | Inode of the file used as file serialnumber
PackageNumber | Transferred package
Data | Payload containing data
FLDA | Package flag

## End Package

After all data packages were sent, the end package must be sent to indicate that
the filetransfer is over using:

` int dlt_user_log_file_end(DltContext *fileContext,const char *filename,int deleteFlag) `

End Package Protocol:

Value | Description
:--- | :---
FLFI | Package flag
fileserialnumber | Inode of the file
FLFI | Package flag

## File information

The library offers the user the possibility to log informations about a file
using the following method without transferring the file itself using:

` dlt_user_log_file_infoAbout(DltContext *fileContext, const char *filename) `

File Information Protocol:

Value | Description
:--- | :---
FLIF | Package flag
fileserialnumber | Inode of the file used as file serialnumber
filename | Use the absolute filepath to the file
filesize | Filesize of the file
file creation date | Creation date of the file
number of packages | Counted packages which will be transferred in the data packages
FLIF | Package flag

## File transfer error

```c
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
```

If an error happens during file transfer, the library will execute the method:

` void dlt_user_log_file_errorMessage(DltContext *fileContext, const char *filename, int errorCode) `

File transfer error Protocol:

Value | Description
:--- | :---
FLER | Package flag
error code | see error codes above
linux error code | standard linux error code
fileserialnumber | Inode of the file used as file serialnumber
filename | Use the absolute filepath to the file
filesize | Filesize of the file
file creation date | Creation date of the file
number of packages | Counted packages which will be transferred in the data packages
FLER | Package flag

If the file doesn't exist, the content of the error package is a little bit
different:

Value | Description
:--- | :---
FLER | Package flag
error code | see error codes above
linux error code | standard linux error code
filename | Use the absolute filepath to the file
FLER | Package flag

## Using Using DLT Filetransfer

There are two ways to use the filetransfer

- Automatic filetransfer in one step
- Header, data and end package order handeld by the user

### Automatic

Call

- dlt\_user\_log\_file\_complete

The method needs the following arguments:

- fileContext -> Context for logging the file to dlt
- filename -> Use the absolute file path to the file
- deleteFlag -> Flag if the file will be deleted after transfer. 1->delete, 0->notDelete
- timeout -> Deprecated.

The order of the packages is to send at first the header, then one or more data
packages (depends on the filesize) and in the end the end package. The advantage
of this method is, that you must not handle the package ordering by your own.

Within dlt\_user\_log\_file\_complete the free space of the user buffer will be
checked. If the free space of the user buffer < 50% then the actual package
won't be transferred and a timeout will be executed.

If the daemon crashes and the user buffer is full, the automatic method is in an
endless loop.

### Manual

Manual starting filetransfer with the following commands:

- dlt\_user\_log\_file\_head | Transfers only the header of the file
- dlt\_user\_log\_file\_data | Transfers only one single package of a file
- dlt\_user\_log\_file\_end | Tranfers only the end of the file

This ordering is very important, so that you can save the transferred files to
hard disk on client side with a dlt viewer plugin. The advantage of using
several steps to transfer files by your own is, that you are very flexible to
integrate the filetransfer in your code.

An other difference to the automatic method is, that only a timeout will be
done. There is no check of the user buffer.

## Important for integration

You should care about blocking the main program when you intergrate filetransfer
in your code. Maybe it's useful to extract the filetransfer in an extra thread.
Another point is the filesize. The bigger the file is, the longer takes it to
log the file to dlt.

## Example dlt filetransfer

For an example file transfer you can use

```bash
Usage: dlt-example-filetransfer \[options\] \<command\>
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
```

## Testing dlt filetransfer

When you call "sudo make install", some automatic tests will be installed. Start
the test using the following command from bash:

` dlt-test-filetransfer `

It's important that the dlt-filetransfer example files are installed in
/usr/share/dlt-filetransfer which will be done automatically by using
"sudo make install". If not, use -t and -i options to specify the path to a text
file and an image file.

- testFile1Run1: Test the file transfer with the condition that the transferred file is smaller as the file transfer buffer using dlt\_user\_log\_file\_complete.
- testFile1Run2: Test the file transfer with the condition that the transferred file is smaller as the file transfer buffer using single package transfer
- testFile2Run1: Test the file transfer with the condition that the transferred file is bigger as the file transfer buffer using dlt\_user\_log\_file\_complete.
- testFile2Run2: Test the file transfer with the condition that the transferred file is bigger as the file transfer buffer using single package transfer
- testFile3Run1: Test the file transfer with the condition that the transferred file does not exist using dlt\_user\_log\_file\_complete.
- testFile3Run2: Test the file transfer with the condition that the transferred file does not exist using single package transfer
- testFile3Run3: Test which logs some information about the file.

## AUTHOR

Christian Muck <Christian (dot) Muck (at) bmw (dot) de>

## COPYRIGHT

Copyright (C) 2012 - 2015 BMW AG. License MPL-2.0: Mozilla Public License version 2.0 <http://mozilla.org/MPL/2.0/>.
