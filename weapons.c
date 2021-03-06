/*
 * Functions for dealing with problems brought about by weapons
 *
 * @(#)weapons.c	3.17 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>
#include <string.h>

#include "rogue.h"

#define NONE 100

/*
 * NOTE ammo must not be at index 0 in these tables; see o_group
 * assignment code that allows ammo to stack in the inventory.
 */
char *w_names[MAXWEAPONS] = {
    "mace",
    "long sword",
    "short bow",
    "arrow",
    "dagger",
    "rock",
    "two handed sword",
    "sling",
    "dart",
    "crossbow",
    "crossbow bolt",
    "spear",
};

/* *INDENT-OFF* */
static struct init_weps {
    char *iw_dam;
    char *iw_hrl;
    short iw_hplus;
    short iw_dplus;
    char iw_launch;
    int iw_flags;
} init_dam[MAXWEAPONS] = {
/*  attack throw  h+  d+  launcher */
  { "2d4", "1d3",  0,  0, NONE,     0},                 /* Mace */
  { "1d10","1d1",  0,  0, NONE,     0},                 /* Long sword */
  { "1d1", "1d1",  0,  0, NONE,     0},                 /* Bow */
  { "1d1", "1d6",  0,  0, BOW,      ISMANY | ISMISL},   /* Arrow */
  { "1d6", "1d6",  1, -1, NONE,     ISMISL},            /* Dagger */
  { "1d2", "1d4",  2,  0, SLING,    ISMANY | ISMISL},   /* Rock */
  { "3d6", "1d1", -3,  1, NONE,     0},                 /* 2h sword */
  { "0d0", "0d0",  0,  0, NONE,     0},                 /* Sling */
  { "1d1", "1d3",  3,  0, NONE,     ISMANY | ISMISL},   /* Dart */
  { "1d1", "1d1",  0,  0, NONE,     0},                 /* Crossbow */
  { "1d1", "1d10", 0,  0, CROSSBOW, ISMANY | ISMISL},   /* Crossbow bolt */
  { "1d8", "1d8",  1,  0, NONE,     ISMISL}             /* Spear */
/*  attack throw  launcher */
};
/* *INDENT-ON* */

/*
 * missile:
 *      Throw a missile in a given direction
 */

void missile(int ydelta, int xdelta)
{
    struct object *obj;
    struct linked_list *item, *nitem;

    /*
     * Get which thing we are hurling
     */
    if ((item = get_item("throw", THROWABLE)) == NULL)
        return;
    obj = (struct object *) ldata(item);
    if (!dropcheck(obj) || is_current(obj))
        return;
    /*
     * Get rid of the thing.  If it is a non-multiple item object, or
     * if it is the last thing, just drop it.  Otherwise, create a new
     * item with a count of one.
     */
    if (obj->o_count < 2) {
        detach(pack, item);
        inpack--;
    } else {
        obj->o_count--;
        if (obj->o_group == 0)
            inpack--;
        nitem = (struct linked_list *) new_item(sizeof *obj);
        obj = (struct object *) ldata(nitem);
        *obj = *((struct object *) ldata(item));
        obj->o_count = 1;
        item = nitem;
    }
    do_motion(obj, ydelta, xdelta);
    /*
     * AHA! Here it has hit something.  Only mulch if ammo hits
     * a monster.
     */
    if (!(obj->o_type == WEAPON) || obj->o_group == 0
            || !hit_monster(unc(obj->o_pos), obj))
        fall(item, TRUE);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
void do_motion(struct object *obj, int ydelta, int xdelta)
{
    /*
     * Come fly with us ...
     */
    obj->o_pos = hero;
    while (1) {
        int ch;

        /*
         * Erase the old one
         */
        if (!ce(obj->o_pos, hero) && cansee(unc(obj->o_pos)) &&
            mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ')
            mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
                     show(obj->o_pos.y, obj->o_pos.x));
        nanosleep(&throwdelay, NULL);
        /*
         * Get the new position
         */
        obj->o_pos.y += ydelta;
        obj->o_pos.x += xdelta;
        if (step_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR) {
            /*
             * It hasn't hit anything yet, so display it
             * If it alright.
             */
            if (cansee(unc(obj->o_pos)) &&
                mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
                mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type);
                draw(cw);
            }
            continue;
        }
        break;
    }
}

