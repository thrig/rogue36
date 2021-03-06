/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances). -DWIZARD versions MUST NOT be exposed
 * to untrusted users.
 *
 * @(#)wizard.c	3.8 (Berkeley) 6/3/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "rogue.h"

#ifdef WIZARD
/*
 * create_obj:
 *	Wizard command for getting anything they want
 */

void create_obj(void)
{
    struct linked_list *item;
    struct object *obj;
    char ch, bless;

    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    msg("Type of item \"!?:)],=/\": ");
    obj->o_type = readchar(cw);
    switch (obj->o_type) {
        case POTION:
        case SCROLL:
        case FOOD:
        case WEAPON:
        case ARMOR:
        case AMULET:
        case RING:
        case STICK:
            break;
        default:
            msg("Unknown type %c", obj->o_type);
            return;
    }
    mpos = 0;
    msg("Which %c do you want? (0-f)", obj->o_type);
    obj->o_which = (isdigit((ch = readchar(cw))) ? ch - '0' : ch - 'a' + 10);
    obj->o_group = 0;
    obj->o_count = 1;
    mpos = 0;
    if (obj->o_type == WEAPON || obj->o_type == ARMOR) {
        msg("Blessing? (+,-,n)");
        bless = readchar(cw);
        mpos = 0;
        if (obj->o_type == WEAPON) {
            init_weapon(obj, obj->o_which);
            if (bless == '-') {
                obj->o_hplus -= rnd(3) + 1;
                obj->o_flags |= ISCURSED;
            }
            if (bless == '+')
                obj->o_hplus += rnd(3) + 1;
        } else {
            obj->o_ac = a_class[obj->o_which];
            if (bless == '-') {
                obj->o_ac += rnd(3) + 1;
                obj->o_flags |= ISCURSED;
            }
            if (bless == '+')
                obj->o_ac -= rnd(3) + 1;
        }
    } else if (obj->o_type == RING)
        switch (obj->o_which) {
        case R_PROTECT:
        case R_ADDSTR:
        case R_ADDHIT:
        case R_ADDDAM:
            msg("Blessing? (+,-,n)");
            bless = readchar(cw);
            mpos = 0;
            if (bless == '-')
                obj->o_flags |= ISCURSED;
            obj->o_ac = (bless == '-' ? -1 : rnd(2) + 1);
    } else if (obj->o_type == STICK)
        fix_stick(obj);
    add_pack(item, FALSE);
}
#endif

/*
 * telport:
 *	Bamf the hero someplace else (used by various !wizard code)
 */

int teleport(void)
{
    int rm;
    coord c;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    do {
        rm = rnd_room();
        rnd_pos(&rooms[rm], &hero);
    } while (winat(hero.y, hero.x) != FLOOR);
    light(&c);
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
        player.t_flags &= ~ISHELD;
        fung_hit = 0;
        strcpy(monsters['F' - 'A'].m_stats.s_dmg, "000d0");
    }
    cmdcount = 0;
    running = FALSE;
    flushinp();
    return rm;
}

/*
 * whatis:
 *	What a certin object is
 */

void whatis(void)
{
    struct object *obj;
    struct linked_list *item;

    if ((item = get_item("identify", 0)) == NULL)
        return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type) {
    case SCROLL:
        s_know[obj->o_which] = TRUE;
        if (s_guess[obj->o_which]) {
            free(s_guess[obj->o_which]);
            s_guess[obj->o_which] = NULL;
        }
        break;
    case POTION:
        p_know[obj->o_which] = TRUE;
        if (p_guess[obj->o_which]) {
            free(p_guess[obj->o_which]);
            p_guess[obj->o_which] = NULL;
        }
        break;
    case STICK:
        ws_know[obj->o_which] = TRUE;
        obj->o_flags |= ISKNOW;
        if (ws_guess[obj->o_which]) {
            free(ws_guess[obj->o_which]);
            ws_guess[obj->o_which] = NULL;
        }
        break;
    case WEAPON:
    case ARMOR:
        obj->o_flags |= ISKNOW;
        break;
    case RING:
        r_know[obj->o_which] = TRUE;
        obj->o_flags |= ISKNOW;
        if (r_guess[obj->o_which]) {
            free(r_guess[obj->o_which]);
            r_guess[obj->o_which] = NULL;
        }
    }
    msg(inv_name(obj, FALSE));
}
