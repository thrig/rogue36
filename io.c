/*
 * Various input/output functions
 *
 * @(#)io.c	3.10 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "rogue.h"

static struct timespec readdelay = { 0, READ_DELAY };

void strucpy(char *s1, char *s2, int len);

/*
 * msg:
 *	Display a message at the top of the screen.
 */

static char msgbuf[BUFSIZ];
static int newpos = 0;

/*
 * get_str:
 *  A primitive line-editing buffer with length limits
 */
int get_str(char *opt, WINDOW * win)
{
    char *sp;
    int c, oy, ox;
    char buf[GETSTR_MAX];

    draw(win);
    getyx(win, oy, ox);
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

void msg(char *fmt, ...)
{
    va_list ap;
    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0') {
        wmove(cw, 0, 0);
        wclrtoeol(cw);
        mpos = 0;
        return;
    }
    /*
     * otherwise add to the message and flush it out
     */
    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
    endmsg();
}

/*
 * add things to the current message
 */
void addmsg(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
}

/*
 * Display a new msg (giving them a chance to see the previous one if it
 * is up there with the --More--)
 */
void endmsg(void)
{
    strncpy(huh, msgbuf, ROGUE_CHARBUF_MAX);
    huh[79] = 0;

    if (mpos) {
        wmove(cw, 0, mpos);
        waddstr(cw, "--More--");
        draw(cw);
        wait_for(cw, ' ', 1);
    }

    mvwaddstr(cw, 0, 0, msgbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
    draw(cw);
}

void doadd(char *fmt, va_list ap)
{
    vsprintf(&msgbuf[newpos], fmt, ap);
    newpos = (int) strlen(msgbuf);
}

/*
 * step_ok:
 *      returns true if it is ok to step on ch
 */

int step_ok(char ch)
{
    switch (ch) {
    case ' ':
    case '|':
    case '-':
    case SECRETDOOR:
        return FALSE;
    default:
        return !isalpha(ch);
    }
}

/*
 * readchar:
 *      flushes stdout so that screen is up to date and then returns
 *      getchar.
 */

int readchar(WINDOW * win)
{
    int ch;

#ifdef WITHKEYPAD
    ch = md_readchar(win);
#else
    ch = wgetch(win);
    if (ch == ERR)
        ch = 27;
#endif

    nanosleep(&readdelay, NULL);
    flushinp();

    if (ch == 3 || ch == 0) {
        quit(0);
        return 27;
    }

    return ch;
}

/*
 * status:
 *      Display the important stats line.  Keep the cursor where it was.
 */

void status(void)
{
    int oy, ox, temp;
    static char buf[ROGUE_CHARBUF_MAX];
    static int hpwidth = 0, s_hungry = -1;
    static int s_lvl = -1, s_pur, s_hp = -1, s_ac = 0;
    static short s_str;
    static long s_exp = 0;

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
        && s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm)
        && s_str == pstats.s_str && s_lvl == level && s_hungry == hungry_state)
        return;

    getyx(cw, oy, ox);
    if (s_hp != max_hp) {
        temp = s_hp = max_hp;
        for (hpwidth = 0; temp; hpwidth++)
            temp /= 10;
    }
    sprintf(buf, "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %-2d"
            "  Ac: %-2d  Exp: %d/%ld", level, purse, hpwidth, pstats.s_hpt,
            hpwidth, max_hp, pstats.s_str,
            cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
            pstats.s_exp);

    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str;
    s_exp = pstats.s_exp;
    s_ac = (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
    mvwaddstr(cw, ROLINES - 1, 0, buf);
    switch (hungry_state) {
    case HUNGRY_OKAY:
        break;
    case HUNGRY_HUN:
        waddstr(cw, "  Hungry");
        break;
    case HUNGRY_WEAK:
        waddstr(cw, "  Weak");
        break;
    case HUNGRY_FAINT:
        waddstr(cw, "  Fainting");
    }
    wclrtoeol(cw);
    s_hungry = hungry_state;
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *      Sit around until they type the right key (or sometimes also Escape)
 */

void wait_for(WINDOW * win, char ch, int escapes)
{
    char c;

    if (ch == '\n') {
        while ((c = readchar(win)) != '\n' && c != '\r')
            continue;
    } else {
        if (escapes) {
            while ((c = readchar(win)) != ch && c != '\033')
                continue;
        } else {
            while (readchar(win) != ch)
                continue;
        }
    }
}

/*
 * show_win:
 *      function used to display a window and wait before returning
 */

void show_win(WINDOW * scr, char *message)
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, unc(hero));
    draw(scr);
    wait_for(scr, ' ', 1);
    clearok(cw, TRUE);
    touchwin(cw);
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
