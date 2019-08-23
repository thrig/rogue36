# Rogue: Exploring the Dungeons of Doom
# Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
# All rights reserved.
#
# See the LICENSE file for full copyright and licensing information.

ROGUE  ?= rogue
ROLIBS ?= -lcurses
CFLAGS += -std=c99 -Wall -pedantic -pipe
OBJS    = vers.o armor.o chase.o command.o daemon.o daemons.o fight.o init.o\
	  io.o list.o main.o mdport.o misc.o monsters.o move.o newlevel.o\
	  options.o pack.o passages.o potions.o rings.o rip.o rooms.o save.o\
	  scrolls.o state.o sticks.o things.o weapons.o wizard.o

$(ROGUE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(ROLIBS) -o $(ROGUE)

armor.o: armor.c rogue.h
chase.o: chase.c rogue.h
command.o: command.c rogue.h
daemon.o: daemon.c rogue.h
daemons.o: daemons.c rogue.h
fight.o: fight.c rogue.h
init.o: init.c rogue.h
io.o: io.c rogue.h
list.o: list.c rogue.h
main.o: main.c rogue.h
mdport.o: mdport.c
misc.o: misc.c rogue.h
monsters.o: monsters.c rogue.h
move.o: move.c rogue.h
newlevel.o: newlevel.c rogue.h
options.o: options.c rogue.h
pack.o: pack.c rogue.h
passages.o: passages.c rogue.h
potions.o: potions.c rogue.h
rings.o: rings.c rogue.h
rip.o: rip.c rogue.h
rooms.o: rooms.c rogue.h
save.o: save.c rogue.h
scrolls.o: scrolls.c rogue.h
state.o: state.c rogue.h
sticks.o: sticks.c rogue.h
things.o: things.c rogue.h
weapons.o: weapons.c rogue.h
wizard.o: wizard.c rogue.h

clean:
	@-rm -f highscores $(OBJS) $(PROGRAM) sav.*

.PHONY: clean
