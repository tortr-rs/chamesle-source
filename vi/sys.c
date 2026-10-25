/* sys.c: everything under ex, in one piece because c.h, the whole libc, can only be compiled once into a program:
 * c.h, tcc as a library (third/tinycc), what ex expected from V6 Unix (v6.c), and :wq (wq.c) */
#define C_NO_START
/* ex has its own of these: c.h's, and tcc's calls to them, get other names */
#define strcpy c_strcpy
#define strcat c_strcat
#define strlen c_strlen
#define strcmp c_strcmp
#define putchar c_putchar
#define getchar c_getchar
#define printf c_printf
#include "../c.h"
#include "../third/tinycc/libtcc.c"
#undef strcpy
#undef strcat
#undef strlen
#undef strcmp
#undef putchar
#undef getchar
#undef printf

#include "v6.c"
#include "wq.c"
