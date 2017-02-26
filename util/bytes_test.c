#!/usr/bin/tcc -run
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bytes.c"

int main() {
    /* bit fiddling */
    uint8_t b = 0b11010010;

    printf ("original:   %s\n", byte_to_binary(b));
    printf ("b2b:        %s\n", byte_to_binary(b));
    printf ("complement: %s\n", byte_to_binary(~b));
    printf ("reverse:    %s\n", byte_to_binary(byte_reverse(b)));
    printf("\n");

    /* atoi/itoa functions */
    char buf[11];
    itoa(67813, buf);
    printf("buf: %s \n", buf);
    printf("atoi: %d\n", atoi_n("1234567", strlen("1234567")));

    return 0;
}
