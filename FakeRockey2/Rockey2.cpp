// Rockey2.cpp : Defines the entry point for the DLL application.
// Author: achillis

#include "stdafx.h"
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include "Rockey2.h"

#pragma comment(lib,"shlwapi.lib")

#define RY2_SUCCESS	(0)

VOID InitPaths();

char g_szConfigFilePath[MAX_PATH]= {0};
HMODULE g_hMod = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)hModule);
		g_hMod = (HMODULE)hModule;
		InitPaths();
	}
    return TRUE;
}

VOID InitPaths()
{
	char szPath[MAX_PATH] = {0};
	GetModuleFileName(g_hMod,szPath,MAX_PATH);
	PathRemoveFileSpec(szPath);
	lstrcpy(g_szConfigFilePath,szPath);
	lstrcat(g_szConfigFilePath,"\\Rockey2.ini");
}

int WINAPI RY2_Find()
{
	//返回值表示找到的狗的个数，返回1表示找到了一个狗
	return 1;
}

int WINAPI RY2_Open(int mode,DWORD uid,DWORD *hid)
{
	//返回值表示打开的狗的序号，返回0表示打开第0个狗成功,并返回HID的值
	*hid = 0x11223344;
	return 0;
}

void WINAPI RY2_Close(int handle)
{
	//do nothing
}

int WINAPI RY2_GenUID(int handle,DWORD *uid, char *seed,int isProtect)
{
	return RY2_SUCCESS;
}

int WINAPI RY2_Read(int handle,int block_index,char *buffer512)
{
	//如果是字符串的话，从配置文件中读取比较方便
	//否则可以修改为从文件中读取，可自行发挥
	char szBuffer[512] = {0};
	char szBlockName[64] = {0};
	sprintf(szBlockName,"Block%d",block_index);
	GetPrivateProfileString("CONFIG",szBlockName,"",szBuffer,512,g_szConfigFilePath);
	strcpy(buffer512,szBuffer);
	return RY2_SUCCESS;
}

int WINAPI RY2_Write(int handle,int block_index,char *buffer512)
{
	char szBlockName[64] = {0};
	sprintf(szBlockName,"Block%d",block_index);
	WritePrivateProfileString("CONFIG",szBlockName,buffer512,g_szConfigFilePath);
	return RY2_SUCCESS;
}

int WINAPI RY2_Transform(int handle, int len, BYTE *data)
{
	return RY2_SUCCESS;
}

int WINAPI RY2_GetVersion(int handle)
{
	//要求返回版本号，这里随便给一个
	//某些版本的Rockey2.dll提供的该函数接口可能不一样
	return 0x010F;
}