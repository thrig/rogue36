/* mkencstr - makes an "encryption" key for vers.c */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ENCSTR_LEN 42

uint8_t encstr[ENCSTR_LEN];

int main(void)
{
    arc4random_buf(encstr, ENCSTR_LEN);
    for (int i = 0; i < ENCSTR_LEN; i++) {
        if (isprint(encstr[i]) && encstr[i] != '"')
            putchar(encstr[i]);
        else
            printf("\\%03o", encstr[i]);
    }
    exit(EXIT_SUCCESS);
}
