CC = gcc
INCLUDE_PATH = -I../../../arch/x86/include
CFLAGS = -Wall -Wextra -O0 -g $(INCLUDE_PATH)
VDSO_LAME_PATH ?=

# If VDSO_LAME_PATH is set, add it to LDFLAGS
ifeq ($(VDSO_LAME_PATH),)
LDFLAGS :=
else
LDFLAGS := -Wl,-rpath,$(dir $(VDSO_LAME_PATH)) -L$(dir $(VDSO_LAME_PATH)) -l:$(notdir $(VDSO_LAME_PATH))
endif

TARGETS = print_active write_active_print

all: $(TARGETS)

print_active: print_active.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

write_active_print: write_active_print.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean 