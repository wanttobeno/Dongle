// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__DA6335BB_3ACA_409B_8CC0_6186874A5E0D__INCLUDED_)
#define AFX_STDAFX_H__DA6335BB_3ACA_409B_8CC0_6186874A5E0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x502
#define _CRT_SECURE_NO_WARNINGS
// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(disable : 4819)

#include <windows.h>
#include <stdlib.h>
#include <stdio.H>
#include <setupapi.h>

extern "C" {
#include "HID/hidsdi.h"
};

#include "DbgPrint.h"

#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"HID/HID.lib")

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__DA6335BB_3ACA_409B_8CC0_6186874A5E0D__INCLUDED_)
