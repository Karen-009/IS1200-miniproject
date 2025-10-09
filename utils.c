#include <stdint.h>

static unsigned int seed = 1;

// Minimal random generator (simple LCG)
int rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

void srand(unsigned int s) {
    seed = s;
}

// Simple memset
void *memset(void *s, int c, unsigned int n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

// Simple memcpy
void *memcpy(void *dest, const void *src, unsigned int n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

// Dummy printf (does nothing)
int printf(const char *fmt, ...) {
    return 0;
}

// Dummy time() – return a changing value if you want randomness
unsigned int time(unsigned int *t) {
    static unsigned int fake_time = 1;
    fake_time += 12345;
    if (t) *t = fake_time;
    return fake_time;
}

// Absolute value function
int abs(int n) {
    return (n < 0) ? -n : n;
}

void handle_exception(void) { while(1); }