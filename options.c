/*
 * This file has all the code for the option command.
 * I would rather this command were not necessary, but
 * it is the only way to keep the wolves off of my back.
 *
 * @(#)options.c	3.3 (Berkeley) 5/25/81
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

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char *o_name;               /* option name */
    char *o_prompt;             /* prompt for interactive entry */
    int *o_opt;                 /* pointer to thing to set */
    int (*o_putfunc) ();        /* function to print value */
    int (*o_getfunc) ();        /* function to get value interactively */
};

typedef struct optstruct OPTION;

int put_bool(bool * b);
int get_bool(bool * bp, WINDOW * win);

OPTION optlist[] = {
    {"terse", "Terse output: ",
     (int *) &terse, put_bool, get_bool},
    {"flush", "Flush typeahead during battle: ",
     (int *) &fight_flush, put_bool, get_bool},
    {"jump", "Show position only at end of run: ",
     (int *) &jump, put_bool, get_bool},
    {"step", "Do inventories one line at a time: ",
     (int *) &slow_invent, put_bool, get_bool},
    {"askme", "Ask me about unidentified things: ",
     (int *) &askme, put_bool, get_bool}
};

/*
 * print and then set options from the terminal
 */
void option(void)
{
    OPTION *op;
    int retval;

    wclear(hw);
    touchwin(hw);
    /*
     * Display current values of options
     */
    for (op = optlist; op <= &optlist[NUM_OPTS - 1]; op++) {
        waddstr(hw, op->o_prompt);
        (*op->o_putfunc) (op->o_opt);
        waddch(hw, '\n');
    }
    /*
     * Set values
     */
    wmove(hw, 0, 0);
    for (op = optlist; op <= &optlist[NUM_OPTS - 1]; op++) {
        waddstr(hw, op->o_prompt);
        if ((retval = (*op->o_getfunc) (op->o_opt, hw))) {
            if (retval == QUIT) {
                break;
            } else if (op > optlist) {  /* MINUS */
                wmove(hw, (op - optlist) - 1, 0);
                op -= 2;
            } else {            /* trying to back up beyond the top */
                beep();
                wmove(hw, 0, 0);
                op--;
            }
        }
    }
    /*
     * Switch back to original screen
     */
    mvwaddstr(hw, ROLINES - 1, 0, "--Press space to continue--");
    draw(hw);
    wait_for(hw, ' ');
    clearok(cw, TRUE);
    touchwin(cw);
    after = FALSE;
}

/*
 * put out a boolean
 */
int put_bool(bool * b)
{
    waddstr(hw, *b ? "Yes" : "No");
    return 0;
}

/*
 * allow changing a boolean option and print it out
 */

int get_bool(bool * bp, WINDOW * win)
{
    int oy, ox;
    bool op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? "Yes" : "No");
    while (op_bad) {
        wmove(win, oy, ox);
        draw(win);
        switch (readchar(win)) {
        case 't':
        case 'T':
        case 'y':
        case 'Y':
            *bp = TRUE;
            op_bad = FALSE;
            break;
        case 'f':
        case 'F':
        case 'n':
        case 'N':
            *bp = FALSE;
            op_bad = FALSE;
            break;
        case '\n':
        case '\r':
            op_bad = FALSE;
            break;
        case '\033':
        case '\007':
            return QUIT;
        case '-':
            return MINUS;
        default:
            mvwaddstr(win, oy, ox + 10, "(Y or N)");
        }
    }
    wmove(win, oy, ox);
    waddstr(win, *bp ? "Yes" : "No");
    waddch(win, '\n');
    return NORM;
}

/*
 * set a string option -- only used by other calls now as the string
 * options have all been removed
 */
int get_str(char *opt, WINDOW * win)
{
    char *sp;
    int c, oy, ox;
    char buf[GETSTR_MAX];

    draw(win);
    getyx(win, oy, ox);
    // TODO does ncurses have a better routine for this (that does not
    // have security issues?
    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    for (sp = buf;
         (c = readchar(win)) != '\n' && c != '\r' && c != '\033' && c != '\007';
         wclrtoeol(win), draw(win)) {
        if (c == -1) {
            continue;
        } else if (c == erasechar()) {
            if (sp > buf) {
                int i;
                int myx, myy;

                sp--;

                for (i = (int) strlen(unctrl(*sp)); i; i--) {
                    getyx(win, myy, myx);
                    if ((myx == 0) && (myy > 0)) {
                        wmove(win, myy - 1, getmaxx(win) - 1);
                        waddch(win, ' ');
                        wmove(win, myy - 1, getmaxx(win) - 1);
                    } else {
                        waddch(win, '\b');
                    }
                }
            }
            continue;
        } else if (c == killchar()) {
            sp = buf;
            wmove(win, oy, ox);
            continue;
        } else if (sp == buf) {
            if (c == '-') {
                break;
            }
        }
        if ((sp - buf) < GETSTR_MAX) {  /* Avoid overflow */
            *sp++ = c;
            waddstr(win, unctrl(c));
        }
    }
    *sp = '\0';
    /* only change option if something has been typed */
    if (sp > buf)
        strucpy(opt, buf, strlen(buf));
    wmove(win, oy, ox);
    waddstr(win, opt);
    waddch(win, '\n');
    draw(win);
    if (win == cw)
        mpos += sp - buf;
    if (c == '-')
        return MINUS;
    else if (c == '\033' || c == '\007')
        return QUIT;
    else
        return NORM;
}

/*
 * parse options from string, usually taken from the environment.
 * the string is a series of comma seperated values, with booleans
 * being stated as "name" (true) or "noname" (false), and strings
 * being "name=....", with the string being defined up to a comma
 * or the end of the entire option string.
 */

void parse_opts(char *str)
{
    char *sp;
    OPTION *op;
    int len;

    while (*str) {
        /*
         * Get option name
         */
        for (sp = str; isalpha(*sp); sp++)
            continue;
        len = sp - str;
        /*
         * Look it up and deal with it
         */
        for (op = optlist; op <= &optlist[NUM_OPTS - 1]; op++) {
            if (EQSTR(str, op->o_name, len)) {
                if (op->o_putfunc == put_bool) {
                    *(bool *) op->o_opt = TRUE;
                }
                break;
            }
            /*
             * check for "noname" for booleans
             */
            else if (op->o_putfunc == put_bool && EQSTR(str, "no", 2)
                     && EQSTR(str + 2, op->o_name, len - 2)) {
                *(bool *) op->o_opt = FALSE;
                break;
            }
        }

        /*
         * skip to start of next option name
         */
        while (*sp && !isalpha(*sp))
            sp++;
        str = sp;
    }
}

/*
 * copy string using unctrl for things
 */
void strucpy(char *s1, char *s2, int len)
{
    const char *sp;

    while (len--) {
        strcpy(s1, (sp = unctrl(*s2)));
        s1 += strlen(sp);
        s2++;
    }
    *s1 = '\0';
}
