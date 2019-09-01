CFLAGS=-std=c99 -g -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

ROBJ=../armor.o ../chase.o ../command.o ../daemon.o ../daemons.o ../fight.o ../io.o ../init.o ../list.o ../misc.o ../monsters.o ../move.o ../newlevel.o ../options.o ../pack.o ../passages.o ../potions.o ../rings.o ../rip.o ../rooms.o ../save.o ../scrolls.o ../state.o ../sticks.o ../things.o ../vers.o ../weapons.o ../wizard.o

count-things: count-things.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o count-things.c -o count-things

door-search: door-search.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o door-search.c -o door-search

fight-sim: fight-sim.c $(ROBJ) ../rogue.h
	$(CC) $(CFLAGS) $(ROBJ) fight-sim.c -lcurses -o fight-sim

gaze-odds: gaze-odds.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o gaze-odds.c -o gaze-odds

loot-o-matic: loot-o-matic.c $(ROBJ) ../rogue.h
	$(CC) $(CFLAGS) $(ROBJ) loot-o-matic.c -lcurses -o loot-o-matic

monster-odds: monster-odds.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o monster-odds.c -o monster-odds

newesim: newesim.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o newesim.c -o newesim

rogue.o: rogue.c rogue.h
	$(CC) $(CFLAGS) -c rogue.c -o rogue.o

scroll-o-matic: scroll-o-matic.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o scroll-o-matic.c -o scroll-o-matic

trap-odds: trap-odds.c rogue.o rogue.h
	$(CC) $(CFLAGS) rogue.o trap-odds.c -o trap-odds

clean:
	rm -f *.core *.o count-things door-search gaze-odds monster-odds newesim scroll-o-matic trap-odds

.PHONY: all clean
