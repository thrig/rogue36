/*
 * save and restore routines
 *
 * @(#)save.c	3.9 (Berkeley) 6/16/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rogue.h"

extern char version[], encstr[];
extern int version_num, revision_num;

int dosave(FILE * savef);

int save_game(void)
{
    FILE *savef;

    mvwaddstr(cw, 0, 0, "Save game?");
    draw(cw);
    if (readchar(cw) != 'Y')
        return FALSE;

    msg("Be seeing you...");

    if ((savef = fopen(save_file, "w")) == NULL)
        msg(strerror(errno));   /* fake perror() */

    if (dosave(savef) != 0) {
        msg("Save game failed!");
        return FALSE;
    }
    return TRUE;
}

/*
 * automatically save a file.  This is used if a HUP signal is
 * recieved
 */
void auto_save(int p)
{
    FILE *savef;

    for (int i = 0; i < NSIG; i++)
        signal(i, SIG_IGN);
    if ((savef = fopen(save_file, "w")) != NULL)
        dosave(savef);
    endwin();
    exit(1);
}

/*
 * write the saved game on the file
 */
int dosave(FILE * savef)
{
    char buf[ROGUE_CHARBUF_MAX];
    int ret;

    wmove(cw, ROLINES - 1, 0);
    draw(cw);

    memset(buf, 0, ROGUE_CHARBUF_MAX);
    strcpy(buf, version);
    encwrite(buf, ROGUE_CHARBUF_MAX, savef);
    memset(buf, 0, ROGUE_CHARBUF_MAX);
    sprintf(buf, "R%d %d\n", version_num, revision_num);
    encwrite(buf, ROGUE_CHARBUF_MAX, savef);
    memset(buf, 0, ROGUE_CHARBUF_MAX);

    ret = rs_save_file(savef);
    fclose(savef);
    return ret;
}

inline void restore(void)
{
    int inf;
    char buf[ROGUE_CHARBUF_MAX];
    int rogue_version = 0, savefile_version = 0;

    if ((inf = open(save_file, O_RDONLY)) < 0) {
        endwin();
        err(1, "open failed");
    }

    encread(buf, ROGUE_CHARBUF_MAX, inf);

    if (strcmp(buf, version) != 0) {
        endwin();
        printf("Sorry, saved game is out of date.\n");
        exit(1);
    }

    encread(buf, ROGUE_CHARBUF_MAX, inf);
    sscanf(buf, "R%d %d\n", &rogue_version, &savefile_version);

    if ((rogue_version != version_num) && (savefile_version != revision_num)) {
        endwin();
        printf("Sorry, saved game format is out of date.\n");
        exit(1);
    }

    if (rs_restore_file(inf) != 0) {
        endwin();
        errx(1, "cannot restore savefile");
    }

    if (!wizard && (unlink(save_file) < 0)) {
        endwin();
        err(1, "unlink failed");
    }
}

/*
 * perform an "encrypted" write
 */
unsigned int encwrite(void *starta, unsigned int size, FILE * outf)
{
    char *ep;
    char *start = starta;
    unsigned int o_size = size;
    ep = encstr;

    while (size) {
        if (putc(*start++ ^ *ep++, outf) == EOF)
            return o_size - size;
        if (*ep == '\0')
            ep = encstr;
        size--;
    }

    return o_size - size;
}

/*
 * perform an encrypted read
 */
int encread(void *starta, unsigned int size, int inf)
{
    char *ep;
    int read_size;
    char *start = starta;

    if ((read_size = read(inf, start, size)) == -1 || read_size == 0)
        return read_size;

    ep = encstr;

    while (size--) {
        *start++ ^= *ep++;
        if (*ep == '\0')
            ep = encstr;
    }
    return read_size;
}
