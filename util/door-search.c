/* turns to find a secret door using the new search code
 *
 *   make door-search && ./door-search | r-fu acumper
 *
 * https://github.com/thrig/r-fu */

#include "rogue.h"

#define SEEDS  1024
#define TRIALS 1024

int search(void);
int new_search(void);

int main(int argc, char *argv[])
{
    for (int s = 0; s < SEEDS; s++) {
        seed = (int) arc4random();
        for (int t = 0; t < TRIALS; t++) {
            printf("%d\n", new_search());
        }
    }
    exit(0);
}

int search(void)
{
    int turns = 1;
    while (1) {
        if (rnd(100) < 20)
            break;
        turns++;
    }
    return turns;
}

int new_search(void)
{
    int turns = 0;
    while (1) {
        if (rnd(100) < 10 + 6 * turns++)
            break;
    }
    return turns;
}
