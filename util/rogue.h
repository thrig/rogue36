#ifndef _H_ROGUE_H_
#define _H_ROGUE_H_

/* shared stuff for utilities */

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int seed;                       /* Random number seed */

#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)

int rnd(int range);
long argtol(const char *arg, const long min, const long max);

#endif
