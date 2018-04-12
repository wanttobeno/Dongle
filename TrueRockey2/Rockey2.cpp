// Rockey2.cpp : Defines the entry point for the DLL application.
// Author: achillis

#include "stdafx.h"
#include "../Inc/Rockey2.h"

typedef struct _ROCKEY_INFO
{
    char szDevicePath[256];
    HANDLE hDevice;
    ULONG dwUID;
    ULONG dwHID;
    USHORT uVersion;
    HANDLE hMutex;
}ROCKEY_INFO;

#define MAX_DONGLE_COUNT	(32)
#define RY2_VID			(0x096E)
#define RY2_PID			(0x0201)
#define RY2_REPORTLEN		(73)


VOID RockeyInit();
int  CheckDeviceReportLength(HANDLE hDevice);
VOID CloseAllDogs();
int  RockeySetHidFeature(HANDLE HidDeviceObject, PVOID ReportBuffer);
int  RockeyGetHidFeature(HANDLE HidDeviceObject, PVOID ReportBuffer);
int  ConvertErrCode(BYTE Code);
int  RockeyHidReadWrite(int handle, BYTE *pInputBuffer, BYTE *pOutputBuffer);
VOID UnicodeStrToAnsi(char *a1);
int  HexStrToInt(char *szStr);

BOOL g_bUninited = TRUE;
int g_DogCount = 0 ;
ROCKEY_INFO g_RockeyInfo[MAX_DONGLE_COUNT] = {0};


VOID RockeyInit()
{
	int  i = 0 ;
	char szName[256] = {0}; 

	//初始化Mutex用于对设备的互斥使用
	for (i=0;i<MAX_DONGLE_COUNT;i++)
	{
		wsprintfA(szName, "ROCKEY2_MUTEX%02d", i);
		g_RockeyInfo[i].hMutex = CreateMutex(NULL, TRUE, szName);
	}

	g_bUninited = FALSE;
}


int  CheckDeviceReportLength(HANDLE hDevice)
{
	int ret = 0; 
	PHIDP_PREPARSED_DATA pData;
	HIDP_CAPS HidCaps = {0};
	
	if (!HidD_GetPreparsedData(hDevice, &pData) )
		return RY2ERR_GET_PREPARSED_DATA;
	
	if ( HidP_GetCaps(pData, &HidCaps) )
		ret = (HidCaps.FeatureReportByteLength != RY2_REPORTLEN) ? RY2ERR_UNKNOWN_DEVICE : RY2ERR_SUCCESS;
	else
		ret = RY2ERR_GETCAPS;

	if ( !HidD_FreePreparsedData(pData) )
		ret = RY2ERR_FREE_PREPARSED_DATA;
	
	return ret;
	
}

void CloseAllDogs()
{
	for (int i = 0 ; i < MAX_DONGLE_COUNT ; i++)
	{
		RY2_Close(i);
	}
}

int  RockeySetHidFeature(HANDLE HidDeviceObject, PVOID ReportBuffer)
{
	return HidD_SetFeature(HidDeviceObject, ReportBuffer, RY2_REPORTLEN) != 0 ? RY2ERR_SUCCESS : RY2ERR_WRITE_REPORT;
}

int RockeyGetHidFeature(HANDLE HidDeviceObject, PVOID ReportBuffer)
{
	int result;
	
	memset(ReportBuffer, 0, RY2_REPORTLEN);
	if (!HidD_FlushQueue(HidDeviceObject) )
		return RY2ERR_FLUSH_QUEUE;

	result = (HidD_GetFeature(HidDeviceObject, ReportBuffer, RY2_REPORTLEN) != 0) ? RY2ERR_SUCCESS : RY2ERR_READ_REPORT;
	return result;
}


int  ConvertErrCode(BYTE Code)
{
	int result;
	
	if ( Code != 0)
	{
		switch ( Code )
		{
		case 1:
			result = RY2ERR_VERIFY;
			break;
		case 2:
			result = RY2ERR_WRONG_UID;
			break;
		case 4:
			result = RY2ERR_WRONG_INDEX;
			break;
		default:
			result = (Code != 8) ? RY2ERR_UNKNOWN_ERROR : RY2ERR_WRITE_PROTECT;
			break;
		}
	}
	else
	{
		result = RY2ERR_SUCCESS;
	}
	return result;
}

