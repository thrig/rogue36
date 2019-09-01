/* loot-o-matic - show the loot generated for a level (on average). this
 * does not include monster drops as those are fairly rare even with the
 * increased carry odds of this version */

#include <err.h>
#include <stdlib.h>

#include "../rogue.h"

#define RUNSEEDS 1000UL

void gen_things(int lv);
void loot(int lv);

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

    for (unsigned long i = 0; i < RUNSEEDS; i++)
        loot(lv);

    exit(EXIT_SUCCESS);
}

// adapted from put_things in newlevel.c
inline void gen_things(int lv)
{
    struct linked_list *item;
    //struct object *cur;
    int objodds;

    free_list(lvl_obj);
    level = lv;
    objodds = max(45, 105 - level * 10);

    for (int i = 0; i < MAXOBJ; i++) {
        if (rnd(100) < objodds) {
            item = new_thing();
            attach(lvl_obj, item);
        }
    }
}

inline void loot(int lv)
{
    seed = (int) arc4random();
    gen_things(lv);
    for (struct linked_list * item = lvl_obj; item != NULL; item = next(item)) {
        struct object *obj = ldata(item);
        switch (obj->o_type) {
        case SCROLL:
            printf("scroll\t%s\n", s_magic[obj->o_which].mi_name);
            break;
        case POTION:
            printf("potion\t%s\n", p_magic[obj->o_which].mi_name);
            break;
        case FOOD:
            printf("food\t%s\n", obj->o_which == MANGO ? "fruit" : "ration");
            break;
        case WEAPON:
            if (obj->o_group == 0)
                printf("weapon\t%s\t%d\t%d\n", w_names[obj->o_which],
                       obj->o_hplus, obj->o_dplus);
            else
                // plusses are fixed on ammo in my version
                printf("thrown\t%s\t%d\n", w_names[obj->o_which], obj->o_count);
            break;
        case ARMOR:
            printf("armor\t%s\t%d\n", a_names[obj->o_which],
                   a_class[obj->o_which] - obj->o_ac);
            break;
        case AMULET:
            puts("amulet");
            break;
        case STICK:
            printf("stick\t%s\t%d\n", ws_magic[obj->o_which].mi_name,
                   obj->o_charges);
            break;
        case RING:
            // no-plus rings show up as "AC 11"
            printf("ring\t%s\t%d\n", r_magic[obj->o_which].mi_name, obj->o_ac);
            break;
        default:
            abort();
        }
    }
}
