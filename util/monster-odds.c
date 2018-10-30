/* statistics on who shows up when (spoilers!) */

#include "rogue.h"

#define SEEDS 1024
#define TRIALS 8192

char lvl_mons[27]  = "KJBSHEAOZGLCRQNYTWFIXUMVDP";
char wand_mons[27] = "KJBSH AOZG CRQ Y W IXU V  ";

unsigned long counts[27];

int randmonster(int level, int wander);

int main(void)
{
    for (int level = 1; level <= 27; level++) {
        int fd;
        for (int i = 0; i < SEEDS; i++) {
            seed = (int) arc4random();
            for (int n = 0; n < TRIALS; n++)
                counts[randmonster(level, 0) - 'A']++;
        }
        printf("%-3d", level);
        for (int c = 0; c < 27; c++) {
            if (counts[c] > 0)
                printf("%c:%.2f ", 'A' + c,
                       counts[c] / (double) (SEEDS * TRIALS));
            counts[c] = 0;
        }
        putchar('\n');
    }
    exit(0);
}

inline int randmonster(int level, int wander)
{
    int d;
    char *mons;

    mons = wander ? wand_mons : lvl_mons;
    do {
        d = level + (rnd(10) - 5);
        if (d < 1)
            d = rnd(5) + 1;
        if (d > 26)
            d = rnd(5) + 22;
    } while (mons[--d] == ' ');
    return mons[d];
}