int RockeyHidReadWrite(int handle, BYTE *pInputBuffer, BYTE *pOutputBuffer)
{
	int result;
	
	result = RockeySetHidFeature(g_RockeyInfo[handle].hDevice, pInputBuffer);
	if ( result == RY2ERR_SUCCESS )
	{
		result = RockeyGetHidFeature(g_RockeyInfo[handle].hDevice, pOutputBuffer);
		if ( result == RY2ERR_SUCCESS  )
		{
			result = ConvertErrCode(pOutputBuffer[1]);
			if ( result == RY2ERR_SUCCESS )
				result = (pInputBuffer[2] != pOutputBuffer[2]) ? RY2ERR_UNKNOWN_ERROR : 0;
		}
	}
	return result;
}

//设备返回的字串均为UNICODE，但都是ASCII字符，所以可以使用简单的方法转换为ANSI String
VOID UnicodeStrToAnsi(char *szAnsi)
{
	int i; 
	char *pWideChar = szAnsi;
	
	for (i=0;i<1024;i++)
	{
		szAnsi[i] = *pWideChar;
		if ( *pWideChar == 0 )
			break;

		pWideChar += 2;
	}

	memset(szAnsi + i , 0, 1024 - i);
}

//功能：把字符串"287073C9"转化为值0x287073C9
int  HexStrToInt(char *szStr)
{
	int result = 0;
	int i;
	char ch;
	int TempValue;
	
	for (i=0;i<8;i++)
	{
		ch = szStr[i];
		if ( ch >= '0' && ch <= '9' )
		{
			TempValue = ch - '0';
			result = TempValue + 16 * result;
		}
		else if ( ch >= 'a' && ch <= 'f' )
		{
			TempValue = ch - 'a' + 0xA;
			result = TempValue + 16 * result;
		}
		else if ( ch >= 'A' && ch <= 'F' )
		{
			TempValue = ch - 'A' + 0xA;
			result = TempValue + 16 * result;
		}
	}

	return result;
}


int WINAPI RY2_Find()
{
	int errcode = 0;
	HDEVINFO hDeviceInfoSet = NULL ; 
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	int result; 
	int MemberIndex; 
	DWORD RequiredSize = 0; 
	HIDD_ATTRIBUTES Attributes ;
	GUID HidGuid;
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	char szString[1024];
	BYTE DetailData[4096];
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDeviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA *)DetailData;
	
	//初始化和清理工作
	if ( g_bUninited )
	{
		RockeyInit();
		g_bUninited = FALSE;
	}
	g_DogCount = 0;
	CloseAllDogs();
	
	//获取HID设备的ClassId并枚举设备
	HidD_GetHidGuid(&HidGuid);
	hDeviceInfoSet = SetupDiGetClassDevs(&HidGuid, 0, 0, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
	if ( hDeviceInfoSet == INVALID_HANDLE_VALUE )
		return RY2ERR_SETUP_DI_CLASS_DEVS;
	
	memset(&DeviceInterfaceData, 0, sizeof(DeviceInterfaceData));
	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
	
	MemberIndex = 0;
	while ( MemberIndex < 128 )
	{
		ZeroMemory(DetailData, sizeof(SP_DEVINFO_DATA));
		if ( !SetupDiEnumDeviceInterfaces(hDeviceInfoSet, 0, &HidGuid, MemberIndex++, &DeviceInterfaceData) )
			break;

		SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet, &DeviceInterfaceData, 0, 0, &RequiredSize, 0);
		if (RequiredSize > 4096 )
		{
			errcode = RY2ERR_TOO_LONG_DEVICE_DETAIL;
			goto FAILEXIT;
		}
		memset(DetailData, 0, 4096);
		pDeviceInterfaceDetailData->cbSize = 5;
		if ( !SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet, &DeviceInterfaceData, pDeviceInterfaceDetailData, RequiredSize, &RequiredSize, 0) )
		{
			errcode = RY2ERR_SETUP_DI_GET_DEVICE_INTERFACE_DETAIL;
			goto FAILEXIT;
		}
		
		//printf("DevicePath : %s\n",pDeviceInterfaceDetailData->DevicePath);
		hDevice = CreateFile(pDeviceInterfaceDetailData->DevicePath, 
			GENERIC_READ|GENERIC_WRITE, 
			FILE_SHARE_READ|FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			0);
		
		if (hDevice == INVALID_HANDLE_VALUE )
			continue;
		
		Attributes.Size = sizeof(HIDD_ATTRIBUTES);
		if (!HidD_GetAttributes(hDevice, &Attributes) )
		{
			errcode = RY2ERR_GET_ATTRIBUTES;
			goto _NEXTLOOP;
		}
		
		//比对VID和PID
		if ( Attributes.VendorID == RY2_VID && Attributes.ProductID == RY2_PID )
		{
			errcode = CheckDeviceReportLength(hDevice);
			if ( errcode == RY2ERR_SUCCESS )
			{
				if (!HidD_GetProductString(hDevice, szString, 1024) )
				{
					errcode = RY2ERR_GET_PRODUCT_STRING;
					goto _NEXTLOOP;
				}
				
				UnicodeStrToAnsi(szString);
				if ( strcmp(szString, "USB DONGLE") != 0 )
				{
					errcode = RY2ERR_UNKNOWN_DEVICE;
					goto _NEXTLOOP;
				}
				
				if (!HidD_GetSerialNumberString(hDevice, szString, 1024) )
				{
					errcode = RY2ERR_GET_SERIAL;
					goto _NEXTLOOP;
				}

				/*SerialNumber为如下形式:
				0018FA9C  32 00 41 00 39 00 35 00 44 00 34 00 42 00 34 00  2.A.9.5.D.4.B.4.
				0018FAAC  35 00 36 00 33 00 30 00 37 00 33 00 43 00 39 00  2.8.7.0.7.3.C.9.

				前16字节是HID，后面部分与生成UID有关
				*/
				
				UnicodeStrToAnsi(szString);
				g_RockeyInfo[g_DogCount].dwHID = HexStrToInt(szString);
				g_RockeyInfo[g_DogCount].dwUID = HexStrToInt(szString + 8) ^ g_RockeyInfo[g_DogCount].dwHID;
				g_RockeyInfo[g_DogCount].uVersion = Attributes.VersionNumber;
				strcpy(g_RockeyInfo[g_DogCount].szDevicePath, pDeviceInterfaceDetailData->DevicePath);

				dprintf("[DOG_%d] UID = 0x%08X HID = 0x%08X Ver = 0x%04X\n",
					g_DogCount,
					g_RockeyInfo[g_DogCount].dwUID,
					g_RockeyInfo[g_DogCount].dwHID,
					g_RockeyInfo[g_DogCount].uVersion);

				dprintf("[DOG_%d] DevicePath : %s\n",g_DogCount,g_RockeyInfo[g_DogCount].szDevicePath);

				g_DogCount++;
			}
		}
		
