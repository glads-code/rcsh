/* Ce que 9pm prend de sa bibliothèque win (fonctions Windows en
   Unicode ou en ANSI selon la version) : avec tcc, le Unicode suffit. */
#include <windows.h>


int	win_useunicode = 1;

int
win_hasunicode(void)
{
	return 1;
}

char *
win_error(char *buf, int n)
{
	int e = GetLastError();
	char *p;

	if(!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, e,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, n, 0))
		wsprintfA(buf, "windows error %d", e);
	for(p = buf; *p; p++)
		if(*p == '\r' || *p == '\n')
			*p = ' ';
	return buf;
}
