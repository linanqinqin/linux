#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <link.h>      // for AT_SYSINFO_EHDR
#include <sys/auxv.h>
extern void __vdso_lame_entry(void);

int main(void) {
    // Method A: via dlsym
    void *h = dlopen("linux-vdso.so.1", RTLD_NOW);
    void *sym = dlsym(h, "__vdso_lame_entry");
    printf("via dlsym:    __vdso_lame_entry = %p\n", sym);
    sym = dlsym(h, "lame_handle_array");
    printf("via dlsym:    lame_handle_array = %p\n", sym);

    printf("via direct print: __vdso_lame_entry = %p\n", (void *)__vdso_lame_entry);

    // Method B: via getauxval + symbol offset
    Elf64_Ehdr *vdso_base = (Elf64_Ehdr *)getauxval(AT_SYSINFO_EHDR);
    printf("vDSO base via auxv: %p\n", vdso_base);

    return 0;
}
