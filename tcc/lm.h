/* Ce dont 9pm a besoin de lm.h (absent des en-têtes de tcc). Les
   fonctions sont chargées à l'exécution depuis netapi32 (sec.c). */
#ifndef RCSH_LM_H
#define RCSH_LM_H
typedef DWORD NET_API_STATUS;
#define NET_API_FUNCTION __stdcall
#define NERR_Success 0
#define LG_INCLUDE_INDIRECT 1
#define MAX_PREFERRED_LENGTH ((DWORD)-1)
typedef struct { LPWSTR lgrui0_name; } LOCALGROUP_USERS_INFO_0;
typedef struct { LPWSTR grui0_name; } GROUP_USERS_INFO_0;
#endif
