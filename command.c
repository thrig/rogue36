/*
 * Read and execute the user commands
 *
 * @(#)command.c	3.45 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "rogue.h"

extern int revision_num;

struct timespec jumpdelay = { 0, JUMP_DELAY };

void call(void);
void d_level(void);
void help(void);
void identify(void);
void search(void);
void u_level(void);
#ifdef WIZARD
void wiz_command(int ch);
#endif

/*
 * command:
 *	Process the user commands
 */

inline void command(void)
{
    char ch;
    int ntimes = 1;             /* Number of player moves */
    static char countch, direction, newcount = FALSE;

    if (on(player, ISHASTE))
        ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--) {
        look(TRUE);
        if (!running)
            door_stop = FALSE;
        status();
        lastscore = purse;
        wmove(cw, unc(hero));
        if (!((running || count) && jump)) {
            draw(cw);           /* Draw screen */
            nanosleep(&jumpdelay, NULL);
        }
        take = 0;
        after = TRUE;
        /*
         * Read command or continue run
         */
        if (!no_command) {
            if (running) {
                ch = runch;
            } else if (count) {
                ch = countch;
            } else {
                ch = readchar(cw);
                /* Erase message if its there */
                if (mpos != 0 && !running) {
                    msg("");
                    /* bounce back to hero if (maybe) entering digits */
                    wmove(cw, unc(hero));
                }
            }
        } else {
            ch = ' ';
        }
        if (no_command) {
            if (--no_command == 0) {
                msg("You can move again.");
                /*
                 * Avoid giving runners a(nother) free whack at the
                 * hero, though this does let the BEFORE fuses fire.
                 */
                after = FALSE;
            }
        } else {
            /*
             * check for prefixes
             */
            if (isdigit(ch)) {
                count = 0;
                newcount = TRUE;
                while (isdigit(ch)) {
                    count = count * 10 + (ch - '0');
                    ch = readchar(cw);
                }
                countch = ch;
                /*
                 * turn off count for commands which don't make sense
                 * to repeat
                 */
                switch (ch) {
                case 'h':
                case 'j':
                case 'k':
                case 'l':
                case 'y':
                case 'u':
                case 'b':
                case 'n':
                case 'H':
                case 'J':
                case 'K':
                case 'L':
                case 'Y':
                case 'U':
                case 'B':
                case 'N':
                case 'q':
                case 'r':
                case 's':
                case 'f':
                case 't':
                case 'I':
                case 'p':
                case 'z':
                case '.':
                    break;
                default:
                    count = 0;
                }
            }
            switch (ch) {
            case 'f':
                if (!on(player, ISBLIND)) {
                    door_stop = TRUE;
                    firstmove = TRUE;
                }
                if (count && !newcount)
                    ch = direction;
                else
                    ch = readchar(cw);
                switch (ch) {
                case 'h':
                case 'j':
                case 'k':
                case 'l':
                case 'y':
                case 'u':
                case 'b':
                case 'n':
                    ch = toupper(ch);
                }
                direction = ch;
            }
            newcount = FALSE;
            /*
             * execute a command
             */
            if (count && !running)
                count--;
            switch (ch) {
            case 'h':
                do_move(0, -1);
                break;
            case 'j':
                do_move(1, 0);
                break;
            case 'k':
                do_move(-1, 0);
                break;
            case 'l':
                do_move(0, 1);
                break;
            case 'y':
                do_move(-1, -1);
                break;
            case 'u':
                do_move(-1, 1);
                break;
            case 'b':
                do_move(1, -1);
                break;
            case 'n':
                do_move(1, 1);
                break;
            case 'H':
                do_run('h');
                break;
            case 'J':
                do_run('j');
                break;
            case 'K':
                do_run('k');
                break;
            case 'L':
                do_run('l');
                break;
            case 'Y':
                do_run('y');
                break;
            case 'U':
                do_run('u');
                break;
            case 'B':
                do_run('b');
                break;
            case 'N':
                do_run('n');
                break;
            case 't':          /* throw */
                if (!get_dir())
                    after = FALSE;
                else
                    missile(delta.y, delta.x);
                break;
            case 'Q':
                after = FALSE;
                quit(0);
                break;
            case 'i':
                after = FALSE;
                inventory(pack, 0);
                break;
            case 'I':
                after = FALSE;
                picky_inven();
                break;
            case 'd':
                drop();
                break;
            case 'q':
                quaff();
                break;
            case 'r':
                read_scroll();
                break;
                /* no I haven't been playing Angbands why do you ask */
            case 'E':
            case 'e':
                eat();
                break;
            case 'w':
                wield();
                break;
            case 'W':
                wear();
                break;
            case 'T':
                take_off();
                break;
            case 'P':
                ring_on();
                break;
            case 'R':
                ring_off();
                break;
            case 'o':
                option();
                break;
            case 'c':
                call();
                break;
            case '>':
                after = FALSE;
                d_level();
                break;
            case '<':
                after = FALSE;
                u_level();
                break;
            case '?':
                after = FALSE;
                help();
                break;
            case '/':
                after = FALSE;
                identify();
                break;
            case 's':
                search();
                break;
            case 'z':
                do_zap(FALSE);
                break;
            case 'p':
                if (get_dir())
                    do_zap(TRUE);
                else
                    after = FALSE;
                break;
            case 'v':
                msg("Rogue 3.6.3 + bugfixes + changes (%d)", revision_num);
                break;
            case CTRL('L'):
                after = FALSE;
                clearok(curscr, TRUE);
                draw(curscr);
                break;
            case CTRL('R'):
                after = FALSE;
                msg(huh);
                break;
            case 'S':
                after = FALSE;
                if (save_game()) {
                    wmove(cw, ROLINES - 1, 0);
                    wclrtoeol(cw);
                    draw(cw);
                    endwin();
                    exit(1);
                }
                break;
            case '.':          /* Rest command (used to be space) */
                break;
#ifdef WIZARD
            case '&':
                after = FALSE;
                if (!wizard) {
                    msg("You are suddenly as smart as Ken Arnold in dungeon %d",
                        dnum);
                    wizard = TRUE;
                }
                break;
#endif
            case ESCAPE:
                door_stop = FALSE;
                count = 0;
                after = FALSE;
                break;
            default:
                after = FALSE;
                if (wizard) {
#ifdef WIZARD
                    wiz_command(ch);
#else
                    ;
#endif
                } else {
                    msg("Illegal command '%s'.", unctrl(ch));
                    count = 0;
                }
            }
            /*
             * turn off flags if no longer needed
             */
            if (!running)
                door_stop = FALSE;
        }
        /*
         * If they ran into something, let them pick it up.
         */
        if (take != 0)
            pick_up(take);
        if (!running)
            door_stop = FALSE;
    }
    /*
     * Kick off the rest if the daemons and fuses
     */
    if (after) {
        look(FALSE);
        do_daemons(AFTER);
        do_fuses(AFTER);

        if (ISRING(LEFT, R_SEARCH))
            search();
        else if (ISRING(LEFT, R_TELEPORT) && rnd(100) < 2)
            teleport();

        if (ISRING(RIGHT, R_SEARCH))
            search();
        else if (ISRING(RIGHT, R_TELEPORT) && rnd(100) < 2)
            teleport();
    }
}