_NEXTLOOP:
		if (hDevice != INVALID_HANDLE_VALUE )
		{
			CloseHandle(hDevice);
			hDevice = INVALID_HANDLE_VALUE ;
		}
		
	}

	if ( g_DogCount != 0)
		errcode = RY2ERR_SUCCESS;

FAILEXIT:
	SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
	result = errcode;

	//如果整个过程没有失败，就返回实际找到的狗的个数，可能为0
	if ( errcode == RY2ERR_SUCCESS )
		result = g_DogCount;

	return result;
}


int WINAPI RY2_Open(int mode, DWORD uid, DWORD *hid)
{
	int MatchedDogCnt = 0; 
	int Index;
	int result;
	HANDLE hDevice;
	BOOL bFound = FALSE ;
	
	if ( g_DogCount <= 0 )
		return RY2ERR_NO_SUCH_DEVICE;
	
	if (mode == AUTO_MODE)
	{
		//打开第一个找到的加密锁，不理会具体的UID和HID
		Index = 0;
		bFound = TRUE;
	}
	else if (mode == HID_MODE)
	{
		//按HID打开，需要HID相匹配
		for (Index = 0;Index < g_DogCount;Index++)
		{
			if (g_RockeyInfo[Index].dwHID == *hid)
			{
				bFound = TRUE ;
				break;
			}
		}
	}
	else
	{
		//按UID打开，此时mode是UID为指定值的第几把锁
		for (Index = 0;Index < g_DogCount;Index++)
		{
			if (g_RockeyInfo[Index].dwUID == uid)
			{
				MatchedDogCnt++;
				if (mode == MatchedDogCnt)
				{
					bFound = TRUE ;
					break;
				}	
			}
		}
	}

	if (!bFound)
		return RY2ERR_NO_SUCH_DEVICE;

	*hid = g_RockeyInfo[Index].dwHID;
	hDevice = CreateFile(g_RockeyInfo[Index].szDevicePath, 
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);

	g_RockeyInfo[Index].hDevice = hDevice;
	if (hDevice != INVALID_HANDLE_VALUE )
		result = Index;
	else
		result = RY2ERR_OPEN_DEVICE;
	
	return result;
}


