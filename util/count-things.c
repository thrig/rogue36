/* how many things get generated on average? (from newlevel.c)
 *
 *   make count-things && ./count-things | r-fu summary
 *
 * https://github.com/thrig/r-fu */

#include "rogue.h"

#define SEEDS  1024
#define TRIALS 1024

#define MAXOBJ 9

int make_things(int level);
int n_levels(int maxlevel);

int main(int argc, char *argv[])
{
    for (int s = 0; s < SEEDS; s++) {
        seed = (int) arc4random();
        for (int t = 0; t < TRIALS; t++)
            printf("%d\n", n_levels(1));
    }
    exit(0);
}

int n_levels(int maxlevel)
{
    int count = 0;
    for (int i = 0; i < maxlevel; i++)
        count += make_things(i);
    return count;
}

int make_things(int level)
{
    int count = 0;
    int odds = 100 - level * 10;
    if (odds < 30) odds = 30;
    for (int i = 0; i < MAXOBJ; i++)
        if (rnd(100) < 30) count++;
        //if (rnd(100) < odds) count++;
    return count;
}

/* outcomes over 25 levels with the original 35% odds of item drop:

	elements 1048576 mean 78.827 range 46.000 min 58.000 max 104.000 sd 7.192

	quantile (type 8)
	  0%  25%  50%  75% 100% 
	  58   74   79   84  104 */
