// lame-add.c
#include <stdio.h>
extern int __vdso_lame_add(int x, int y);  // declare the vDSO function

int main() {
    int result = __vdso_lame_add(0, 0);
    printf("lame_add = %x\n", result);
    return 0;
}
