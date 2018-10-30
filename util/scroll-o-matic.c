/* a traditional scroll name generator */

#include "rogue.h"

#define MAXSCROLLS 16

int seed;
char prbuf[80];
char *sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
    "dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
    "esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
    "glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
    "nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
    "ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
    "sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
    "ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
    "wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum",
};

int main(int argc, char *argv[])
{
    char *cp, *sp;
    int nsyl, nwords;
    long count = MAXSCROLLS, i;
    time_t now;

    if (argc == 1) {
        time(&now);
        seed = (int) now + getpid();
    } else {
        argv++;
        if (argc == 2) {
            seed = (int) argtol(*argv, (long) INT_MIN, (long) INT_MAX);
        } else if (argc == 3) {
            count = argtol(*argv, 1, (long) INT_MAX);
            argv++;
            seed = (int) argtol(*argv, (long) INT_MIN, (long) INT_MAX);
        } else {
            fprintf(stderr, "Usage: scroll-o-matic [ seed | count seed ]\n");
        }
    }

    for (i = 0; i < count; i++) {
        cp = prbuf;
        nwords = rnd(4) + 2;
        while (nwords--) {
            nsyl = rnd(3) + 1;
            while (nsyl--) {
                sp = sylls[rnd((sizeof sylls) / (sizeof(char *)))];
                while (*sp)
                    *cp++ = *sp++;
            }
            *cp++ = ' ';
        }
        *--cp = '\0';
        puts(prbuf);
    }
    exit(0);
}
