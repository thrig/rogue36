/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	3.7 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include "rogue.h"

/*
 * doctor:
 *	A healing daemon that restores player hit points after rest
 */

int doctor(void)
{
    int lv, ohp;

    lv = pstats.s_lvl;
    ohp = pstats.s_hpt;
    quiet++;
    if (lv < 8) {
        if (quiet > 11 - lv)
            pstats.s_hpt++;
    } else if (quiet >= 3)
        pstats.s_hpt += rnd(lv - 7) + 1;
    if (ISRING(LEFT, R_REGEN))
        pstats.s_hpt++;
    if (ISRING(RIGHT, R_REGEN))
        pstats.s_hpt++;
    if (ohp != pstats.s_hpt) {
        if (pstats.s_hpt > max_hp)
            pstats.s_hpt = max_hp;
        quiet = 0;
    }
    return 0;
}

/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */

int swander(void)
{
    start_daemon(rollwand, 0, BEFORE);
    return 0;
}

/*
 * rollwand:
 *	Called to roll to see if a wandering monster starts up
 */

int between = 0;

int rollwand(void)
{
    if (++between >= 4) {
        if (roll(1, 6) == 4) {
            wanderer();
            kill_daemon(rollwand);
            fuse(swander, 0, WANDERTIME, BEFORE);
        }
        between = 0;
    }
    return 0;
}

/*
 * unconfuse:
 *	Release the poor player from their confusion
 */

int unconfuse(void)
{
    player.t_flags &= ~ISHUH;
    msg("You feel less confused now");
    return 0;
}


/*
 * unsee:
 *	They lost their see invisible power
 */

int unsee(void)
{
    player.t_flags &= ~CANSEE;
    return 0;
}

/*
 * sight:
 *	They got their sight back
 */

int sight(void)
{
    if (on(player, ISBLIND)) {
        extinguish(sight);
        player.t_flags &= ~ISBLIND;
        light(&hero);
        msg("The veil of darkness lifts");
    }
    return 0;
}

/*
 * nohaste:
 *	End the hasting
 */

int nohaste(void)
{
    player.t_flags &= ~ISHASTE;
    msg("You feel yourself slowing down.");
    return 0;
}

/*
 * digest the hero's food
 */
int stomach(void)
{
    int oldfood;

    if (food_left <= 0) {
        /*
         * the hero is fainting
         */
        if (no_command || rnd(100) > 20)
            return 0;
        no_command = rnd(8) + 4;
        if (!terse)
            addmsg("You feel too weak from lack of food.  ");
        msg("You faint");
        running = FALSE;
        count = 0;
        hungry_state = 3;
    } else {
        oldfood = food_left;
        food_left -= ring_eat(LEFT) + ring_eat(RIGHT) + 1 - amulet;

        if (food_left < MORETIME && oldfood >= MORETIME) {
            msg("You are starting to feel weak");
            hungry_state = 2;
        } else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME) {
            if (!terse)
                msg("You are starting to get hungry");
            else
                msg("Getting hungry");
            hungry_state = 1;
        }
    }
    return 0;
}
