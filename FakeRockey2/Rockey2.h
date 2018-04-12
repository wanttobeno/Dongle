#if !defined(_ROCKEY_API_INCLUDED_)
#define _ROCKEY_API_INCLUDED_


int WINAPI RY2_Find();
int WINAPI RY2_GenUID(int handle,DWORD *uid, char *seed,int isProtect);
int WINAPI RY2_GetVersion(int handle);
void WINAPI RY2_Close(int handle);
int WINAPI RY2_Open(int mode,DWORD uid,DWORD *hid);
int WINAPI RY2_Read(int handle,int block_index,char *buffer512);
int WINAPI RY2_Write(int handle,int block_index,char *buffer512);
int WINAPI RY2_Transform(int handle, int len, BYTE *data);

#endif //_ROCKEY_API_INCLUDED_

