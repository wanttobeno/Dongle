#if !defined(AFX_DBGPRINT_H_INCLUDED)
#define AFX_DBGPRINT_H_INCLUDED

#include <windows.h>
#include <stdio.h>

#ifdef _CONSOLE
#define dprintf printf
#else
#define DBG 1
#define dprintf if (DBG) MyDbgPrint
#endif // _CONSOLE


void	MyDbgPrint(const char * sz, ...);
VOID	GetErrorMessage( char *ErrorMsgBuffer, DWORD ErrCode );
//VOID	GetErrorMsgForNative( char *funcname, NTSTATUS status );

#endif // !defined(AFX_DBGPRINT_H_INCLUDED)