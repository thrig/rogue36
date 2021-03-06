/*
 * @(#)main.c	3.27 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef RDRND
#include <immintrin.h>
#endif

#include "rogue.h"

static int have_seed;

long argtol(const char *arg, const long min, const long max);
void endit(int p);
char *getroguedir(void);
static int load_savefile(void);
static void new_game(void);
static void seed_rng(void);

int main(int argc, char *argv[])
{
    int ch, show_score = 0;

#ifdef __OpenBSD__
    if (pledge("cpath flock rpath stdio tty unveil wpath", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?Wd:n:s")) != -1) {
        switch (ch) {
        case 'W':
#ifdef WIZARD
            wizard = TRUE;
#else
            errx(1, "there are no Wizards");
#endif
            break;
        case 'd':
            dnum = (int) argtol(optarg, (long) INT_MIN, (long) INT_MAX);
            have_seed = 1;
            break;
        case 'n':
            strncpy(whoami, optarg, WHOAMI_LEN);
            break;
        case 's':
            show_score = 1;
            break;
        case 'h':
        case '?':
        default:
            fputs("Usage: rogue [-d seed] [-n name] [-s]\n", stderr);
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    strcpy(roguedir, getroguedir());

    initscr();
    if (COLS < ROCOLS)
        fatal("Terminal must be at least 80 columns wide.\n");
    if (LINES < ROLINES)
        fatal("Terminal must have at least 24 rows.\n");

#ifdef __OpenBSD__
    if (unveil(roguedir, "crw") == -1) {
        endwin();
        err(1, "unveil failed");
    }
    if (unveil(NULL, NULL) == -1) {
        endwin();
        err(1, "unveil failed");
    }
#endif

    if (show_score) {
        endwin();
        score(SCORE_VIEW, 0, '\0');
        exit(1);
    }

    raw();
    noecho();
    nonl();
    setup_sigs();
    cw = newwin(ROLINES, ROCOLS, 0, 0);
    mw = newwin(ROLINES, ROCOLS, 0, 0);
    hw = newwin(ROLINES, ROCOLS, 0, 0);
#ifdef WITHKEYPAD
    keypad(cw, 1);
#endif
    clearok(cw, TRUE);

    if (load_savefile())
        restore();
    else
        new_game();

    playit();

    exit(1);                    /* NOTREACHED */
}

long argtol(const char *arg, const long min, const long max)
{
    char *ep;
    long val;
    errno = 0;
    val = strtol(arg, &ep, 0);
    if (arg[0] == '\0' || *ep != '\0')
        err(1, "strtol failed");
    if (errno == ERANGE && (val == LONG_MIN || val == LONG_MAX))
        err(1, "strtol failed");
    if (min != LONG_MIN && val < min)
        errx(1, "value is below minimum %ld", min);
    if (max != LONG_MAX && val > max)
        errx(1, "value is above maximum %ld", max);
    return val;
}

/*
 * endit:
 *      Exit the program abnormally.
 */

void endit(int p)
{
    fatal("O, I am slain!\n");
}

inline char *getroguedir(void)
{
    char *dir;
    struct stat sb;

    if ((dir = getenv("ROGUEHOME")) != NULL) {
        if (*dir) {
            if (strnlen(dir, ROGUEDIR_MAX + 1) > ROGUEDIR_MAX)
                errx(1, "ROGUEHOME is too long");
            if (stat(dir, &sb) == -1)
                err(1, "could not stat ROGUEHOME");
            if (S_ISDIR(sb.st_mode))
                return dir;
            else
                errx(1, "ROGUEHOME must be a directory");
        }
    }

    return ".";
}

/*
 * This should allow multiple savefiles per account, minus those where
 * the player name contains reserved characters.
 */

inline static int load_savefile(void)
{
    struct stat sb;
    char fname[WHOAMI_LEN + 1], *fp, *wp;
    if (!*whoami)
        strncpy(whoami, "Nobody", 7);
    wp = whoami;
    fp = fname;
    while (*wp) {
        *fp = *wp;
        if (*fp == '/' || *fp == '.')
            *fp = '_';
        wp++;
        fp++;
    }
    *fp = '\0';
    snprintf(save_file, PATH_MAX, "%s/sav.%x.%s", roguedir, getuid(), fname);
    if (stat(save_file, &sb) == -1)
        return 0;
    if (S_ISREG(sb.st_mode))
        return 1;
    else
        return 0;
}

inline static void new_game(void)
{
    char *ropts;

    if ((ropts = getenv("ROGUEOPTS")) != NULL)
        parse_opts(ropts);

    seed_rng();
    seed = dnum;

    init_player();
    init_things();              /* Set up probabilities of things */
    init_names();               /* Set up names of scrolls */
    init_colors();              /* Set up colors of potions */
    init_stones();              /* Set up stone settings of rings */
    init_materials();           /* Set up materials of wands */

    new_level();

    start_daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    start_daemon(stomach, 0, AFTER);
    start_daemon(runners, 0, AFTER);

    if (wizard)
        msg("Welcome to dungeon %d, Wizard", dnum);
    else
        msg("Welcome to the Dungeons of Doom!");
    draw(cw);
}

/*
 * playit:
 *      The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

inline void playit(void)
{
    if (baudrate() < 1200) {
        terse = TRUE;
        jump = TRUE;
    }

    oldpos = hero;
    oldrp = roomin(&hero);

    while (1)
        command();

    /* NOTREACHED */
    endit(-1);
}

static void seed_rng(void)
{
    if (!have_seed) {
#ifdef RDRND
        if (_rdrand32_step((unsigned int *)&dnum) != 1)
            errx(1, "RDRAND failed");
#elif defined(__OpenBSD__)
        dnum = (int) arc4random();
#else
        int fd = open(DEV_RANDOM, O_RDONLY);
        if (fd == -1)
            err(1, "open failed '%s'", DEV_RANDOM);
        if (read(fd, &dnum, sizeof(dnum)) != sizeof(dnum))
            err(1, "read failed %s", DEV_RANDOM);
        close(fd);
#endif
    }
}

inline void setup_sigs(void)
{
#ifdef SIGHUP
    signal(SIGHUP, auto_save);
#endif
    signal(SIGILL, auto_save);
#ifdef SIGTRAP
    signal(SIGTRAP, auto_save);
#endif
#ifdef SIGIOT
    signal(SIGIOT, auto_save);
#endif
#ifdef SIGEMT
    signal(SIGEMT, auto_save);
#endif
    signal(SIGFPE, auto_save);
#ifdef SIGBUS
    signal(SIGBUS, auto_save);
#endif
    signal(SIGSEGV, auto_save);
#ifdef SIGSYS
    signal(SIGSYS, auto_save);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, auto_save);
#endif
    signal(SIGTERM, auto_save);
    signal(SIGINT, quit);
    /* disabled so can try to get core files when some goes awry
       #ifdef SIGQUIT
       signal(SIGQUIT, endit);
       #endif
     */
    /* There used to be TSTP code but that is also done by ncurses, and
     * this code is not acting on a ^Z anyways so the TSTP code got
     * removed. */
}
