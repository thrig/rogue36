CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

all: monster-odds scroll-o-matic

monster-odds: monster-odds.c rogue.o rogue.h
	$(CC) $(LDFLAGS) rogue.o monster-odds.c -o monster-odds

rogue.o: rogue.c rogue.h
	$(CC) $(CFLAGS) -c rogue.c -o rogue.o

scroll-o-matic: scroll-o-matic.c rogue.o rogue.h
	$(CC) $(LDFLAGS) rogue.o scroll-o-matic.c -o scroll-o-matic

clean:
	rm -f monster-odds rogue.o scroll-o-matic

.PHONY: all clean