/*
 * quit:
 *      Have player make certain, then exit.
 */

void quit(int p)
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != &quit)
        mpos = 0;
    msg("Really quit?");
    draw(cw);
    if (readchar(cw) == 'Y') {
        clear();
        move(ROLINES - 1, 0);
        draw(stdscr);
        endwin();
        score(SCORE_QUIT, purse, '\0');
        exit(1);
    } else {
        signal(SIGINT, quit);
        wmove(cw, 0, 0);
        wclrtoeol(cw);
        status();
        draw(cw);
        mpos = 0;
        count = 0;
    }
}

/*
 * search:
 *      Player gropes around to find hidden things.
 */

void search(void)
{
    int x, y;
    char ch;

    if (on(player, ISBLIND))
        return;
    /*
     * Look all around the hero, if there is something hidden there,
     * give them a chance to find it.  If its found, display it.
     */
    for (x = hero.x - 1; x <= hero.x + 1; x++) {
        for (y = hero.y - 1; y <= hero.y + 1; y++) {
            ch = (char) winat(y, x);
            switch (ch) {
            case SECRETDOOR:
                if (rnd(100) < 10 + 6 * search_repeat++) {
                    mvaddch(y, x, DOOR);
                    count = 0;
                }
                break;
            case TRAP:
                {
                    struct trap *tp;

                    if (mvwinch(cw, y, x) == TRAP)
                        break;
                    if (rnd(100) > 50)
                        break;
                    tp = trap_at(y, x);
                    mvwaddch(cw, y, x, TRAP);
                    count = 0;
                    running = FALSE;
                    if ((tp->tr_flags & ISFOUND) != ISFOUND)
                        msg(tr_name(tp->tr_type));
                    tp->tr_flags |= ISFOUND;
                }
            }
        }
    }
}

