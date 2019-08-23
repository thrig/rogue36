/* shared stuff for utilities */

#include "rogue.h"

/* atoi is certainly less typing */
long argtol(const char *arg, const long min, const long max)
{
    char *ep;
    long val;
    errno = 0;
    val = strtol(arg, &ep, 0);
    if (arg[0] == '\0' || *ep != '\0')
        err(1, "strtol failed");
    if (errno == ERANGE && (val == LONG_MIN || val == LONG_MAX))
        err(1, "argument outside range of long");
    if (min != LONG_MIN && val < min)
        errx(1, "value is below minimum");
    if (max != LONG_MAX && val > max)
        errx(1, "value is above maximum");
    return val;
}

int rnd(int range)
{
    return range == 0 ? 0 : abs(RN) % range;
}

int roll(int number, int sides)
{
    int dtotal = 0;
    while (number--)
        dtotal += rnd(sides) + 1;
    return dtotal;
}
