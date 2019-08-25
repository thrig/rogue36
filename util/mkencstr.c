/* mkencstr - makes an "encryption" key for vers.c */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ENCSTR_LEN 42

int main(void)
{
    for (int i = 0; i < ENCSTR_LEN; i++) {
        int x = 1 + arc4random_uniform(255);
        if (isprint(x) && x != '"')
            putchar(x);
        else
            printf("\\%03o", x);
    }
    exit(EXIT_SUCCESS);
}