void WINAPI RY2_Close(int handle)
{
	if ( g_RockeyInfo[handle].hDevice != NULL && g_RockeyInfo[handle].hDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_RockeyInfo[handle].hDevice);
		g_RockeyInfo[handle].hDevice = NULL;
	}
}


int WINAPI RY2_GenUID(int handle, DWORD *uid, char *seed, int isProtect)
{
	int result; 
	int retcode; 
	BYTE InBuffer[RY2_REPORTLEN];
	BYTE OutBuffer[RY2_REPORTLEN];
	
	if (handle < 0 || handle > g_DogCount)
		return RY2ERR_NO_SUCH_DEVICE;

	memset(InBuffer, 0, RY2_REPORTLEN);
	InBuffer[1] = 0;
	InBuffer[2] = 0x87;
	if ( isProtect )
	{
		InBuffer[3] = 0;
		InBuffer[4] = 0;
	}
	else
	{
		InBuffer[3] = 0xFF;
		InBuffer[4] = 0xFF;
	}
	*(DWORD *)(InBuffer + 5 ) = 0;
	
	if ( strlen(seed) > 64 )
		return RY2ERR_TOO_LONG_SEED;
	
	
	memcpy(InBuffer + 9, seed, strlen(seed));
	WaitForSingleObject(g_RockeyInfo[handle].hMutex, INFINITE);
	retcode = RockeyHidReadWrite(handle, (BYTE *)InBuffer, (BYTE *)OutBuffer);
	ReleaseMutex(g_RockeyInfo[handle].hMutex);
	if ( retcode )
	{
		result = retcode;
	}
	else
	{
		*uid = *(DWORD *)(OutBuffer + 9);
		result = 0;
	}
	
	
	return result;
}

int WINAPI RY2_Transform(int handle, int len, BYTE *data)
{
	int retcode; 
	int result; 
	BYTE InBuffer[RY2_REPORTLEN]; 
	BYTE OutBuffer[RY2_REPORTLEN]; 
	
	if (handle < 0 || handle > g_DogCount)
		return RY2ERR_NO_SUCH_DEVICE;

	memset(InBuffer, 0, RY2_REPORTLEN);
	*(DWORD *)(InBuffer + 3 ) = LOWORD(len);
	InBuffer[1] = 0;
	InBuffer[2] = 0x8B ;
	*(DWORD *)(InBuffer + 5 ) = g_RockeyInfo[handle].dwUID;
	
	//限定最大长度
	if (len > 64)
		len = 64;

	memcpy(InBuffer + 9, data, len);
	WaitForSingleObject(g_RockeyInfo[handle].hMutex, INFINITE);
	retcode = RockeyHidReadWrite(handle, InBuffer, OutBuffer);
	ReleaseMutex(g_RockeyInfo[handle].hMutex);
	if ( retcode != RY2ERR_SUCCESS)
	{
		result = retcode;
	}
	else
	{
		memcpy(data, OutBuffer + 9, len);
		result = 0;
	}
	return result;
}

int WINAPI RY2_Read(int handle, int block_index, char *buffer512)
{
	int MiniBlockIndex = 0; 
	HANDLE hMutex; 
	int retcode; 
	int result; 
	char *pUserOutBuf; 
	BYTE InBuffer[RY2_REPORTLEN]; 
	BYTE OutBuffer[RY2_REPORTLEN]; 
	
	if (handle < 0 || handle > g_DogCount)
		return RY2ERR_NO_SUCH_DEVICE;

	if ( block_index < 0 || block_index > 5 )
		return RY2ERR_WRONG_INDEX;
	
	HANDLE hDevice = g_RockeyInfo[handle].hDevice;
	if ( hDevice == NULL || hDevice == INVALID_HANDLE_VALUE )
		return RY2ERR_NOT_OPENED_DEVICE;
	
	memset(InBuffer, 0, RY2_REPORTLEN);

	InBuffer[1] = 0;
	InBuffer[2] = 0x81;
	InBuffer[3] = block_index;
	*(DWORD *)(InBuffer + 5) = g_RockeyInfo[handle].dwUID;
	pUserOutBuf = buffer512;
	while ( MiniBlockIndex < 8 )
	{
		hMutex = g_RockeyInfo[handle].hMutex;
		InBuffer[4] = MiniBlockIndex;
		WaitForSingleObject(hMutex, INFINITE);
		retcode = RockeyHidReadWrite(handle, InBuffer, OutBuffer);
		ReleaseMutex(g_RockeyInfo[handle].hMutex);
		if ( retcode != RY2ERR_SUCCESS )
			break;
		++MiniBlockIndex;
		memcpy(pUserOutBuf, OutBuffer + 9, 64);
		pUserOutBuf += 64;
	}
	result = retcode;
	
	return result;
}

