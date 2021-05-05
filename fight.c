/*
 * All the fighting gets done here
 *
 * @(#)fight.c	3.28 (Berkeley) 6/15/81
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

static long e_levels[] = {
    10L, 20L, 40L, 80L, 160L, 320L, 640L, 1280L, 2560L, 5120L, 10240L,
    20480L,
    40920L, 81920L, 163840L, 327680L, 655360L, 1310720L, 2621440L, 0L
};

int add_dam(short str);
void bounce(struct object *weap, char *mname);
void hit(char *er, char *ee);
void miss(char *er, char *ee);
int str_plus(short str);

/*
 * fight:
 *	The player attacks the monster.
 */

bool fight(coord * mp, char mn, struct object *weap, bool thrown)
{
    struct thing *tp;
    struct linked_list *item;
    bool did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL) {
        return 0;
    }
    tp = (struct thing *) ldata(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    quiet = 0;
    runto(mp, &hero);
    /*
     * Let them know it was really a mimic (if it was one).
     */
    if (tp->t_type == 'M' && tp->t_disguise != 'M' && off(player, ISBLIND)) {
        msg("Wait! That's a mimic!");
        tp->t_disguise = 'M';
        did_hit = thrown;
    }
    if (did_hit) {
        char *mname;

        did_hit = FALSE;
        if (on(player, ISBLIND))
            mname = "it";
        else
            mname = monsters[mn - 'A'].m_name;
        if (roll_em(&pstats, &tp->t_stats, weap, thrown)) {
            did_hit = TRUE;
            if (thrown)
                thunk(weap, mname);
            else
                hit(NULL, mname);
            if (on(player, CANHUH)) {
                msg("The %s is confused!", mname);
                tp->t_flags |= ISHUH;
                player.t_flags &= ~CANHUH;
            }
            if (tp->t_stats.s_hpt <= 0)
                killed(item, TRUE);
        } else if (thrown) {
            bounce(weap, mname);
        } else {
            miss(NULL, mname);
        }
    }
    cmdcount = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */

int attack(struct thing *mp)
{
    char *mname;

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    quiet = 0;
    if (mp->t_type == 'M' && off(player, ISBLIND))
        mp->t_disguise = 'M';
    if (on(player, ISBLIND))
        mname = "it";
    else
        mname = monsters[mp->t_type - 'A'].m_name;
    if (roll_em(&mp->t_stats, &pstats, NULL, FALSE)) {
        hit(mname, NULL);
        if (pstats.s_hpt <= 0)
            death(mp->t_type);  /* Bye bye life ... */
        if (off(*mp, ISCANC)) {
            switch (mp->t_type) {
            case 'R':
                /*
                 * If a rust monster hits, you lose armor (maybe)
                 */
                if (cur_armor != NULL && cur_armor->o_which != LEATHER
                    && cur_armor->o_ac < 9) {
                    if (cur_armor->o_which == STUDDED_LEATHER && rnd(100) < 80)
                        break;
                    if (!terse)
                        msg("Your armor appears to be weaker now. Oh my!");
                    else
                        msg("Your armor weakens");
                    cur_armor->o_ac++;
                }
                break;
            case 'E':
                /*
                 * The gaze of the floating eye hypnotizes you.
                 */
                if (on(player, ISBLIND))
                    break;
                if (!no_command) {
                    addmsg("You are transfixed");
                    if (!terse)
                        addmsg(" by the gaze of the floating eye.");
                    endmsg();
                    no_command = SLEEPTIME + rnd(4);
                }
                break;
            case 'A':
                /*
                 * Ants have poisonous bites
                 */
                if (!save(VS_POISON)) {
                    if (!ISWEARING(R_SUSTSTR) && pstats.s_str > MINSTRENGTH) {
                        chg_str(-1, 0);
                        if (!terse)
                            msg("You feel a sting in your arm and now feel weaker");
                        else
                            msg("A sting has weakened you");
                    } else {
                        if (!terse)
                            msg("A sting momentarily weakens you");
                        else
                            msg("Sting has no effect");
                    }
                }
                break;
            case 'W':
                /*
                 * Wraiths might drain energy levels
                 */
                if (rnd(100) < 15) {
                    int fewer;

                    if (pstats.s_exp == 0)
                        death('W');     /* All levels gone */
                    msg("You suddenly feel weaker.");
                    if (--pstats.s_lvl == 0) {
                        pstats.s_exp = 0;
                        pstats.s_lvl = 1;
                    } else
                        pstats.s_exp = e_levels[pstats.s_lvl - 1] + 1;
                    fewer = roll(1, 10);
                    pstats.s_hpt -= fewer;
                    max_hp -= fewer;
                    if (pstats.s_hpt < 1)
                        pstats.s_hpt = 1;
                    if (max_hp < 1)
                        death('W');
                }
                break;
            case 'F':
                /*
                 * Violet fungi stops them from moving
                 */
                player.t_flags |= ISHELD;
                sprintf(monsters['F' - 'A'].m_stats.s_dmg, "%dd1", ++fung_hit);
                break;
            case 'L':
                /*
                 * Lampades drive the player mad (and summon other
                 * monsters). Technically they should cast light like
                 * the player does because torches but that's more work.
                 */
                if (!save(VS_MAGIC)) {
                    if (off(player, ISHUH)) {
                        msg("The lampades strike about you with their torches!");
                        player.t_flags |= ISHUH;
                        fuse(unconfuse, 0, roll(3, 4), AFTER);
                    } else if (rnd(100) > 60) {
                        msg("The lampades shout wildly!");
                        aggravate();
                    }
                }
                break;
            case 'N':
                {
                    struct linked_list *list, *steal;
                    struct object *obj;
                    int nobj;
                    /*
                     * Nymph steal a magic item; look through the pack
                     * and pick out one we like--not worn items.
                     */
                    steal = NULL;
                    for (nobj = 0, list = pack; list != NULL; list = next(list)) {
                        obj = (struct object *) ldata(list);
                        if (obj != cur_armor && obj != cur_weapon
                            && obj != cur_ring[LEFT] && obj != cur_ring[RIGHT]
                            && is_magic(obj) && rnd(++nobj) == 0)
                            steal = list;
                    }
                    if (steal != NULL) {
                        struct object *sobj;

                        sobj = (struct object *) ldata(steal);
                        remove_monster(&mp->t_pos,
                                       find_mons(mp->t_pos.y, mp->t_pos.x));
                        mp = NULL;
                        if (obj->o_count > 1 && obj->o_group == 0) {
                            int oc;

                            oc = obj->o_count;
                            sobj->o_count = 1;
                            msg("The nymph stole %s!", inv_name(sobj, TRUE));
                            sobj->o_count = oc - 1;
                        } else {
                            msg("The nymph stole %s!", inv_name(sobj, TRUE));
                            detach(pack, steal);
                            discard(steal);
                        }
                        inpack--;
                    }
                }
                break;
            default:
                break;
            }
        }
    } else if (mp->t_type != 'E') {
        if (mp->t_type == 'F') {
            pstats.s_hpt -= fung_hit;
            if (pstats.s_hpt <= 0)
                death(mp->t_type);      /* Bye bye life ... */
        }
        miss(mname, NULL);
    }
    /*
     * Check to see if this is a regenerating monster and let it heal if
     * it is.
     */
    if ((mp != NULL) && (on(*mp, ISREGEN) && rnd(100) < 33))
        mp->t_stats.s_hpt++;
    if (fight_flush)
        flushinp();
    cmdcount = 0;
    status();

    if (mp == NULL)
        return -1;
    else
        return 0;
}

/*
 * swing:
 *      returns true if the swing hits
 */

int swing(int at_lvl, int op_arm, int wplus)
{
    int res = rnd(20) + 1;
    int need = (21 - at_lvl) - op_arm;

    return res + wplus >= need;
}

/*
 * check_level:
 *      Check to see if they have gone up a level.
 */

void check_level(void)
{
    int i;

    for (i = 0; e_levels[i] != 0; i++) {
        if (e_levels[i] > pstats.s_exp)
            break;
    }
    i++;
    if (i > pstats.s_lvl) {
        int add = 2 * (i - pstats.s_lvl) + roll(i - pstats.s_lvl, 8);
        max_hp += add;
        if ((pstats.s_hpt += add) > max_hp)
            pstats.s_hpt = max_hp;
        msg("Welcome to level %d", i);
    }

    pstats.s_lvl = i;
}

/*
 * roll_em:
 *      Roll several attacks
 */

bool
roll_em(struct stats *att, struct stats *def, struct object *weap, bool hurl)
{
    char *cp;
    int ndice, nsides, def_arm;
    bool did_hit = FALSE;
    int prop_hplus, prop_dplus;

    prop_hplus = prop_dplus = 0;
    if (weap == NULL) {
        cp = att->s_dmg;
        prop_hplus += att->s_hplus;
        prop_dplus += att->s_dplus;
    } else if (hurl) {
        if ((weap->o_flags & ISMISL) && cur_weapon != NULL &&
            cur_weapon->o_which == weap->o_launch) {
            cp = weap->o_hurldmg;
            prop_hplus = cur_weapon->o_hplus;
            prop_dplus = cur_weapon->o_dplus;
        } else {
            cp = (weap->o_flags & ISMISL ? weap->o_damage : weap->o_hurldmg);
        }
    } else {
        cp = weap->o_damage;
        /*
         * Drain a staff of striking
         */
        if (weap->o_type == STICK && weap->o_which == WS_HIT
            && weap->o_charges == 0) {
            strcpy(weap->o_damage, "0d0");
            weap->o_hplus = weap->o_dplus = 0;
        }
    }
    while (1) {
        int damage;
        int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
        int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

        if (weap == cur_weapon) {
            if (ISRING(LEFT, R_ADDDAM))
                dplus += cur_ring[LEFT]->o_ac;
            else if (ISRING(LEFT, R_ADDHIT))
                hplus += cur_ring[LEFT]->o_ac;
            if (ISRING(RIGHT, R_ADDDAM))
                dplus += cur_ring[RIGHT]->o_ac;
            else if (ISRING(RIGHT, R_ADDHIT))
                hplus += cur_ring[RIGHT]->o_ac;
        }
        ndice = atoi(cp);
        if ((cp = strchr(cp, 'd')) == NULL)
            break;
        nsides = atoi(++cp);
        if (def == &pstats) {
            if (cur_armor != NULL)
                def_arm = cur_armor->o_ac;
            else
                def_arm = def->s_arm;
            if (ISRING(LEFT, R_PROTECT))
                def_arm -= cur_ring[LEFT]->o_ac;
            else if (ISRING(RIGHT, R_PROTECT))
                def_arm -= cur_ring[RIGHT]->o_ac;
        } else {
            def_arm = def->s_arm;
        }
        if (swing(att->s_lvl, def_arm, hplus + str_plus(att->s_str))) {
            int proll;

            proll = roll(ndice, nsides);
            damage = dplus + proll + add_dam(att->s_str);
            def->s_hpt -= max(0, damage);
            did_hit = TRUE;
        }
        if ((cp = strchr(cp, '/')) == NULL)
            break;
        cp++;
    }
    return did_hit;
}

/*
 * prname:
 *      The print name of a combatant
 */

char *prname(char *who, bool upper)
{
    static char tbuf[ROGUE_CHARBUF_MAX];

    *tbuf = '\0';
    if (who == 0) {
        strcpy(tbuf, "you");
    } else if (on(player, ISBLIND)) {
        strcpy(tbuf, "it");
    } else {
        strcpy(tbuf, "the ");
        strcat(tbuf, who);
    }
    if (upper)
        *tbuf = toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *      Print a message to indicate a succesful hit
 */

void hit(char *er, char *ee)
{
    char *s = NULL;

    addmsg(prname(er, TRUE));
    if (terse) {
        s = " hit.";
    } else {
        switch (rnd(4)) {
        case 0:
            s = " scored an excellent hit on ";
            break;
        case 1:
            s = " hit ";
            break;
        case 2:
            s = (er == 0 ? " have injured " : " has injured ");
            break;
        case 3:
            s = (er == 0 ? " swing and hit " : " swings and hits ");
        }
    }
    addmsg(s);
    if (!terse)
        addmsg(prname(ee, FALSE));
    endmsg();
}

/*
 * miss:
 *      Print a message to indicate a poor swing
 */

void miss(char *er, char *ee)
{
    char *s = NULL;

    addmsg(prname(er, TRUE));
    switch (terse ? 0 : rnd(4)) {
    case 0:
        s = (er == 0 ? " miss" : " misses");
        break;
    case 1:
        s = (er == 0 ? " swing and miss" : " swings and misses");
        break;
    case 2:
        s = (er == 0 ? " barely miss" : " barely misses");
        break;
    case 3:
        s = (er == 0 ? " don't hit" : " doesn't hit");
    }
    addmsg(s);
    if (!terse)
        addmsg(" %s", prname(ee, FALSE));
    endmsg();
}

/*
 * save_throw:
 *      See if a creature save against something
 */

int save_throw(int which, struct thing *tp)
{
    int need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return roll(1, 20) >= need;
}

/*
 * save:
 *      See if they save against various nasty things
 */

int save(int which)
{
    return save_throw(which, &player);
}

/*
 * str_plus:
 *      compute bonus/penalties for strength on the "to hit" roll
 */

inline int str_plus(short str)
{
    int adjust = 0;
    if (str > 16) {
        adjust = str - 16;
    } else if (str < 5) {
        adjust = str - 5;
    }
    return adjust;
}

/*
 * add_dam:
 *      compute additional damage done for exceptionally high or low strength
 */

inline int add_dam(short str)
{
    int adjust = 0;
    if (str > 14) {
        adjust = str - 14;
    } else if (str < 7) {
        adjust = str - 7;
    }
    return adjust;
}

/*
 * raise_level:
 *      They just magically went up a level.
 */

void raise_level(void)
{
    pstats.s_exp = e_levels[pstats.s_lvl - 1] + 1L;
    check_level();
}

/*
 * thunk:
 *      A missile hits a monster
 */

void thunk(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
        msg("The %s hits the %s", w_names[weap->o_which], mname);
    else
        msg("You hit the %s.", mname);
}

/*
 * bounce:
 *      A missile misses a monster
 */

void bounce(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
        msg("The %s misses the %s", w_names[weap->o_which], mname);
    else
        msg("You missed the %s.", mname);
}

/*
 * remove a monster from the screen
 */
void remove_monster(coord * mp, struct linked_list *item)
{
    mvwaddch(mw, mp->y, mp->x, ' ');
    mvwaddch(cw, mp->y, mp->x, ((struct thing *) ldata(item))->t_oldch);
    detach(mlist, item);
    discard(item);
}

/*
 * is_magic:
 *      Returns true if an object radiates magic
 */

int is_magic(struct object *obj)
{
    switch (obj->o_type) {
    case ARMOR:
        return obj->o_ac != a_class[obj->o_which];
        break;
    case WEAPON:
        return obj->o_hplus != 0 || obj->o_dplus != 0;
        break;
    case POTION:
    case SCROLL:
    case STICK:
    case RING:
    case AMULET:
        return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *      Called to put a monster to death
 */

void killed(struct linked_list *item, bool pr)
{
    struct thing *tp;
    struct linked_list *pitem, *nexti;

    tp = (struct thing *) ldata(item);
    if (pr) {
        addmsg(terse ? "Defeated " : "You have defeated ");
        if (on(player, ISBLIND)) {
            msg("it.");
        } else {
            if (!terse)
                addmsg("the ");
            msg("%s.", monsters[tp->t_type - 'A'].m_name);
        }
    }
    pstats.s_exp += tp->t_stats.s_exp;
    /*
     * Do adjustments if they went up a level
     */
    check_level();
    /*
     * If the monster was a violet fungi, un-hold the player
     */
    switch (tp->t_type) {
    case 'F':
        player.t_flags &= ~ISHELD;
        fung_hit = 0;
        strcpy(monsters['F' - 'A'].m_stats.s_dmg, "000d0");
        break;
    }
    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    while (pitem != NULL) {
        struct object *obj;

        nexti = next(tp->t_pack);
        obj = (struct object *) ldata(pitem);
        obj->o_pos = tp->t_pos;
        detach(tp->t_pack, pitem);
        fall(pitem, FALSE);
        pitem = nexti;
    }
    /*
     * Get rid of the monster.
     */
    remove_monster(&tp->t_pos, item);
}