/*
 * fall:
 *      Drop an item someplace around here; mulch if nowhere to put it.
 */

void fall(struct linked_list *item, bool pr)
{
    struct object *obj;
    struct room *rp;
    static coord fpos;

    obj = (struct object *) ldata(item);
    if (fallpos(&obj->o_pos, &fpos)) {
        mvaddch(fpos.y, fpos.x, obj->o_type);
        obj->o_pos = fpos;
        if ((rp = roomin(&hero)) != NULL && !(rp->r_flags & ISDARK)) {
            light(&hero);
            mvwaddch(cw, hero.y, hero.x, PLAYER);
        }
        attach(lvl_obj, item);
        return;
    }
    if (pr) {
        if (obj->o_type == WEAPON)      /* BUGFIX: Identification trick */
            msg("Your %s vanishes as it hits the ground.",
                w_names[obj->o_which]);
        else
            msg("%s vanishes as it hits the ground.", inv_name(obj, TRUE));
    }
    discard(item);
}

/*
 * init_weapon:
 *      Set up the initial goodies for a weapon
 */

void init_weapon(struct object *weap, char type)
{
    struct init_weps *iwp;

    iwp = &init_dam[(int) type];
    strcpy(weap->o_damage, iwp->iw_dam);
    strcpy(weap->o_hurldmg, iwp->iw_hrl);
    weap->o_launch = iwp->iw_launch;
    weap->o_flags = iwp->iw_flags;
    weap->o_hplus = iwp->iw_hplus;
    weap->o_dplus = iwp->iw_dplus;
    if (weap->o_flags & ISMANY) {
        weap->o_count = rnd(8) + 8;
        /* 
         * KLUGE this assumes type != 0 and that the type does not
         * conflict with any !weapon groups (only this function used the
         * old newgrp() macro). But want inventory ammo stacking...
         */
        weap->o_group = type;
    } else {
        weap->o_count = 1;
    }
}

/*
 * Does the missile hit the monster
 */

int hit_monster(int y, int x, struct object *obj)
{
    static coord mp;

    mp.y = y;
    mp.x = x;
    return fight(&mp, (char) winat(y, x), obj, TRUE);
}

/*
 * num:
 *      Figure out the plus number for armor/weapons
 */

char *num(int n1, int n2)
{
    static char numbuf[ROGUE_CHARBUF_MAX];

    if (n1 == 0 && n2 == 0)
        return "+0";
    if (n2 == 0)
        sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    else
        sprintf(numbuf, "%s%d,%s%d",
                n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2);
    return numbuf;
}

/*
 * wield:
 *      Pull out a certain weapon
 */

void wield(void)
{
    struct linked_list *item;
    struct object *obj, *oweapon;

    oweapon = cur_weapon;
    if (!dropcheck(cur_weapon)) {
        cur_weapon = oweapon;
        return;
    }
    cur_weapon = oweapon;
    if ((item = get_item("wield", WIELDABLE)) == NULL) {
      BADWIELD:
        after = FALSE;
        return;
    }

    obj = (struct object *) ldata(item);
    if (obj->o_type == ARMOR) {
        msg("You can't wield armor!");
        goto BADWIELD;
    }
    /*
     * This prevents ammo enchants so that ammo can (more easily) stack
     * in the inventory.
     */
    if (obj->o_type == WEAPON && obj->o_flags & ISMANY) {
        msg("You can't wield ammo!");
        goto BADWIELD;
    }
    if (is_current(obj))
        goto BADWIELD;

    if (terse)
        addmsg("W");
    else
        addmsg("You are now w");
    msg("ielding %s", inv_name(obj, TRUE));
    cur_weapon = obj;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
int fallpos(coord * pos, coord * newpos)
{
    int cnt = 0, ch;

    for (int y = pos->y - 1; y <= pos->y + 1; y++) {
        for (int x = pos->x - 1; x <= pos->x + 1; x++) {
            /*
             * check to make certain the spot is empty, if it is,
             * put the object there, set it in the level list
             * and re-draw the room if they can see it
             */
            if (y == hero.y && x == hero.x)
                continue;
            ch = winat(y, x);
            if ((ch == FLOOR || ch == PASSAGE) && rnd(++cnt) == 0) {
                newpos->y = y;
                newpos->x = x;
            }
        }
    }
    return cnt != 0;
}
