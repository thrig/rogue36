/*
 * all sorts of miscellaneous routines
 *
 * @(#)misc.c	3.13 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>

#include "rogue.h"

char secretdoor(int y, int x);

/*
 * tr_name:
 *	print the name of a trap
 */

char *tr_name(char ch)
{
    char *s;

    switch (ch) {
    case TRAPDOOR:
        s = terse ? "A trapdoor." : "You found a trapdoor.";
        break;
    case BEARTRAP:
        s = terse ? "A beartrap." : "You found a beartrap.";
        break;
    case SLEEPTRAP:
        s = terse ? "A sleeping gas trap." : "You found a sleeping gas trap.";
        break;
    case ARROWTRAP:
        s = terse ? "An arrow trap." : "You found an arrow trap.";
        break;
    case TELTRAP:
        s = terse ? "A teleport trap." : "You found a teleport trap.";
        break;
    case DARTTRAP:
        s = terse ? "A dart trap." : "You found a poison dart trap.";
        break;
    default:
        s = terse ? "A no trap??" : "You found no such trap??";
    }
    return s;
}

/*
 * Look:
 *	A quick glance all around the player
 */

void look(bool wakeup)
{
    int x, y;
    char ch;
    int oldx, oldy;
    bool inpass;
    int passcount = 0;
    struct room *rp;
    int ey, ex;

    getyx(cw, oldy, oldx);
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) && off(player, ISBLIND)) {
        for (x = oldpos.x - 1; x <= oldpos.x + 1; x++)
            for (y = oldpos.y - 1; y <= oldpos.y + 1; y++)
                if ((y != hero.y || x != hero.x) && show(y, x) == FLOOR)
                    mvwaddch(cw, y, x, ' ');
    }
    inpass = ((rp = roomin(&hero)) == NULL);
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++) {
        if (x >= 0 && x < ROCOLS) {
            for (y = hero.y - 1; y <= ey; y++) {
                if (y <= 0 || y >= ROLINES - 1)
                    continue;
                if (isupper(mvwinch(mw, y, x))) {
                    struct linked_list *it;
                    struct thing *tp;

                    if (wakeup)
                        it = wake_monster(y, x);
                    else
                        it = find_mons(y, x);
                    tp = (struct thing *) ldata(it);
                    if ((tp->t_oldch = (char) mvinch(y, x)) == TRAP)
                        tp->t_oldch =
                            (trap_at(y, x)->tr_flags & ISFOUND) ? TRAP : FLOOR;
                    if (tp->t_oldch == FLOOR && (rp->r_flags & ISDARK)
                        && off(player, ISBLIND))
                        tp->t_oldch = ' ';
                }
                /*
                 * Secret doors show as walls
                 */
                if ((ch = show(y, x)) == SECRETDOOR)
                    ch = secretdoor(y, x);
                /*
                 * Don't show room walls if they are in a passage
                 */
                if (off(player, ISBLIND)) {
                    if ((y == hero.y && x == hero.x)
                        || (inpass && (ch == '-' || ch == '|')))
                        continue;
                } else if (y != hero.y || x != hero.x) {
                    continue;
                }
                wmove(cw, y, x);
                waddch(cw, ch);
                if (door_stop && !firstmove && running) {
                    switch (runch) {
                    case 'h':
                        if (x == ex)
                            continue;
                        break;
                    case 'j':
                        if (y == hero.y - 1)
                            continue;
                        break;
                    case 'k':
                        if (y == ey)
                            continue;
                        break;
                    case 'l':
                        if (x == hero.x - 1)
                            continue;
                        break;
                    case 'y':
                        if ((x + y) - (hero.x + hero.y) >= 1)
                            continue;
                        break;
                    case 'u':
                        if ((y - x) - (hero.y - hero.x) >= 1)
                            continue;
                        break;
                    case 'n':
                        if ((x + y) - (hero.x + hero.y) <= -1)
                            continue;
                        break;
                    case 'b':
                        if ((y - x) - (hero.y - hero.x) <= -1)
                            continue;
                    }
                    switch (ch) {
                    case DOOR:
                        if (x == hero.x || y == hero.y)
                            running = FALSE;
                        break;
                    case PASSAGE:
                        if (x == hero.x || y == hero.y)
                            passcount++;
                        break;
                    case FLOOR:
                    case '|':
                    case '-':
                    case ' ':
                        break;
                    default:
                        running = FALSE;
                        break;
                    }
                }
            }
        }
    }
    if (door_stop && !firstmove && passcount > 1)
        running = FALSE;
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    wmove(cw, oldy, oldx);
    oldpos = hero;
    oldrp = rp;
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */

