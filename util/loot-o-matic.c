/* loot-o-matic - show the loot generated for a level (on average). this
 * does not include monster drops as those are fairly rare even with the
 * increased carry odds of this version
 *
 *   make loot-o-matic
 *   ./loot-o-matic 1 | perl -anE 'say "@F[1..$#F]"' | tally | $PAGER
 *
 * NOTE this does not account for the forced generation of food via the
 * no_food counter, which varies depending on what previous levels
 * generated and whether the hero fell through a trapdoor. food probably
 * should be on its own schedule independent of the other items: deep
 * levels can generate no items, and this can happen over multiple
 * levels if the RNG is in a bad mood */

#include <err.h>
#include <stdlib.h>

#include "../rogue.h"

void gen_things(int lv);
void loot(int lv, int s);

int main(int argc, char *argv[])
{
    int lv;

    if (argc != 2)
        errx(1, "need level number to generate");
    if (sscanf(argv[1], "%2d", &lv) != 1)
        errx(1, "could not parse level number");
    if (lv < 1 || lv > 30)
        errx(1, "level number out of range");

    init_things();              /* Set up probabilities of things */
    init_names();               /* Set up names of scrolls */
    init_colors();              /* Set up colors of potions */
    init_stones();              /* Set up stone settings of rings */
    init_materials();           /* Set up materials of wands */

    // the RN in rogue is limited to a 14-bit range
    for (int i = 0; i < 16834; i++)
        loot(lv, i);

    exit(EXIT_SUCCESS);
}

// adapted from put_things in newlevel.c
inline void gen_things(int lv)
{
    struct linked_list *item;
    int objodds;

    free_list(lvl_obj);
    level = lv;
    objodds = max(45, 105 - level * 10);
    no_food = 0;

    for (int i = 0; i < MAXOBJ; i++) {
        if (rnd(100) < objodds) {
            item = new_thing();
            attach(lvl_obj, item);
        }
    }
}

inline void loot(int lv, int s)
{
    seed = s;
    gen_things(lv);
    for (struct linked_list * item = lvl_obj; item != NULL; item = next(item)) {
        struct object *obj = ldata(item);
        switch (obj->o_type) {
        case SCROLL:
            printf("%d\tscroll\t%s\n", s, s_magic[obj->o_which].mi_name);
            break;
        case POTION:
            printf("%d\tpotion\t%s\n", s, p_magic[obj->o_which].mi_name);
            break;
        case FOOD:
            printf("%d\tfood\t%s\n", s,
                   obj->o_which == MANGO ? "fruit" : "ration");
            break;
        case WEAPON:
            if (obj->o_group == 0)
                // NOTE the dplus never actually varies, only hplus
                // is affected by a curse
                printf("%d\tweapon\t%s\t%d\t%d\n", s, w_names[obj->o_which],
                       obj->o_hplus, obj->o_dplus);
            else
                // plusses are fixed on ammo in my version so only the count
                printf("%d\tthrown\t%s\t%d\n", s, w_names[obj->o_which],
                       obj->o_count);
            break;
        case ARMOR:
            printf("%d\tarmor\t%s\t%d\n", s, a_names[obj->o_which],
                   a_class[obj->o_which] - obj->o_ac);
            break;
        case STICK:
            printf("%d\tstick\t%s\t%d\n", s, ws_magic[obj->o_which].mi_name,
                   obj->o_charges);
            break;
        case RING:
            // no-plus rings show up as "AC 11"
            printf("%d\tring\t%s\t%d\n", s, r_magic[obj->o_which].mi_name, obj->o_ac);
            break;
/* not implemented
        case AMULET:
            puts("%d\tamulet");
            break;
*/
        default:
            abort();
        }
    }
}
