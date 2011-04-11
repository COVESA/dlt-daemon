/*******************************************************************************
**                                                                            **
**  SRC-MODULE: wintestclient.cpp                                             **
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

#include "stdafx.h"
#include "winclientlib.h"

#include <string.h>

static int counter=0;

int My_message_Callback(DltMessage *message, void *data)
{
	counter++;

	printf("Message received, %d\n", counter);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char text[] = "Hello from WinTestClient";

	printf("WinTestClient\n");

	Dlt_RegisterMessageCallback(My_message_Callback);

    Dlt_StartClient("192.168.56.101");

    Dlt_InjectCall("LOG","TEST",0xFFF,(uint8_t*)text,strlen(text)+1);

    getchar();

    Dlt_ExitClient();

	return 0;
}

