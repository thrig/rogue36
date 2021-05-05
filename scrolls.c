/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c	3.5 (Berkeley) 6/15/81
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

void read_scroll(void)
{
    struct object *obj;
    struct linked_list *item;
    struct room *rp;
    int i, j;
    char ch, nch;
    char buf[ROGUE_CHARBUF_MAX];

    item = get_item("read", SCROLL);
    if (item == NULL)
        return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != SCROLL) {
        if (!terse)
            msg("There is nothing on it to read");
        else
            msg("Nothing to read");
        return;
    }
    /*
     * Calculate the effect it has on the poor hero.
     */
    if (obj == cur_weapon)
        cur_weapon = NULL;
    switch (obj->o_which) {
    case S_CONFUSE:
        /*
         * Scroll of monster confusion.  Give them that power.
         */
        msg("Your hands begin to glow red");
        /* gets old after very few games so just ID it... */
        s_know[S_CONFUSE] = TRUE;
        player.t_flags |= CANHUH;
        break;
    case S_LIGHT:
        s_know[S_LIGHT] = TRUE;
        if ((rp = roomin(&hero)) == NULL) {
            msg("The corridor glows and then fades");
        } else {
            addmsg("The room is lit");
            if (!terse)
                addmsg(" by a shimmering blue light.");
            endmsg();
            rp->r_flags &= ~ISDARK;
            /*
             * Light the room and put the player back up
             */
            light(&hero);
            mvwaddch(cw, hero.y, hero.x, PLAYER);
        }
        break;
    case S_ARMOR:
        if (cur_armor != NULL) {
            msg("Your armor glows faintly for a moment");
            cur_armor->o_ac--;
            cur_armor->o_flags &= ~ISCURSED;
            s_know[S_ARMOR] = TRUE;
        }
        break;
    case S_HOLD:
        /*
         * Hold monster scroll.  Stop all monsters within two spaces
         * from chasing after the hero.
         */
        {
            int held = 0;
            struct linked_list *mon;

            for (int x = hero.x - 2; x <= hero.x + 2; x++) {
                for (int y = hero.y - 2; y <= hero.y + 2; y++) {
                    if (y > 0 && x > 0 && isupper(mvwinch(mw, y, x))) {
                        if ((mon = find_mons(y, x)) != NULL) {
                            struct thing *th;

                            th = (struct thing *) ldata(mon);
                            th->t_flags &= ~ISRUN;
                            th->t_flags |= ISHELD;
                            held++;
                        }
                    }
                }
            }
            if (held) {
                msg("%s freezes in place.",
                    held > 1 ? "Some monsters" : "A monster");
                s_know[S_HOLD] = TRUE;
            } else {
                msg("Nothing appears to happen.");
            }
        }
        break;
    case S_SLEEP:
        /*
         * Scroll which makes you fall asleep
         */
        s_know[S_SLEEP] = TRUE;
        msg("You fall asleep.");
        no_command += 4 + rnd(SLEEPTIME);
        break;
    case S_CREATE:
        create_monster(randmonster(FALSE));
        s_know[S_CREATE] = TRUE;
        break;
    case S_IDENT:
        if (s_know[S_IDENT] == FALSE)
            msg("This scroll is an identify scroll");
        s_know[S_IDENT] = TRUE;
        whatis();
        break;
    case S_MAP:
        msg("Oh, now this scroll has a map on it.");
        s_know[S_MAP] = TRUE;
        overwrite(stdscr, hw);
        /*
         * Take all the things we want to keep hidden out of the window
         */
        for (i = 0; i < ROLINES; i++) {
            for (j = 0; j < ROCOLS; j++) {
                switch (nch = ch = (char) mvwinch(hw, i, j)) {
                case SECRETDOOR:
                    nch = DOOR;
                    mvaddch(i, j, nch);
                case '-':
                case '|':
                case DOOR:
                case PASSAGE:
                case ' ':
                case STAIRS:
                    if (mvwinch(mw, i, j) != ' ') {
                        struct thing *it;

                        it = (struct thing *) ldata(find_mons(i, j));
                        if ((it != NULL) && (it->t_oldch == ' '))
                            it->t_oldch = nch;
                    }
                    break;
                default:
                    nch = ' ';
                }
                if (nch != ch)
                    waddch(hw, nch);
            }
        }
        /*
         * Copy in what they have discovered
         */
        overlay(cw, hw);
        /*
         * And set up for display
         */
        overwrite(hw, cw);
        break;
    case S_GFIND:
        /*
         * Potion of gold detection
         */
        {
            unsigned int gtotal = 0;

            wclear(hw);
            for (i = 0; i < MAXROOMS; i++) {
                gtotal += rooms[i].r_goldval;
                if (rooms[i].r_goldval != 0 &&
                    mvwinch(stdscr, rooms[i].r_gold.y, rooms[i].r_gold.x)
                    == GOLD)
                    mvwaddch(hw, rooms[i].r_gold.y, rooms[i].r_gold.x, GOLD);
            }
            if (gtotal) {
                s_know[S_GFIND] = TRUE;
                show_win(hw,
                         "You begin to feel greedy and you sense gold.--More--");
            } else {
                msg("You begin to feel a pull downward");
            }
        }
        break;
    case S_TELEP:
        /*
         * Scroll of teleportation:
         *  Some hold that this makes the player vanish and reappear
         *  somewhere else. Another--perhaps heretical--claim is that
         *  the "similarity of two different locations" may be increased
         *  to the point that "the source location becomes the
         *  destination location". Regardless, this is how it happens.
         */
        {
            int rm;
            struct room *cur_room;

            cur_room = roomin(&hero);
            rm = teleport();
            if (cur_room != &rooms[rm])
                s_know[S_TELEP] = TRUE;
            search_repeat = 0;
        }
        break;
    case S_ENCH:
        if (cur_weapon == NULL) {
            msg("Your hands glow blue for moment. Oops.");
        } else {
            cur_weapon->o_flags &= ~ISCURSED;
            if (rnd(100) > 50)
                cur_weapon->o_hplus++;
            else
                cur_weapon->o_dplus++;
            msg("Your %s glows blue for a moment.",
                w_names[cur_weapon->o_which]);
        }
        s_know[S_ENCH] = TRUE;
        break;
    case S_SCARE:
        /*
         * A monster will refuse to step on a scare monster scroll
         * if it is dropped.  Thus reading it is a mistake and produces
         * laughter at the poor rogue's boo boo.
         */
        msg("You hear maniacal laughter in the distance.");
        s_know[S_SCARE] = TRUE;
        break;
    case S_REMOVE:
        if (cur_armor != NULL)
            cur_armor->o_flags &= ~ISCURSED;
        if (cur_weapon != NULL)
            cur_weapon->o_flags &= ~ISCURSED;
        if (cur_ring[LEFT] != NULL)
            cur_ring[LEFT]->o_flags &= ~ISCURSED;
        if (cur_ring[RIGHT] != NULL)
            cur_ring[RIGHT]->o_flags &= ~ISCURSED;
        msg("You feel as if somebody is watching over you.");
        s_know[S_REMOVE] = TRUE;
        break;
    case S_AGGR:
        /*
         * This scroll aggravates all the monsters on the current
         * level and sets them running towards the hero
         */
        aggravate();
        msg("You hear a high pitched humming noise.");
        s_know[S_AGGR] = TRUE;
        break;
    case S_NOP:
        msg("Nothing appears to happen.");
        break;
    case S_GENOCIDE:
        msg("You have been granted the boon of genocide");
        genocide();
        s_know[S_GENOCIDE] = TRUE;
        break;
    default:
        msg("What a puzzling scroll!");
        return;
    }
    look(TRUE);                 /* put the result of the scroll on the screen */
    status();
    if (s_know[obj->o_which] && s_guess[obj->o_which]) {
        free(s_guess[obj->o_which]);
        s_guess[obj->o_which] = NULL;
    } else if (!s_know[obj->o_which] && askme && s_guess[obj->o_which] == NULL) {
        msg(terse ? "Call it: " : "What do you want to call it? ");
        if (get_str(buf, cw) == NORM) {
            s_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
            if (s_guess[obj->o_which] != NULL)
                strcpy(s_guess[obj->o_which], buf);
        }
    }
    if (--obj->o_count < 1) {
        detach(pack, item);
        discard(item);
    }
    inpack--;
}
