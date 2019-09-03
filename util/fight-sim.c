/* fight-sim - see how deadly monsters are
 *
 *   make fight-sim
 *   for m in B K J H S; do
 *      echo -n "$m "
 *      ./fight-sim $m | r-fu summary | grep '^[0-9]'
 *   done
 *
 * would review the starting lineup of monsters
 *
 * https://github.com/thrig/r-fu
 *
 * another interesting stat may be how many turns the fight takes
 */

#include <err.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../rogue.h"

#define RUNSEEDS  8192UL
#define RUNTRIALS 100UL

double trials(struct linked_list *mlist);
double thunderdome(char m);

int main(int argc, char *argv[])
{
    char ch;

    struct linked_list *item;
    struct object *obj;

    //initscr();

    // setup player
    pstats.s_lvl = 1;
    max_hp = pstats.s_hpt = 12;
    pstats.s_str = 12;
    strcpy(pstats.s_dmg, "1d4");
    pstats.s_arm = 10;
    max_stats = pstats;
    pack = NULL;

    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = DAGGER;
    // higher plus is better
    //obj->o_hplus = 1;
    //obj->o_dplus = 1;
    init_weapon(obj, DAGGER);
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;

    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = ARMOR;
    obj->o_which = LEATHER;
    // negative AC is better
    //obj->o_ac = a_class[LEATHER] - 1;
    obj->o_ac = a_class[LEATHER];
    obj->o_flags |= ISKNOW;
    cur_armor = obj;
    add_pack(item, TRUE);

    //endwin();

    ch = *argv[1];
    if (argc != 2 || !isupper(ch))
        errx(1, "need monster name A-Z");

    for (unsigned long i = 0; i < RUNSEEDS; i++)
        printf("%.4f\n", thunderdome(ch));

    exit(EXIT_SUCCESS);
}

inline double thunderdome(char m)
{
    coord somewhere;
    struct linked_list *mlist;
    struct thing *tp;

    double rate;

    // setup monster
    mlist = new_item(sizeof *tp);
    new_monster(mlist, m, &somewhere);

    /* TODO this should be a room with a fuller simulation of the game
     * loop due to the idiosyncrasies of the so-called RNG in rogue */
    rate = trials(mlist);

    discard(mlist);

    return rate;
}

inline double trials(struct linked_list *mlist)
{
    struct thing *mp;
    struct stats mnst, plst;
    unsigned long mded = 0, mhit = 0, pded = 0, phit = 0, turn = 0;

    mp = ldata(mlist);
    seed = (int) arc4random();

    // an energy system might be better than all this flag fiddling
    for (unsigned long t = 0; t < RUNTRIALS; t++) {
        int mswings = 1, pswings = 1;

        if (on(*mp, ISHASTE))
            mswings++;
        else if (on(*mp, ISSLOW))
            pswings++;

        memcpy(&mnst, &mp->t_stats, sizeof(mnst));
        memcpy(&plst, &pstats, sizeof(plst));

        while (turn++) {
            // player however will not always get the first hit in
            // TODO thrown weapons, staves
            for (int i = 0; i < pswings; i++) {
                if (roll_em(&plst, &mnst, cur_weapon, FALSE)) {
                    phit++;
                    if (mnst.s_hpt <= 0) {
                        mded++;
                        goto itsover;
                    }
                }
            }
            for (int i = 0; i < mswings; i++) {
                if (roll_em(&mnst, &plst, NULL, FALSE)) {
                    mhit++;
                    if (plst.s_hpt <= 0) {
                        pded++;
                        goto itsover;
                    }
                }
            }
        }
      itsover:;
    }
    return mded / (double) (mded + pded);
}
