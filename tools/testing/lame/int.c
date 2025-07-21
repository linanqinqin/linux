// lame_int.c

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	printf("Before INT 0x1F\n");
	system("cat /proc/self/maps | grep vdso");
	asm volatile("int $0x1f");
	printf("After INT 0x1F\n");

	return 0;
}
