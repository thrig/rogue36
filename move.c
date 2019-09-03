/*
 * Hero movement commands
 *
 * @(#)move.c	3.26 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>

#include "rogue.h"

char be_trapped(coord * tc);

/*
 * Used to hold the new hero position
 */

static coord nh;

/*
 * do_run:
 *	Start the hero running
 */

void do_run(char ch)
{
    running = TRUE;
    after = FALSE;
    runch = ch;
}

/*
 * do_move:
 *	Check to see that a move is legal.  If it is handle the
 * consequences (fighting, picking up, etc.)
 */

void do_move(int dy, int dx)
{
    char ch;

    firstmove = FALSE;
    if (no_move) {
        no_move--;
        msg("You are still stuck in the bear trap");
        return;
    }
    /*
     * Do a confused move (maybe)
     */
    if (rnd(100) < 80 && on(player, ISHUH)) {
        nh = *rndmove(&player);
    } else {
        nh.y = hero.y + dy;
        nh.x = hero.x + dx;
    }
    search_repeat = 0;

    /*
     * Check if they tried to move off the screen or make an illegal
     * diagonal move, and stop them if they did.
     */
    if (nh.x < 0 || nh.x > ROCOLS - 1 || nh.y < 0 || nh.y > ROLINES - 1
        || !diag_ok(&hero, &nh)) {
        after = FALSE;
        running = FALSE;
        return;
    }
    if (running && ce(hero, nh))
        after = running = FALSE;
    ch = (char) winat(nh.y, nh.x);
    if (on(player, ISHELD) && ch != 'F') {
        msg("You are being held");
        return;
    }
    switch (ch) {
    case ' ':
    case '|':
    case '-':
    case SECRETDOOR:
        after = running = FALSE;
        return;
    case TRAP:
        ch = be_trapped(&nh);
        if (ch == TRAPDOOR || ch == TELTRAP)
            return;
        goto MOVE_STUFF;
    case GOLD:
    case POTION:
    case SCROLL:
    case FOOD:
    case WEAPON:
    case ARMOR:
    case RING:
    case AMULET:
    case STICK:
        running = FALSE;
        take = ch;
    default:
      MOVE_STUFF:
        if (ch == PASSAGE && winat(hero.y, hero.x) == DOOR) {
            light(&hero);
        } else if (ch == DOOR) {
            running = FALSE;
            if (winat(hero.y, hero.x) == PASSAGE)
                light(&nh);
        } else if (ch == STAIRS) {
            running = FALSE;
        } else if (isupper(ch)) {
            running = FALSE;
            fight(&nh, ch, cur_weapon, FALSE);
            return;
        }
        ch = (char) winat(hero.y, hero.x);
        wmove(cw, unc(hero));
        waddch(cw, ch);
        hero = nh;
        wmove(cw, unc(hero));
        waddch(cw, PLAYER);
    }
}

/*
 * Called to illuminate a room.
 * If it is dark, remove anything that might move.
 */

void light(coord * cp)
{
    struct room *rp;
    int j, k;
    char ch, rch;
    struct linked_list *item;

    if ((rp = roomin(cp)) != NULL && !on(player, ISBLIND)) {
        for (j = 0; j < rp->r_max.y; j++) {
            for (k = 0; k < rp->r_max.x; k++) {
                ch = show(rp->r_pos.y + j, rp->r_pos.x + k);
                wmove(cw, rp->r_pos.y + j, rp->r_pos.x + k);
                /*
                 * Figure out how to display a secret door
                 */
                if (ch == SECRETDOOR) {
                    if (j == 0 || j == rp->r_max.y - 1)
                        ch = '-';
                    else
                        ch = '|';
                }
                /*
                 * If the room is a dark room, we might want to remove
                 * monsters and the like from it (since they might
                 * move)
                 */
                if (isupper(ch)) {
                    item = wake_monster(rp->r_pos.y + j, rp->r_pos.x + k);
                    if (((struct thing *) ldata(item))->t_oldch == ' ')
                        if (!(rp->r_flags & ISDARK))
                            ((struct thing *) ldata(item))->t_oldch = (char)
                                mvwinch(stdscr, rp->r_pos.y + j,
                                        rp->r_pos.x + k);
                }
                if (rp->r_flags & ISDARK) {
                    rch = (char) mvwinch(cw, rp->r_pos.y + j, rp->r_pos.x + k);
                    switch (rch) {
                    case DOOR:
                    case STAIRS:
                    case TRAP:
                    case '|':
                    case '-':
                    case ' ':
                        ch = rch;
                        break;
                    case FLOOR:
                        ch = (on(player, ISBLIND) ? FLOOR : ' ');
                        break;
                    default:
                        ch = ' ';
                    }
                }
                mvwaddch(cw, rp->r_pos.y + j, rp->r_pos.x + k, ch);
            }
        }
    }
}

/*
 * show:
 *	returns what a certain thing will display as to the un-initiated
 */

char show(int y, int x)
{
    char ch = (char) winat(y, x);
    struct linked_list *it;
    struct thing *tp;

    if (ch == TRAP) {
        return (trap_at(y, x)->tr_flags & ISFOUND) ? TRAP : FLOOR;
    } else if (ch == 'M' || ch == 'I') {
        if ((it = find_mons(y, x)) == NULL)
            fatal("Can't find monster in show");
        tp = (struct thing *) ldata(it);
        if (ch == 'M') {
            ch = tp->t_disguise;
            /*
             * Hide invisible monsters
             */
        } else if (off(player, CANSEE)) {
            ch = (char) mvwinch(stdscr, y, x);
        }
    }
    return ch;
}

/*
 * be_trapped:
 *	Stepped on a trap: make them pay.
 */

char be_trapped(coord * tc)
{
    struct trap *tp;
    char ch;

    tp = trap_at(tc->y, tc->x);
    count = running = FALSE;
    mvwaddch(cw, tp->tr_pos.y, tp->tr_pos.x, TRAP);
    tp->tr_flags |= ISFOUND;
    switch (ch = tp->tr_type) {
    case TRAPDOOR:
        no_food++;              /* assume food was missed */
        level++;
        new_level();
        msg("You fell into a trap!");
        break;
    case BEARTRAP:
        no_move += BEARTIME;
        msg("You are caught in a bear trap");
        break;
    case SLEEPTRAP:
        no_command += SLEEPTIME;
        msg("A strange white mist envelops you and you fall asleep");
        break;
    case ARROWTRAP:
        if (swing(pstats.s_lvl - 1, pstats.s_arm, 1)) {
            msg("Oh no! An arrow shot you");
            if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
                msg("The arrow killed you.");
                death('a');
            }
        } else {
            struct linked_list *item;
            struct object *arrow;

            msg("An arrow shoots past you.");
            item = new_item(sizeof *arrow);
            arrow = (struct object *) ldata(item);
            arrow->o_type = WEAPON;
            arrow->o_which = ARROW;
            init_weapon(arrow, ARROW);
            arrow->o_count = 1;
            arrow->o_pos = hero;
            fall(item, FALSE);
        }
        break;
    case TELTRAP:
        msg("You teleport!");
        teleport();
        break;
    case DARTTRAP:
        if (swing(pstats.s_lvl + 1, pstats.s_arm, 1)) {
            msg("A small dart just hit you in the shoulder");
            if ((pstats.s_hpt -= roll(1, 4)) <= 0) {
                msg("The dart killed you.");
                death('d');
            }
            if (!ISWEARING(R_SUSTSTR))
                chg_str(-1);
        } else {
            msg("A small dart whizzes by your ear and vanishes.");
        }
    }
    flushinp();
    return ch;
}

/*
 * trap_at:
 *	find the trap at (y,x) on screen.
 */

struct trap *trap_at(int y, int x)
{
    struct trap *tp, *ep;

    ep = &traps[ntraps];
    for (tp = traps; tp < ep; tp++) {
        if (tp->tr_pos.y == y && tp->tr_pos.x == x)
            break;
    }
    if (tp == ep) {
        sprintf(prbuf, "Trap at %d,%d not in array", y, x);
        fatal(prbuf);
    }
    return tp;
}

/*
 * rndmove:
 *	move in a random direction if the monster/person is confused
 */

coord *rndmove(struct thing * who)
{
    int x, y;
    char ch;
    int ex, ey, nopen = 0;
    struct linked_list *item;
    struct object *obj;
    static coord ret;           /* what we will be returning */
    static coord dest;

    ret = who->t_pos;
    /*
     * Now go through the spaces surrounding the player and
     * set that place in the array to true if the space can be
     * moved into
     */
    ey = ret.y + 1;
    ex = ret.x + 1;
    for (y = who->t_pos.y - 1; y <= ey; y++) {
        if (y >= 0 && y < ROLINES) {
            for (x = who->t_pos.x - 1; x <= ex; x++) {
                if (x < 0 || x >= ROCOLS)
                    continue;
                ch = (char) winat(y, x);
                if (step_ok(ch)) {
                    dest.y = y;
                    dest.x = x;
                    if (!diag_ok(&who->t_pos, &dest))
                        continue;
                    if (ch == SCROLL) {
                        item = NULL;
                        for (item = lvl_obj; item != NULL; item = next(item)) {
                            obj = (struct object *) ldata(item);
                            if (y == obj->o_pos.y && x == obj->o_pos.x)
                                break;
                        }
                        if (item != NULL && obj->o_which == S_SCARE)
                            continue;
                    }
                    if (rnd(++nopen) == 0)
                        ret = dest;
                }
            }
        }
    }
    return &ret;
}