int WINAPI RY2_Write(int handle, int block_index, char *buffer512)
{
	HANDLE hDevice; 
	int MiniBlockIndex = 0; 
	BYTE *pUserInBuffer; 
	int retcode; 
	int result; 
	BYTE InBuffer[RY2_REPORTLEN]; 
	BYTE OutBuffer[RY2_REPORTLEN];
	
	if (handle < 0 || handle > g_DogCount)
		return RY2ERR_NO_SUCH_DEVICE;

	if ( block_index < 0 || block_index > 5 )
		return RY2ERR_WRONG_INDEX;
	
	hDevice = g_RockeyInfo[handle].hDevice;
	if ( hDevice == NULL || hDevice == INVALID_HANDLE_VALUE )
		return RY2ERR_NOT_OPENED_DEVICE;
	
	
	
	memset(InBuffer, 0, RY2_REPORTLEN);
	InBuffer[1] = 0;
	InBuffer[2] = 0x82;
	InBuffer[3] = block_index;
	InBuffer[4] = 0;
	*(DWORD *)(InBuffer + 5) = g_RockeyInfo[handle].dwUID;
	WaitForSingleObject(g_RockeyInfo[handle].hMutex, INFINITE);

	pUserInBuffer = (BYTE *)buffer512;
	do
	{
		InBuffer[4] = MiniBlockIndex;
		memcpy(InBuffer + 9, pUserInBuffer, 64);
		retcode = RockeyHidReadWrite(handle, InBuffer, OutBuffer);
		if ( retcode != RY2ERR_SUCCESS )
			break;
		++MiniBlockIndex;
		pUserInBuffer += 64;
	}
	while ( MiniBlockIndex < 8 );

	ReleaseMutex(g_RockeyInfo[handle].hMutex);
	result = retcode;
	
	return result;
}


int WINAPI RY2_GetVersion(int handle)
{
	int result;
	
	HANDLE hDevice = g_RockeyInfo[handle].hDevice;
	if ( hDevice != NULL && hDevice != INVALID_HANDLE_VALUE )
		result = g_RockeyInfo[handle].uVersion;
	else
		result = RY2ERR_NOT_OPENED_DEVICE;
	return result;
}

BOOL OpenDevice(GUID Guid, int maxInfNum)
{
	BOOL bRet = FALSE;
	HDEVINFO hwDevInfo = SetupDiGetClassDevs(&Guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (INVALID_HANDLE_VALUE == hwDevInfo)
		return false;
	for (unsigned int infIndex = 0; infIndex < maxInfNum; ++infIndex)
	{
		//Enum device interfaces
		SP_INTERFACE_DEVICE_DATA devData;

		//必须设置devData的cbSize，否则SetupDiEnumDeviceInterfaces调用会失败，GetLastError会返回1784
		devData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
		if (SetupDiEnumDeviceInterfaces(hwDevInfo,
			NULL,
			&Guid,
			infIndex,
			&devData))
		{
			DWORD symLinkLen = 0;
			SetupDiGetDeviceInterfaceDetail(hwDevInfo, &devData, NULL, 0, &symLinkLen, NULL);
			PSP_INTERFACE_DEVICE_DETAIL_DATA pDevDetailData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(new char[symLinkLen]);

			//必须设置devData的cbSize，否则SetupDiGetDeviceInterfaceDetail调用会失败，GetLastError会返回1784
			pDevDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
			if (!SetupDiGetDeviceInterfaceDetail(hwDevInfo,
				&devData,
				pDevDetailData,
				symLinkLen,
				&symLinkLen,
				NULL))
			{
				printf("SetupDiGetDeviceInterfaceDetail() FAILD!");
				delete[] pDevDetailData;
				pDevDetailData = NULL;
			}
			else
			{
				HANDLE deviceHandle = CreateFile(pDevDetailData->DevicePath,
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL);
				if (INVALID_HANDLE_VALUE != deviceHandle)
				{
					printf("CreateFile() OK!");
					CloseHandle(deviceHandle);
					bRet = TRUE;
				}
			}
			delete[] pDevDetailData;
			pDevDetailData = NULL;
		}
		else
		{
			printf("SetupDiEnumDeviceInterfaces() FAILD!");
			//SetupDiDestroyDeviceInfoList(m_hwDevInfo);
			DWORD ErrorCode = GetLastError();
		}

	}
	return bRet;
}