/* Ce dont 9pm a besoin de winnetwk.h (absent des en-têtes de tcc) :
   parcours des partages réseau, fonctions de mpr.dll. */
#ifndef RCSH_WINNETWK_H
#define RCSH_WINNETWK_H
#define RESOURCE_GLOBALNET		2
#define RESOURCETYPE_DISK		1
#define RESOURCEDISPLAYTYPE_GENERIC	0
#define RESOURCEUSAGE_CONTAINER		2
#define UNIVERSAL_NAME_INFO_LEVEL	1
typedef struct {
	DWORD	dwScope, dwType, dwDisplayType, dwUsage;
	LPWSTR	lpLocalName, lpRemoteName, lpComment, lpProvider;
} NETRESOURCEW, NETRESOURCE, *LPNETRESOURCEW;
typedef struct { LPWSTR lpUniversalName; } UNIVERSAL_NAME_INFOW, UNIVERSAL_NAME_INFO;
DWORD WINAPI WNetOpenEnumW(DWORD, DWORD, DWORD, LPNETRESOURCEW, LPHANDLE);
DWORD WINAPI WNetEnumResourceW(HANDLE, LPDWORD, LPVOID, LPDWORD);
DWORD WINAPI WNetCloseEnum(HANDLE);
DWORD WINAPI WNetGetUniversalNameW(LPCWSTR, DWORD, LPVOID, LPDWORD);
#define WNetOpenEnum		WNetOpenEnumW
#define WNetEnumResource	WNetEnumResourceW
#define WNetGetUniversalName	WNetGetUniversalNameW
#endif
