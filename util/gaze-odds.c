/* model E paralyze attacks: what are the chances of a softlock?
 *
 * common AC values are be 6 (starting armor), 9 (leather on D9 to avoid
 * rust monsters) 10 (no armour on D9) or possibly worst case 11 with -3
 * leather armour (or even worse with bad rings of protection)
 *
 * the limit is similar to the "off to infinity" test of fractals, here
 * instead to test whether the player has seen no moves in the given
 * limit. try values in the 100..1000 range; larger values especially
 * with more SEEDS or TRIALS can get very expensive */

#include "rogue.h"

#define SEEDS 1024
#define TRIALS 1024

int gaze(int player_ac);
int softlock(int player_ac, int limit);

int main(int argc, char *argv[])
{
    int player_ac, limit;
    if (argc != 3) {
        fprintf(stderr, "Usage: gaze-odds player-ac limit\n");
        exit(1);
    }
    player_ac = (int) argtol(argv[1], -20, 20);
    limit = (int) argtol(argv[2], 1, 100000);
    for (int s = 0; s < SEEDS; s++) {
        int locks = 0;
        seed = (int) arc4random();
        for (int t = 0; t < TRIALS; t++) {
            if (softlock(player_ac, limit)) locks++;
        }
        printf("%f\n", locks / (double) TRIALS * 100);
    }
    exit(0);
}

int gaze(int player_ac)
{
    int res = rnd(20) + 1;
    int need = 20 - player_ac;
    return (res >= need);
}

int softlock(int player_ac, int limit)
{
    int no_command = 0;
    int i;
    for (i = 0; i < limit; i++) {
        if (gaze(player_ac)) no_command += rnd(2) + 2;
        /* TWEAK - what happens if one or more rnd calls are made here? */
        //rnd(100);
        if (no_command) --no_command;
        if (no_command == 0) break;
    }
    return (i == limit) ? 1 : 0;
}
