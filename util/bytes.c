#include <stdint.h>

uint16_t pack_u16(uint8_t lo, uint8_t hi) {
    return lo | (hi << 8);
}

void unpack_u16(uint16_t val, uint8_t *arr) {
    arr[0] = val & 0xFF;
    arr[1] = val >> 8;
}

uint8_t byte_reverse(uint8_t ch) {
    uint8_t res = 0;
    for (int i = 0; i < 8; i++) {
        res <<= 1;
        res |= (ch & (1 << i)) >> i;
    }
    return res;
}

const char *byte_to_binary_strcat(int x) {
    char b[9];
    b[0] = '\0';
    for (int z = 128; z > 0; z >>= 1)
        strcat(b, ((x & z) == z) ? "1" : "0");
    return b;
}

const char *byte_to_binary(int x) {
    char b[9];
    int i = 0;
    for (int z = 128; z > 0; z >>= 1)
        b[i++] = ((x & z) == z) ? '1' : '0';
    b[8] = '\0';
    return b;
}

int atoi_n(char *str, int len) {
    int n = 0;
    int mul = 1;
    char *ptr = str + len - 1;
    while (ptr >= str) {
        n += (*(ptr--) - '0') * mul;
        mul *= 10;
    }
    return n;
}

int atoi(char *str) {
    int len = strlen(str);
    return atoi_n(str, len);
}

void itoa(int n, char *buf) {
    int i = 0;
    //do buf[i++] = "0123456789"[n % 10];
    do buf[i++] = '0' + n % 10;
    while (n /= 10);
    /* reverse string and null-terminate */
    char *end = &buf[i - 1];
    for (char *ptr = buf; ptr < end; ptr++) {
        char tmp = *ptr;
        *ptr = *end;
        *end = tmp;
        end--;
    }
    buf[i] = '\0';
}
