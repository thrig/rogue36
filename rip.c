/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	3.13 (Berkeley) 6/16/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <sys/file.h>
#include <sys/types.h>

#include <err.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rogue.h"

extern int revision_num;

static char *rip[] = {
    "                       __________",
    "                      /          \\",
    "                     /    REST    \\",
    "                    /      IN      \\",
    "                   /     PEACE      \\",
    "                  /                  \\",
    "                  |                  |",
    "                  |                  |",
    "                  |   killed by a    |",
    "                  |                  |",
    "                  |       1980       |",
    "                 *|     *  *  *      | *",
    "         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

char *killname(char monst);

/*
 * death:
 *	Do something really fun when they die
 */

void death(char monst)
{
    char **dp = rip, *killer;
    struct tm *lt;
    time_t date;
    char buf[ROGUE_CHARBUF_MAX];

    time(&date);
    lt = localtime(&date);
    clear();
    move(8, 0);
    while (*dp)
        printw("%s\n", *dp++);
    mvaddstr(14, 28 - (((int) strlen(whoami) + 1) / 2), whoami);
    purse -= purse / 10;
    sprintf(buf, "%d Au", purse);
    mvaddstr(15, 28 - (((int) strlen(buf) + 1) / 2), buf);
    killer = killname(monst);
    mvaddstr(17, 28 - (((int) strlen(killer) + 1) / 2), killer);
    mvaddstr(16, 33, vowelstr(killer));
    sprintf(prbuf, "%4d", 1900 + lt->tm_year);
    mvaddstr(18, 26, prbuf);
    move(ROLINES - 1, 0);
    draw(stdscr);
    score(SCORE_DEATH, purse, monst);
    exit(1);
}

/*
 * score -- figure score and post it.
 */

void score(int type, int amount, char monst)
{
    static struct sc_ent {
        int sc_score;
        char sc_name[WHOAMI_LEN + 1];
        int sc_type;
        int sc_level;
        unsigned char sc_monster;
    } scores[MAX_SCORES];
    struct sc_ent *scp;
    int i;
    unsigned int sc_monster_i;
    struct sc_ent *sc2;
    FILE *outf;
    char *killer;
    int fd;
    static char *reason[] = {
        "killed",
        "quit",
        "A total winner",
    };
    char scoreline[ROGUE_CHARBUF_MAX];
    char score_file[PATH_MAX];
    int scorefile_ver = 0;

    snprintf(score_file, PATH_MAX, "%s/highscores", roguedir);

    if ((fd = open(score_file, O_RDWR | O_CREAT, 0666)) < 0) {
        if (type != SCORE_VIEW && type != SCORE_QUIT)
            endwin();
        err(1, "could not open score file");
    }
    flock(fd, LOCK_SH);
    outf = fdopen(fd, "w");

    for (scp = scores; scp < &scores[MAX_SCORES]; scp++) {
        scp->sc_score = 0;
        for (i = 0; i < WHOAMI_LEN; i++)
            scp->sc_name[i] = rnd(255);
        scp->sc_name[WHOAMI_LEN] = '\0';
        scp->sc_type = RN;
        scp->sc_level = RN;
        scp->sc_monster = RN;
    }

    signal(SIGINT, SIG_DFL);

    if (type != SCORE_VIEW && type != SCORE_QUIT) {
        mvaddstr(ROLINES - 1, 0, "--Press return to continue--");
        draw(stdscr);
        wait_for(stdscr, '\n');
        endwin();
    }

    read(fd, scoreline, 32);
    sscanf(scoreline, "rogue 3.6.3 highscores %8d\n", &scorefile_ver);
    reset_encstr();
    if (scorefile_ver == revision_num) {
        for (i = 0; i < MAX_SCORES; i++) {
            encread((char *) &scores[i].sc_name, WHOAMI_LEN + 1, fd);
            encread((char *) scoreline, ROGUE_CHARBUF_MAX, fd);
            sscanf(scoreline, " %d %d %d %u \n",
                   &scores[i].sc_score, &scores[i].sc_type,
                   &scores[i].sc_level, &sc_monster_i);
            scores[i].sc_monster = (unsigned char) sc_monster_i;
        }
    }

    /*
     * Insert them in the list if need be
     */
    if (type != SCORE_VIEW && !wizard) {
        for (scp = scores; scp < &scores[MAX_SCORES]; scp++) {
            if (amount > scp->sc_score)
                break;
        }
        if (scp < &scores[MAX_SCORES]) {
            for (sc2 = &scores[MAX_SCORES - 1]; sc2 > scp; sc2--)
                *sc2 = *(sc2 - 1);
            scp->sc_score = amount;
            strcpy(scp->sc_name, whoami);
            scp->sc_type = type;
            if (type == SCORE_WIN)
                scp->sc_level = max_level;
            else
                scp->sc_level = level;
            scp->sc_monster = monst;
        }
    }

    if (type != SCORE_VIEW)
        printf("\n\n\n");

    printf("Top Ten Adventurers:\nRank\tScore\tName\n");
    for (scp = scores; scp < &scores[MAX_SCORES]; scp++) {
        if (scp->sc_score) {
            printf("%ld\t%d\t%s: %s on level %d", (long) (scp - scores + 1),
                   scp->sc_score, scp->sc_name, reason[scp->sc_type],
                   scp->sc_level);
            if (scp->sc_type == SCORE_DEATH) {
                printf(" by a");
                killer = killname(scp->sc_monster);
                if (*killer == 'a' || *killer == 'e' || *killer == 'i' ||
                    *killer == 'o' || *killer == 'u')
                    putchar('n');
                printf(" %s", killer);
            }
            printf(".\n");
        }
    }
    if (type == SCORE_VIEW || wizard) {
        fclose(outf);
        flock(fd, LOCK_UN);
        return;
    }

    flock(fd, LOCK_EX);
    fseek(outf, 0L, SEEK_SET);
    fprintf(outf, "rogue 3.6.3 highscores %08d\n", revision_num);
    reset_encstr();
    for (i = 0; i < MAX_SCORES; i++) {
        encwrite((char *) &scores[i].sc_name, WHOAMI_LEN + 1, outf);
        sprintf(scoreline, " %d %d %d %d \n",
                scores[i].sc_score, scores[i].sc_type,
                scores[i].sc_level, scores[i].sc_monster);
        encwrite((char *) scoreline, ROGUE_CHARBUF_MAX, outf);
    }
    fclose(outf);
    flock(fd, LOCK_UN);
}

void total_winner(void)
{
    struct linked_list *item;
    struct object *obj;
    int worth = 0;
    char c;
    int oldpurse;

    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr
        ("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr("a great profit and are admitted to the fighters guild.\n");
    mvaddstr(ROLINES - 1, 0, "--Press space to continue--");
    refresh();
    wait_for(cw, ' ');
    clear();
    mvaddstr(0, 0, "   Worth  Item");
    oldpurse = purse;
    for (c = 'a', item = pack; item != NULL; c++, item = next(item)) {
        obj = (struct object *) ldata(item);
        switch (obj->o_type) {
        case FOOD:
            worth = 2 * obj->o_count;
            break;
        case WEAPON:
            switch (obj->o_which) {
            case MACE:
                worth = 8;
                break;
            case SWORD:
                worth = 15;
                break;
            case BOW:
                worth = 75;
                break;
            case ARROW:
                worth = 1;
                break;
            case DAGGER:
                worth = 2;
                break;
            case ROCK:
                worth = 1;
                break;
            case TWOSWORD:
                worth = 30;
                break;
            case SLING:
                worth = 1;
                break;
            case DART:
                worth = 1;
                break;
            case CROSSBOW:
                worth = 15;
                break;
            case BOLT:
                worth = 1;
                break;
            case SPEAR:
                worth = 2;
                break;
            default:
                worth = 0;
            }
            worth *= (1 + (10 * obj->o_hplus + 10 * obj->o_dplus));
            worth *= obj->o_count;
            obj->o_flags |= ISKNOW;
            break;
        case ARMOR:
            switch (obj->o_which) {
            case LEATHER:
                worth = 5;
                break;
            case RING_MAIL:
                worth = 30;
                break;
            case STUDDED_LEATHER:
                worth = 15;
                break;
            case SCALE_MAIL:
                worth = 3;
                break;
            case CHAIN_MAIL:
                worth = 75;
                break;
            case SPLINT_MAIL:
                worth = 80;
                break;
            case BANDED_MAIL:
                worth = 90;
                break;
            case PLATE_MAIL:
                worth = 400;
                break;
            default:
                worth = 0;
            }
            worth *= (1 + (10 * (a_class[obj->o_which] - obj->o_ac)));
            obj->o_flags |= ISKNOW;
            break;
        case SCROLL:
            s_know[obj->o_which] = TRUE;
            worth = s_magic[obj->o_which].mi_worth;
            worth *= obj->o_count;
            break;
        case POTION:
            p_know[obj->o_which] = TRUE;
            worth = p_magic[obj->o_which].mi_worth;
            worth *= obj->o_count;
            break;
        case RING:
            obj->o_flags |= ISKNOW;
            r_know[obj->o_which] = TRUE;
            worth = r_magic[obj->o_which].mi_worth;
            if (obj->o_which == R_ADDSTR || obj->o_which == R_ADDDAM ||
                obj->o_which == R_PROTECT || obj->o_which == R_ADDHIT) {
                if (obj->o_ac > 0)
                    worth += obj->o_ac * 20;
                else
                    worth = 50;
            }
            break;
        case STICK:
            obj->o_flags |= ISKNOW;
            ws_know[obj->o_which] = TRUE;
            worth = ws_magic[obj->o_which].mi_worth;
            worth += 20 * obj->o_charges;
            break;
        case AMULET:
            worth = 1000;
        }
        mvprintw(c - 'a' + 1, 0, "%c) %5d  %s", c, worth, inv_name(obj, FALSE));
        purse += worth;
    }
    mvprintw(c - 'a' + 1, 0, "   %5d  Gold Peices          ", oldpurse);
    refresh();
    score(SCORE_WIN, purse, '\0');

    exit(EXIT_SUCCESS);
}

char *killname(char monst)
{
    if (isupper(monst))
        return monsters[monst - 'A'].m_name;
    else
        switch (monst) {
        case 'a':
            return "arrow";
        case 'd':
            return "dart";
        case 'b':
            return "bolt";
        }
    return "";
}
