#
# Makefile for rogue
# %W% (Berkeley) %G%
#
# Rogue: Exploring the Dungeons of Doom
# Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
# All rights reserved.
#
# See the LICENSE file for full copyright and licensing information.
#

PROGRAM=rogue

O=o

HDRS= 	rogue.h machdep.h

OBJS1 = vers.$(O) armor.$(O) chase.$(O) command.$(O) daemon.$(O) daemons.$(O) \
        fight.$(O) init.$(O) io.$(O) list.$(O) main.$(O) mdport.$(O) \
	misc.$(O) monsters.$(O) move.$(O) newlevel.$(O) options.$(O) 
OBJS2 =	pack.$(O) passages.$(O) potions.$(O) rings.$(O) rip.$(O) rooms.$(O) \
	save.$(O) scrolls.$(O) state.$(O) sticks.$(O) things.$(O) \
	weapons.$(O) wizard.$(O) xcrypt.$(O)
OBJS  = $(OBJS1) $(OBJS2)

CFILES= vers.c armor.c chase.c command.c daemon.c daemons.c fight.c \
	init.c io.c list.c main.c mdport.c misc.c monsters.c move.c newlevel.c \
	options.c pack.c passages.c potions.c rings.c rip.c rooms.c \
	save.c scrolls.c state.c sticks.c things.c weapons.c wizard.c xcrypt.c

CFLAGS += -Wall -pedantic -pipe
LIBS   ?= -lcurses
RM      = rm -f
LD      = $(CC)
LDOUT   = -o 

$(PROGRAM): $(HDRS) $(OBJS) Makefile
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) $(LDOUT) $@

clean:
	$(RM) $(OBJS) $(PROGRAM)

.PHONY: clean
