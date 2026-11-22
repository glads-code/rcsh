#include <u.h>
#include <9pm/libc.h>
#include <9pm/windows.h>
#include "9pm.h"

int
tas(int *p)
{
	return InterlockedExchange((LONG*)p, 1);
}
