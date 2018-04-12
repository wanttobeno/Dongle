#include "StdAfx.h"
#include "../Inc/Rockey2.h"

extern VOID RockeyInit();

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		RockeyInit();
	
    return TRUE;
}