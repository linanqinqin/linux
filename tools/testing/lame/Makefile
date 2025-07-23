CC = gcc
INCLUDE_PATH = -I../../../arch/x86/include
CFLAGS = -Wall -Wextra -O0 -g $(INCLUDE_PATH)
LDFLAGS = ../../../arch/x86/entry/vdso/vdso64.so

TARGETS = print_active write_active_print int print_vdso_lame reg add switch_r13

all: $(TARGETS)

print_active: print_active.c
	$(CC) $(CFLAGS) -o $@ $< 

write_active_print: write_active_print.c
	$(CC) $(CFLAGS) -o $@ $< 

int: int.c
	$(CC) $(CFLAGS) -o $@ $< 

print_vdso_lame: print_vdso_lame.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# cpuid: cpuid.c
# 	$(CC) $(CFLAGS) -o $@ $< 

reg: reg.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

add: add.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

switch_r13: switch_r13.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean 