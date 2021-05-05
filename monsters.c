/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	3.18 (Berkeley) 6/15/81
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

/*
 * List of monsters in rough order of vorpalness
 */
char lvl_mons[27] = "KJBSHEAOZGNCRQLYTWFIXUMVDP";
char wand_mons[27] = "KJBSH AOZG CRQ Y WLIXU V  ";

/*
 * create_monster
 *   Create a new monster
 */
void create_monster(char type)
{
    coord mp;
    int appear = 0;
    struct linked_list *titem;

    for (int y = hero.y - 1; y <= hero.y + 1; y++) {
        for (int x = hero.x - 1; x <= hero.x + 1; x++) {
            if (y == hero.y && x == hero.x)
                continue;
            if (step_ok((char) winat(y, x))) {
                if (rnd(++appear) == 0) {
                    mp.y = y;
                    mp.x = x;
                }
            }
        }
    }
    if (appear) {
        titem = new_item(sizeof(struct thing));
        msg("A monster appears!");
        new_monster(titem, type, &mp);
    } else {
        msg("You hear a faint cry of anguish in the distance.");
    }
}

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */

int randmonster(bool wander)
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

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */

void new_monster(struct linked_list *item, char type, coord * cp)
{
    int minhp, monsthp;
    struct thing *tp;
    struct monster *mp;

    attach(mlist, item);
    tp = (struct thing *) ldata(item);
    tp->t_type = type;
    tp->t_pos = *cp;
    tp->t_oldch = (char) mvwinch(cw, cp->y, cp->x);

    mvwaddch(mw, cp->y, cp->x, tp->t_type);

    mp = &monsters[tp->t_type - 'A'];
    memcpy(&tp->t_stats, &mp->m_stats, sizeof(struct stats));
    monsthp = roll(mp->m_stats.s_lvl, 8) + mp->m_stats.s_hpt;
    minhp = 1 + level / 2;
    if (minhp > 14) minhp = 14;
    tp->t_stats.s_hpt = max(minhp, monsthp);

    tp->t_flags = mp->m_flags;
    tp->t_turn = TRUE;
    tp->t_pack = NULL;

    if (ISWEARING(R_AGGR))
        runto(cp, &hero);

    if (type == 'M') {
        char mch = GOLD;

        if (tp->t_pack != NULL) {
            mch = ((struct object *) ldata(tp->t_pack))->o_type;
        } else {
            switch (rnd(level > 25 ? 9 : 8)) {
            case 0:
                mch = GOLD;
                break;
            case 1:
                mch = POTION;
                break;
            case 2:
                mch = SCROLL;
                break;
            case 3:
                mch = STAIRS;
                break;
            case 4:
                mch = WEAPON;
                break;
            case 5:
                mch = ARMOR;
                break;
            case 6:
                mch = RING;
                break;
            case 7:
                mch = STICK;
                break;
            case 8:
                mch = AMULET;
            }
        }
        tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */

void wanderer(void)
{
    int i, ch;
    struct room *rp, *hr = roomin(&hero);
    struct linked_list *item;
    struct thing *tp;
    coord cp;

    item = new_item(sizeof *tp);
    do {
        i = rnd_room();
        if ((rp = &rooms[i]) == hr)
            continue;
        rnd_pos(rp, &cp);
        if ((ch = mvwinch(stdscr, cp.y, cp.x)) == ERR) {
            if (wizard)
                wait_for(cw, '\n', 0);
            return;
        }
    } while (hr == rp || !step_ok(ch));

    new_monster(item, randmonster(TRUE), &cp);
    tp = (struct thing *) ldata(item);
    tp->t_flags |= ISRUN;
    tp->t_pos = cp;
    tp->t_dest = &hero;
    if (wizard)
        msg("Started a wandering %s", monsters[tp->t_type - 'A'].m_name);
}

/*
 * what to do when the hero steps next to a monster
 */
struct linked_list *wake_monster(int y, int x)
{
    struct thing *tp;
    struct linked_list *it;
    struct room *rp;
    char ch;

    if ((it = find_mons(y, x)) == NULL)
        fatal("Can't find monster in wake");
    tp = (struct thing *) ldata(it);
    ch = tp->t_type;
    /*
     * Every time they see a mean monster, it might start chasing them
     */
    if (rnd(100) > 33 && on(*tp, ISMEAN) && off(*tp, ISHELD)
        && !ISWEARING(R_STEALTH)) {
        tp->t_dest = &hero;
        tp->t_flags |= ISRUN;
    }
    if (ch == 'U' && off(player, ISBLIND)) {
        rp = roomin(&hero);
        if ((rp != NULL && !(rp->r_flags & ISDARK))
            || DISTANCE(y, x, hero.y, hero.x) < 3) {
            if (off(*tp, ISFOUND) && !save(VS_MAGIC)) {
                msg("The umber hulk's gaze has confused you.");
                if (on(player, ISHUH))
                    lengthen(unconfuse, roll(3, 6));
                else
                    fuse(unconfuse, 0, roll(3, 6), AFTER);
                player.t_flags |= ISHUH;
            }
            tp->t_flags |= ISFOUND;
        }
    }
    /*
     * Hide invisible monsters
     */
    if (on(*tp, ISINVIS) && off(player, CANSEE))
        ch = (char) mvwinch(stdscr, y, x);
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && off(*tp, ISRUN)) {
        rp = roomin(&hero);

        if (rp != NULL && rp->r_goldval) {
            tp->t_dest = &rp->r_gold;
            tp->t_flags |= ISRUN;
        }
    }

    return it;
}

void genocide(void)
{
    struct linked_list *ip;
    struct thing *mp;
    char c;
    int i;
    struct linked_list *nip;

    addmsg("Which monster");
    if (!terse)
        addmsg(" do you wish to wipe out");
    msg("? ");
    while (!isalpha(c = readchar(cw)))
        if (c == ESCAPE) {
            return;
        } else {
            mpos = 0;
            msg("Please specify a letter between 'A' and 'Z'");
        }
    if (islower(c))
        c = toupper(c);
    for (ip = mlist; ip; ip = nip) {
        mp = (struct thing *) ldata(ip);
        nip = next(ip);
        if (mp->t_type == c)
            remove_monster(&mp->t_pos, ip);
    }
    for (i = 0; i < 26; i++) {
        if (lvl_mons[i] == c) {
            lvl_mons[i] = ' ';
            wand_mons[i] = ' ';
            break;
        }
    }
}
