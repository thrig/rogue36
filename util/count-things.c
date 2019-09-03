/* how many things get generated on average? (from newlevel.c)
 *
 *   make count-things && ./count-things | r-fu summary
 *
 * https://github.com/thrig/r-fu */

#include "rogue.h"

#define MAXOBJ 9
#define TRIALS 8192

int make_things(int level);
int n_levels(int maxlevel);

int main(int argc, char *argv[])
{
    for (int s = 0; s < TRIALS; s++)
        printf("%d\n", n_levels(25));
    exit(0);
}

int n_levels(int maxlevel)
{
    int count = 0;
    for (int i = 0; i < maxlevel; i++) {
        /* the seed (probably) is somewhere else when generating
         * each new level */
        seed = arc4random();
        count += make_things(i);
    }
    return count;
}

#define max(a, b) ((a) > (b) ? (a) : (b))

int make_things(int level)
{
    int count = 0;
    int odds = 105 - level * 10;
    odds = max(45, odds);
    for (int i = 0; i < MAXOBJ; i++) {
        //if (rnd(100) < 35) count++;
        if (rnd(100) < odds) count++;
    }
    return count;
}

/* outcomes over 25 levels with the original 35% odds of item drop:

  0%  25%  50%  75% 100%
  58   74   79   84  104 */