/*
 * help:
 *      Give single character help, or the whole mess
 */

void help(void)
{
    struct h_list *strp = helpstr;
    char helpch;
    int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar(cw);
    mpos = 0;
    /*
     * If its not a *, print the right help string or an error if they
     * typed a funny character.
     */
    if (helpch != '*') {
        wmove(cw, 0, 0);
        while (strp->h_ch) {
            if (strp->h_ch == helpch) {
                msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
                break;
            }
            strp++;
        }
        if (strp->h_ch != helpch)
            msg("Unknown character '%s'", unctrl(helpch));
        return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
        mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
        waddstr(hw, strp->h_desc);
        cnt++;
        strp++;
    }
    wmove(hw, ROLINES - 1, 0);
    wprintw(hw, "--Press space to continue--");
    draw(hw);
    wait_for(hw, ' ');
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status();
    touchwin(cw);
}

/*
 * identify:
 *      Tell the player what a certain thing is.
 */

void identify(void)
{
    char ch, *str;

    msg("What do you want identified? ");
    ch = readchar(cw);
    mpos = 0;
    if (ch == ESCAPE) {
        msg("");
        return;
    }
    if (isalpha(ch) && isupper(ch)) {
        str = monsters[ch - 'A'].m_name;
    } else {
        switch (ch) {
        case '|':
        case '-':
            str = "wall of a room";
            break;
        case GOLD:
            str = "gold";
            break;
        case STAIRS:
            str = "passage leading down";
            break;
        case DOOR:
            str = "door";
            break;
        case FLOOR:
            str = "room floor";
            break;
        case PLAYER:
            str = "you";
            break;
        case PASSAGE:
            str = "passage";
            break;
        case TRAP:
            str = "trap";
            break;
        case POTION:
            str = "potion";
            break;
        case SCROLL:
            str = "scroll";
            break;
        case FOOD:
            str = "food";
            break;
        case WEAPON:
            str = "weapon";
            break;
        case ' ':
            str = "solid rock";
            break;
        case ARMOR:
            str = "armor";
            break;
        case AMULET:
            str = "The Amulet of Yendor";
            break;
        case RING:
            str = "ring";
            break;
        case STICK:
            str = "wand or staff";
            break;
        default:
            str = "unknown character";
        }
    }
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
void call(void)
{
    struct object *obj;
    struct linked_list *item;
    char **guess, *elsewise;
    bool *know;

    item = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
        return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type) {
    case RING:
        guess = r_guess;
        know = r_know;
        elsewise = (r_guess[obj->o_which] != NULL ?
                    r_guess[obj->o_which] : r_stones[obj->o_which]);
        break;
    case POTION:
        guess = p_guess;
        know = p_know;
        elsewise = (p_guess[obj->o_which] != NULL ?
                    p_guess[obj->o_which] : p_colors[obj->o_which]);
        break;
    case SCROLL:
        guess = s_guess;
        know = s_know;
        elsewise = (s_guess[obj->o_which] != NULL ?
                    s_guess[obj->o_which] : s_names[obj->o_which]);
        break;
    case STICK:
        guess = ws_guess;
        know = ws_know;
        elsewise = (ws_guess[obj->o_which] != NULL ?
                    ws_guess[obj->o_which] : ws_made[obj->o_which]);
        break;
    default:
        msg("You can't call that anything");
        return;
    }
    if (know[obj->o_which]) {
        msg("That has already been identified");
        return;
    }
    if (terse)
        addmsg("C");
    else
        addmsg("Was c");
    msg("alled \"%s\"", elsewise);
    if (terse)
        msg("Call it: ");
    else
        msg("What do you want to call it? ");
    strcpy(prbuf, elsewise);
    if (get_str(prbuf, cw) == NORM) {
        if (guess[obj->o_which] != NULL)
            free(guess[obj->o_which]);
        guess[obj->o_which] = malloc((unsigned int) strlen(prbuf) + 1);
        if (guess[obj->o_which] != NULL)
            strcpy(guess[obj->o_which], prbuf);
    }
}

