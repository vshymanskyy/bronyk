#include "main.h"
#include <stdlib.h>

int main(void)
{
	init();

	setup();
    
	for (;;) {
		loop();
	}

	for(;;);
	return 0;
}

extern "C" void __cxa_pure_virtual()
{
	cli();
	for (;;);
}

__extension__ typedef int __guard __attribute__((mode (__DI__)));

void* operator new(size_t size)
{
	return malloc(size);
}

void operator delete(void* ptr)
{
	free(ptr);
}

void* operator new[](size_t size)
{
	return malloc(size);
}

void operator delete[](void* ptr)
{
	if (ptr) {
		free(ptr);
	}
}

namespace __cxxabiv1
{
	/* guard variables */

	/* The ABI requires a 64-bit type.  */
	__extension__ typedef int __guard __attribute__((mode(__DI__)));

	extern "C" int __cxa_guard_acquire (__guard *);
	extern "C" void __cxa_guard_release (__guard *);
	extern "C" void __cxa_guard_abort (__guard *);

	extern "C" int __cxa_guard_acquire (__guard *g)
	{
		return !*(char *)(g);
	}

	extern "C" void __cxa_guard_release (__guard *g)
	{
		*(char *)g = 1;
	}

	extern "C" void __cxa_guard_abort (__guard *)
	{

	}
}
