/*******************************************************************************
**                                                                            **
**  SRC-MODULE: winClientLib.h                                                **
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

#include "dlt_common.h"  // for DltMessage

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the WWINCLIENTLIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// WWINCLIENTLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINCLIENTLIB_EXPORTS
#define WWINCLIENTLIB_API __declspec(dllexport)
#else
#define WWINCLIENTLIB_API __declspec(dllimport)
#endif

WWINCLIENTLIB_API int  Dlt_StartClient(char* server_address);
WWINCLIENTLIB_API int  Dlt_ExitClient();
WWINCLIENTLIB_API int  Dlt_InjectCall( char appID[4], char contID[4], uint32_t serviceID, uint8_t *buf, uint32_t buf_len );
WWINCLIENTLIB_API void Dlt_RegisterMessageCallback(int (*registerd_callback) (DltMessage *message, void *data));
