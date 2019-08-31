/* experiment with different odds of trap generation, with limitations
 * of the "RNG" used by rogue... */

#include "rogue.h"

#define SEEDS  8192
#define TRIALS 128

const char *traps[] = { "door", "sleep", "arrow", "dart", "tele", "bear" };
unsigned long freq[] = { 0, 0, 0, 0, 0, 0 };

int what_trap(void);

int main(void)
{
    for (int s = 0; s < SEEDS; s++) {
        seed = (int) arc4random();
        for (int t = 0; t < TRIALS; t++)
            freq[what_trap()]++;
    }
    unsigned long sum = 0;
    for (int i = 0; i < 6; i++)
        sum += freq[i];
    for (int i = 0; i < 6; i++)
        printf("%.2f %s\n", freq[i] / (double) sum, traps[i]);
    exit(0);
}

int what_trap(void)
{
    //int n = arc4random_uniform(4) + arc4random_uniform(4);
    // 3d3 no bueno, makes the odds for the tails far too low?
    //int n = rnd(4) + rnd(4);
    int n = rnd(8);
    if (n == 5)
        n = 4;
    else if (n > 5)
        n = 5;
    return n;
}
