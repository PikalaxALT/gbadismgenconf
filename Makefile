CC := gcc
CFLAGS := -O3 -Wno-unused-result
LD := gcc

OBJS := main.o
HEADERS := elf.h global.h

.PHONY: all clean

all: gbadismgenconf
clean:
	$(RM) gbadismgenconf $(OBJS)

gbadismgenconf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<
