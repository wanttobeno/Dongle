#include "StdAfx.h"
#include "DbgPrint.h"

char Debug_szErrorMsg[1024]={0};
char Debug_szErrorInfo[1024]={0};

//************************************
// Method:    MyDbgPrint
// FullName:  MyDbgPrint
// Purpose:   增强版的OutputDebugString，使用起来更方便
// Returns:   void
// Parameter: const char * sz
// Parameter: ...
//************************************
void MyDbgPrint(const char * sz, ...)
{
#ifdef DBG
	char szData[512]={0};
	
	va_list args;
	va_start(args, sz);
	_vsnprintf(szData, sizeof(szData) - 1, sz, args);
	va_end(args);
	
	OutputDebugString(szData);
#endif
}

/********************************************************************
	Function Name: 	GetErrorMessage
	Function purpose:	获取给定的状态码所代表的错误含义
	Input Arguments :	
		ErrorMsgBuffer:缓冲区，返回的内容放在这里
		ErrCode		:错误状态码
	ReturnValue:	无

*********************************************************************/
void GetErrorMessage( char *ErrorMsgBuffer, DWORD ErrCode )
{
    char *errMsg;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		ErrCode,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		( LPTSTR )&errMsg, 0, NULL );
	lstrcpy(ErrorMsgBuffer,errMsg);
    LocalFree( errMsg );
    return;
}