/*
 * d_level:
 *      They want to go down a level
 */

void d_level(void)
{
    if (winat(hero.y, hero.x) != STAIRS) {
        msg("I see no way down.");
    } else {
        level++;
        new_level();
    }
}

/*
 * u_level:
 *      They want to go up a level
 */

void u_level(void)
{
    if (winat(hero.y, hero.x) == STAIRS) {
        if (amulet) {
            level--;
            if (level == 0)
                total_winner();
            new_level();
            msg("You feel a wrenching sensation in your gut.");
            return;
        }
    }
    msg("I see no way up.");
}

#ifdef WIZARD
/*
 * wiz_command:
 *  Meddles with the keyboard input of a Wizard
 */

inline void wiz_command(int ch)
{
    switch (ch) {
    case '@':
        msg("@ %d,%d (window %d,%d)", hero.y, hero.x, ROLINES, ROCOLS);
        break;
    case 'C':
        create_obj();
        break;
    case CTRL('I'):
        inventory(lvl_obj, 0);
        break;
    case CTRL('W'):
        whatis();
        break;
    case CTRL('D'):
        level++;
        new_level();
        break;
    case CTRL('U'):
        level--;
        new_level();
        break;
    case CTRL('F'):
        show_win(stdscr, "--More (level map)--");
        break;
    case CTRL('X'):
        show_win(mw, "--More (monsters)--");
        break;
    case CTRL('T'):
        teleport();
        break;
    case CTRL('E'):
        msg("food left: %d", food_left);
        break;
    case CTRL('A'):
        msg("%d things in your pack", inpack);
        break;
    case CTRL('C'):
        add_pass();
        break;
    case CTRL('N'):
        {
            struct linked_list *item;

            if ((item = get_item("charge", STICK)) != NULL)
                ((struct object *) ldata(item))->o_charges = 10000;
        }
        break;
    case CTRL('H'):
        {
            struct linked_list *item;
            struct object *obj;

            for (int i = 0; i < 9; i++)
                raise_level();
            /*
             * Give the rogue a sword (+1,+1)
             */
            item = new_item(sizeof *obj);
            obj = (struct object *) ldata(item);
            obj->o_type = WEAPON;
            obj->o_which = TWOSWORD;
            init_weapon(obj, SWORD);
            obj->o_hplus = 1;
            obj->o_dplus = 1;
            add_pack(item, TRUE);
            cur_weapon = obj;
            /*
             * And their suit of armor
             */
            item = new_item(sizeof *obj);
            obj = (struct object *) ldata(item);
            obj->o_type = ARMOR;
            obj->o_which = PLATE_MAIL;
            obj->o_ac = -5;
            obj->o_flags |= ISKNOW;
            cur_armor = obj;
            add_pack(item, TRUE);
        }
        break;
    default:
        msg("Illegal command '%s'.", unctrl(ch));
        count = 0;
    }
}
#endif
