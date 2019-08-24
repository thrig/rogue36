/*
 * Some version numbers that are mostly for score and save file use in
 * rip.c and save.c.
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

char version[] = "@(#)vers.c    3.6 (Berkeley) 4/21/81";
int version_num = 36;

/*
 * For the "encryption" of the save and score files.
 */
char encstr[] =
    "\354\251\243\332A\201|\301\321p\210\251\327\"\257\365t\341%3\271^`~\203z{\341};\f\341\231\222e\234\351]\321";

/*
 * Increment this when the save or score file formats change.
 */
int revision_num = 4;
