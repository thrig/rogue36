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
extern int revision_num;

static char *encsp;

int dosave(FILE * savef);

void reset_encstr(void)
{
    encsp = encstr;
}

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
    int ret;

    wmove(cw, ROLINES - 1, 0);
    draw(cw);

    fputs(version, savef);
    fprintf(savef, "\n%08d\n", revision_num);
    reset_encstr();
    ret = rs_save_file(savef);
    fclose(savef);
    return ret;
}

inline void restore(void)
{
    int inf, read_size, len;
    char buf[ROGUE_CHARBUF_MAX];
    int savefile_version = 0;

    if ((inf = open(save_file, O_RDONLY)) < 0) {
        endwin();
        err(1, "open failed");
    }

    len = strlen(version);
    read_size = read(inf, buf, len);
    if (read_size != len || strcmp(buf, version) != 0) {
        endwin();
        errx(1, "unknown save game version");
    }

    read_size = read(inf, buf, 10);
    if (read_size != 10) {
        endwin();
        errx(1, "unknown save game revision");
    }
    sscanf(buf, "\n%8d\n", &savefile_version);

    if (savefile_version != revision_num) {
        endwin();
        errx(1, "save game format is out of date");
    }

    reset_encstr();
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
    char *start = starta;
    unsigned int o_size = size;

    while (size) {
        if (putc(*start++ ^ *encsp++, outf) == EOF)
            return o_size - size;
        if (*encsp == '\0')
            encsp = encstr;
        size--;
    }

    return o_size - size;
}

/*
 * perform an encrypted read
 */
size_t encread(void *starta, unsigned int size, int inf)
{
    ssize_t read_size;
    char *start = starta;

    if ((read_size = read(inf, start, size)) < 1)
        return 0;

    while (size--) {
        *start++ ^= *encsp++;
        if (*encsp == '\0')
            encsp = encstr;
    }
    return read_size;
}
