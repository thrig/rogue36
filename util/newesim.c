/* new E: it does damage when the player is frozen. how much should it
 * do? monte carlo simulate how a (player-free) fight will go down */

#include "rogue.h"

#define SEEDS 1024
#define TRIALS 1024

int gaze(int player_ac);

int main(int argc, char *argv[])
{
    int player_ac;
    if (argc != 2) {
        fprintf(stderr, "Usage: gaze-odds player-ac\n");
        exit(1);
    }
    player_ac = (int) argtol(argv[1], -20, 20);
    for (int s = 0; s < SEEDS; s++) {
        seed = (int) arc4random();
        for (int t = 0; t < TRIALS; t++) {
            if (gaze(player_ac)) {
                int nocmd = rnd(4) + 5;
                int damage = 0;
                for (int n = 0; n < nocmd; n++) {
                    /* they have to hit while the player is frozen for
                     * the damage to happen */
                    if (gaze(player_ac))
                        damage += roll(1, 4);
                }
                printf("%d\n", damage);
            } else {
                printf("0\n");
            }
        }
    }
    exit(0);
}

int gaze(int player_ac)
{
    int res = rnd(20) + 1;
    int need = 20 - player_ac;
    return (res >= need);
}
