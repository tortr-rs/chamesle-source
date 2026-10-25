/* tests c.h: run it, every line says ok or what went wrong */
#include "../c.h"

static int bad;
static void check(const char *what, const char *got, const char *want)
{
    if (strcmp(got, want)) { printf("FAIL %s: got [%s] want [%s]\n", what, got, want); bad++; }
}
static int cmp(const void *a, const void *b) { return *(int *)a - *(int *)b; }

int main(void)
{
    char b[256];
    snprintf(b, sizeof b, "%d|%5d|%-5d|%05d|%+d|%.3d", 42, 42, 42, -42, 7, 5); check("ints", b, "42|   42|42   |-0042|+7|005");
    snprintf(b, sizeof b, "%x|%X|%#x|%o|%#o|%lu|%lld|%zu", 255, 255, 255, 8, 8, 4000000000UL, -9000000000LL, (size_t)3); check("unsigned", b, "ff|FF|0xff|10|010|4000000000|-9000000000|3");
    snprintf(b, sizeof b, "%s|%10s|%-4s|%.2s|%c|%%", "hi", "right", "l", "cut", 'z'); check("strings", b, "hi|     right|l   |cu|z|%");
    snprintf(b, sizeof b, "%f|%.2f|%e|%g|%g|%g", 3.14159, 2.005, 12345.678, 0.0001, 1e20, 100.0); check("floats", b, "3.141590|2.00|1.234568e+04|0.0001|1e+20|100");
    snprintf(b, sizeof b, "%d", snprintf(NULL, 0, "%s", "12345")); check("length", b, "5");
    snprintf(b, sizeof b, "%ld %lu %ld", strtol("-0x1f", 0, 0), strtoul("0777", 0, 0), strtol("  99z", 0, 10)); check("strtol", b, "-31 511 99");
    snprintf(b, sizeof b, "%.6f %.1f %g", strtod("1.5e3", 0), strtod("0x1.8p1", 0), strtod(".25", 0)); check("strtod", b, "1500.000000 3.0 0.25");
    int v[] = { 5, 3, 9, 1, 7, 2 }; qsort(v, 6, sizeof *v, cmp);
    snprintf(b, sizeof b, "%d%d%d%d%d%d", v[0], v[1], v[2], v[3], v[4], v[5]); check("qsort", b, "123579");
    jmp_buf j; volatile int n = 0;
    if (setjmp(j) < 3) { n++; longjmp(j, n); }
    snprintf(b, sizeof b, "%d", n); check("setjmp", b, "3");
    FILE *f = fopen("/tmp/c-h-test", "w"); fprintf(f, "line one\nline %d\n", 2); fclose(f);
    f = fopen("/tmp/c-h-test", "r"); fgets(b, sizeof b, f); check("fgets", b, "line one\n"); fgets(b, sizeof b, f); check("fgets 2", b, "line 2\n"); fclose(f); unlink("/tmp/c-h-test");
    check("realpath", realpath("/a/./b/../c//d", b), "/a/c/d");
    time_t t = 951782400; struct tm *tm = localtime(&t); snprintf(b, sizeof b, "%d-%d-%d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday); check("time", b, "2000-2-29");
    char *m = malloc(10); strcpy(m, "grow"); m = realloc(m, 10000); check("realloc", m, "grow"); free(m);
    printf(bad ? "%d failed\n" : "all ok\n", bad);
    return bad;
}