char secretdoor(int y, int x)
{
    int i;
    struct room *rp;
    coord *cpp;
    static coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++) {
        if (inroom(rp, cpp)) {
            if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
                return '-';
            else
                return '|';
        }
    }
    return 'p';
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct linked_list *find_obj(int y, int x)
{
    struct linked_list *obj;
    struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj)) {
        op = (struct object *) ldata(obj);
        if (op->o_pos.y == y && op->o_pos.x == x)
            return obj;
    }
    sprintf(prbuf, "Non-object %d,%d", y, x);
    debug(prbuf);
    return NULL;
}

/*
 * eat:
 *	They want to eat something, so let them try
 */

void eat(void)
{
    struct linked_list *item;
    struct object *obj;

    if ((item = get_item("eat", FOOD)) == NULL)
        return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != FOOD) {
        if (!terse)
            msg("Ugh, you would get ill if you ate that.");
        else
            msg("That's Inedible!");
        return;
    }
    inpack--;
    if (obj->o_which == MANGO) {
        msg("My, that was a yummy %s", fruit);
    } else if (rnd(100) > 70) {
        msg("Yuk, this food tastes awful");
        pstats.s_exp++;
        check_level();
    } else {
        msg("Yum, that tasted good");
    }
    if ((food_left += HUNGERTIME + rnd(400) - 200) > STOMACHSIZE)
        food_left = STOMACHSIZE;
    hungry_state = 0;
    if (obj == cur_weapon)
        cur_weapon = NULL;
    if (--obj->o_count < 1) {
        detach(pack, item);
        discard(item);
    }
}

/*
 * Used to modify the players strength
 * it keeps track of the highest it has been, just in case
 */

void chg_str(short amt)
{
    short new;
    if (amt == 0)
        return;
    new = pstats.s_str + amt;
    if (new > MAXSTRENGTH)
        new = MAXSTRENGTH;
    else if (new < MINSTRENGTH)
        new = MINSTRENGTH;
    pstats.s_str = new;
    if (new > max_stats.s_str)
        max_stats.s_str = new;
}

/*
 * add_haste:
 *	add a haste to the player
 */

void add_haste(bool potion)
{
    if (on(player, ISHASTE)) {
        msg("You faint from exhaustion.");
        no_command += rnd(8);
        extinguish(nohaste);
    } else {
        player.t_flags |= ISHASTE;
        if (potion)
            fuse(nohaste, 0, rnd(4) + 4, AFTER);
    }
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */

void aggravate(void)
{
    struct linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
        runto(&((struct thing *) ldata(mi))->t_pos, &hero);
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char *vowelstr(char *str)
{
    switch (*str) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
        return "n";
    default:
        return "";
    }
}

/*
 * see if the object is one of the currently used items
 */
int is_current(struct object *obj)
{
    if (obj == NULL)
        return FALSE;
    if (obj == cur_armor || obj == cur_weapon || obj == cur_ring[LEFT]
        || obj == cur_ring[RIGHT]) {
        msg(terse ? "In use." : "That's already in use.");
        return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
int get_dir(void)
{
    char *prompt;
    bool gotit;

    if (!terse)
        msg(prompt = "Which direction? ");
    else
        prompt = "Direction: ";

    do {
        gotit = TRUE;
        switch (readchar(cw)) {
        case 'h':
        case 'H':
            delta.y = 0;
            delta.x = -1;
            break;
        case 'j':
        case 'J':
            delta.y = 1;
            delta.x = 0;
            break;
        case 'k':
        case 'K':
            delta.y = -1;
            delta.x = 0;
            break;
        case 'l':
        case 'L':
            delta.y = 0;
            delta.x = 1;
            break;
        case 'y':
        case 'Y':
            delta.y = -1;
            delta.x = -1;
            break;
        case 'u':
        case 'U':
            delta.y = -1;
            delta.x = 1;
            break;
        case 'b':
        case 'B':
            delta.y = 1;
            delta.x = -1;
            break;
        case 'n':
        case 'N':
            delta.y = 1;
            delta.x = 1;
            break;
        case ESCAPE:
            return FALSE;
            break;
        default:
            mpos = 0;
            msg(prompt);
            gotit = FALSE;
        }
    } while (!gotit);

    if (on(player, ISHUH) && rnd(100) > 80) {
        do {
            delta.y = rnd(3) - 1;
            delta.x = rnd(3) - 1;
        } while (delta.y == 0 && delta.x == 0);
    }

    mpos = 0;
    return TRUE;
}
