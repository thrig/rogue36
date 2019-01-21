CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

all: gaze-odds monster-odds newesim scroll-o-matic

gaze-odds: gaze-odds.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o gaze-odds.c -o gaze-odds

monster-odds: monster-odds.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o monster-odds.c -o monster-odds

newesim: newesim.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o newesim.c -o newesim

rogue.o: rogue.c rogue.h
	$(CC) $(CFLAGS) -c rogue.c -o rogue.o

scroll-o-matic: scroll-o-matic.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o scroll-o-matic.c -o scroll-o-matic

clean:
	rm -f gaze-odds monster-odds newesim rogue.o scroll-o-matic

.PHONY: all clean
