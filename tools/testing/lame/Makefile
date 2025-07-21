CC = gcc
INCLUDE_PATH = -I../../../arch/x86/include
CFLAGS = -Wall -Wextra -O0 -g $(INCLUDE_PATH)
LDFLAGS = ../../../arch/x86/entry/vdso/vdso64.so

TARGETS = print_active write_active_print

all: $(TARGETS)

print_active: print_active.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

write_active_print: write_active_print.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean 