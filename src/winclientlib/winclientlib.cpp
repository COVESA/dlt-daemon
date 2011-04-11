/*******************************************************************************
**                                                                            **
**  SRC-MODULE: winclientLib.cpp                                              **
**                                                                            **
**  TARGET    : Windows                                                       **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision$
 * $LastChangedDate$
 * $LastChangedBy$
 */

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib

// Disable C4995 and C4996 Warnings
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#include "stdafx.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <io.h>

#include <string>
#include <iostream>

#include "winclientlib.h"
#include "dlt_client.h"

// Function prototypes
DWORD WINAPI MyThreadFunction( LPVOID lpParam );
void ErrorHandler(LPTSTR lpszFunction);

// Variables
static DWORD	dwThreadId;
static HANDLE   hThread;
static HANDLE 	hEvent;

static DltClient windltclient;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    {
        break;
    }
    }
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

using namespace std;

/*
Some helper functions
*/

DWORD WINAPI MyThreadFunction( LPVOID lpParam )
{
    // Enter Main Loop
    dlt_client_main_loop(&windltclient, NULL, 0);

    // Send event about thread termination
    SetEvent(hEvent);

    ExitThread(0);
}

void ErrorHandler(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code.
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;

    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message.
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                      (lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);

    MessageBox(NULL, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK);

    // Free error-handling buffer allocations.
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

/***
The interface functions
****/

WWINCLIENTLIB_API void Dlt_RegisterMessageCallback(int (*registerd_callback) (DltMessage *message, void *data))
{
    dlt_client_register_message_callback(registerd_callback);
}

WWINCLIENTLIB_API int Dlt_StartClient(char* server_address)
{
    WSADATA wsaData;
    int iResult;

    if ((server_address==0) || (server_address[0]=='\0'))
    {
        return 0;
    }

    // Create event, used for thread termination
    hEvent = CreateEvent(NULL,FALSE,FALSE,(LPCWSTR)"Test");

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult)
    {
        printf("winclientlib: WSAStartup failed: %d\n", iResult);
        return -1;
    }

    /* Initialize DLT Client */
    if (dlt_client_init(&windltclient, 0)==-1)
    {
        ErrorHandler(TEXT("dlt_client_init()"));

        Dlt_ExitClient();

        return -1;
    }

    /* Setup parameters of DltClient */
    windltclient.sock = -1;
    windltclient.serial_mode = 0;         /* TCP connection:
										     In Windows (with Visual C++),
										     only TCP connection is allowed! */
    windltclient.servIP = server_address; /* IP address */


    /* Connect to TCP socket */
    if (dlt_client_connect(&windltclient, 0)==-1)
    {
        ErrorHandler(TEXT("dlt_client_connect()"));

        Dlt_ExitClient();

        return -1;
    }

    // Create the thread to begin execution on its own.
    hThread = CreateThread(
                  NULL,                   // default security attributes
                  0,                      // use default stack size
                  MyThreadFunction,       // thread function name
                  0,//(LPVOID)address,    // argument to thread function
                  0,                      // use default creation flags
                  &dwThreadId);           // returns the thread identifier

    // Check the return value for success.
    // If CreateThread fails, terminate execution.
    // This will automatically clean up threads and memory.
    if (hThread==0)
    {
        ErrorHandler(TEXT("CreateThread()"));

        // Cleanup WSA
        WSACleanup();

        return -1;
    }

    return 0;
}

WWINCLIENTLIB_API int Dlt_InjectCall( char appID[4], char  contID[4], uint32_t serviceID, uint8_t *buf, uint32_t buf_len )
{
    return dlt_client_send_inject_msg(&windltclient, appID, contID, serviceID, buf, buf_len);
}

WWINCLIENTLIB_API int Dlt_ExitClient()
{
    printf("winclientlib: exiting ...\n");

    // Terminate thread and close handles
    if (windltclient.sock!=-1)
    {
        if (windltclient.serial_mode==1)
        {
            close(windltclient.sock);
        }
        else
        {
            closesocket(windltclient.sock);
        }
        windltclient.sock = -1;

        WaitForSingleObject(hEvent,INFINITE);
    }

    CloseHandle(hEvent);
    CloseHandle(hThread);

    // Dlt Client Cleanup
    if (dlt_client_cleanup(&windltclient,0)==-1)
    {
        printf("winclientlib: closing error.\n");
    }
    else
    {
        printf("winclientlib: closed.\n");
    }

    // Cleanup WSA
    WSACleanup();

    exit(0);
}
