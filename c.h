/* c.h: the whole libc of Chamesle Source Linux, in one file. x86-64 Linux only.
 * include it once (in one .c file) and you get everything: the start-up code, system calls, strings, memory,
 * files and printf. it builds with tcc (your programs) and gcc (vi and tcc themselves). */
#ifndef C_H
#define C_H

/* ---- types ---- */
typedef unsigned long size_t;
typedef long ssize_t;
typedef long ptrdiff_t;
typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef long off_t;
typedef long time_t;
typedef int pid_t;
typedef int mode_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef long long intmax_t;
typedef unsigned long long uintmax_t;
typedef int wchar_t_c; /* not wchar_t: tcc and gcc both predefine their own */
#define NULL ((void *)0)
#define offsetof(t, m) ((size_t)&((t *)0)->m)
#define CHAR_BIT 8
#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX 4294967295U
#define LONG_MAX 9223372036854775807L
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX 18446744073709551615UL
#define LLONG_MAX LONG_MAX
#define LLONG_MIN LONG_MIN
#define ULLONG_MAX ULONG_MAX
#define PATH_MAX 4096
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, t) __builtin_va_arg(ap, t)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(d, s) __builtin_va_copy(d, s)

/* ---- system calls ----
 * sys6(n, a..f): the number goes in rax, the arguments in rdi rsi rdx r10 r8 r9. written in plain asm so it
 * builds the same under gcc and tcc. returns -errno on failure, like the kernel */
long sys6(long n, long a, long b, long c, long d, long e, long f);
__asm__(".text\n.global sys6\nsys6:\n"
        " mov %rdi,%rax\n mov %rsi,%rdi\n mov %rdx,%rsi\n mov %rcx,%rdx\n mov %r8,%r10\n mov %r9,%r8\n"
        " mov 8(%rsp),%r9\n syscall\n ret\n");
#define sys0(n) sys6(n, 0, 0, 0, 0, 0, 0)
#define sys1(n, a) sys6(n, (long)(a), 0, 0, 0, 0, 0)
#define sys2(n, a, b) sys6(n, (long)(a), (long)(b), 0, 0, 0, 0)
#define sys3(n, a, b, c) sys6(n, (long)(a), (long)(b), (long)(c), 0, 0, 0)

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_lseek 8
#define SYS_mmap 9
#define SYS_mprotect 10
#define SYS_munmap 11
#define SYS_ioctl 16
#define SYS_fork 57
#define SYS_execve 59
#define SYS_exit 60
#define SYS_wait4 61
#define SYS_getcwd 79
#define SYS_mkdir 83
#define SYS_unlink 87
#define SYS_mount 165
#define SYS_time 201
#define SYS_gettimeofday 96
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_brk 12
#define SYS_rt_sigaction 13
#define SYS_rt_sigreturn 15
#define SYS_getpid 39
#define SYS_chdir 80
#define SYS_creat 85
#define SYS_chmod 90
#define SYS_pipe 22
#define SYS_dup 32
#define SYS_getuid 102

/* errno: the libc keeps the last error here. one program, one thread: a plain variable does it */
int errno;
#define ENOENT 2
#define EIO 5
#define EBADF 9
#define ENOMEM 12
#define EACCES 13
#define EEXIST 17
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ERANGE 34
static long fail(long r) { if (r < 0 && r > -4096) { errno = -r; return -1; } return r; }

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_APPEND 02000
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define _SC_PAGESIZE 30

ssize_t read(int fd, void *buf, size_t n) { return fail(sys3(SYS_read, fd, buf, n)); }
ssize_t write(int fd, const void *buf, size_t n) { return fail(sys3(SYS_write, fd, buf, n)); }
int open(const char *path, int flags, ...)
{
    va_list ap; int mode;
    va_start(ap, flags); mode = va_arg(ap, int); va_end(ap);
    return fail(sys3(SYS_open, path, flags, flags & O_CREAT ? mode : 0));
}
int close(int fd) { return fail(sys1(SYS_close, fd)); }
off_t lseek(int fd, off_t off, int whence) { return fail(sys3(SYS_lseek, fd, off, whence)); }
int ioctl(int fd, unsigned long req, void *arg) { return fail(sys3(SYS_ioctl, fd, req, arg)); }
int mprotect(void *p, size_t n, int prot) { return fail(sys3(SYS_mprotect, p, n, prot)); }
int unlink(const char *path) { return fail(sys1(SYS_unlink, path)); }
int mkdir(const char *path, mode_t mode) { return fail(sys2(SYS_mkdir, path, mode)); }
pid_t fork(void) { return fail(sys0(SYS_fork)); }
int execve(const char *path, char *const argv[], char *const envp[]) { return fail(sys3(SYS_execve, path, argv, envp)); }
pid_t waitpid(pid_t pid, int *status, int options) { return fail(sys6(SYS_wait4, pid, (long)status, options, 0, 0, 0)); }
int mount(const char *src, const char *dst, const char *type, unsigned long flags, const void *data)
{
    return fail(sys6(SYS_mount, (long)src, (long)dst, (long)type, flags, (long)data, 0));
}
char *getcwd(char *buf, size_t n) { return fail(sys2(SYS_getcwd, buf, n)) < 0 ? NULL : buf; }
time_t time(time_t *t) { time_t now = sys1(SYS_time, 0); if (t) *t = now; return now; }
long sysconf(int name) { return name == _SC_PAGESIZE ? 4096 : -1; }
int creat(const char *path, mode_t mode) { return fail(sys2(SYS_creat, path, mode)); }
int chdir(const char *path) { return fail(sys1(SYS_chdir, path)); }
int chmod(const char *path, mode_t mode) { return fail(sys2(SYS_chmod, path, mode)); }
int dup(int fd) { return fail(sys1(SYS_dup, fd)); }
int pipe(int fd[2]) { return fail(sys1(SYS_pipe, fd)); }
pid_t getpid(void) { return sys0(SYS_getpid); }
int getuid(void) { return sys0(SYS_getuid); }
pid_t wait(int *status) { return waitpid(-1, status, 0); }

/* the end of the data segment, the "break". brk(p) moves it to p, sbrk(n) moves it n bytes and returns where it was */
static char *cur_brk;
int brk(void *p)
{
    char *r = (char *)sys1(SYS_brk, p);
    cur_brk = r;
    if (r != p) { errno = ENOMEM; return -1; }
    return 0;
}
void *sbrk(long n)
{
    char *old;
    if (!cur_brk) cur_brk = (char *)sys1(SYS_brk, 0);
    old = cur_brk;
    if (n && brk(old + n) < 0) return (void *)-1;
    return old;
}

/* ---- files' details ---- */
struct stat {
    unsigned long st_dev, st_ino, st_nlink;
    unsigned int st_mode, st_uid, st_gid, pad0;
    unsigned long st_rdev;
    long st_size, st_blksize, st_blocks;
    unsigned long st_atime, st_atime_nsec, st_mtime, st_mtime_nsec, st_ctime, st_ctime_nsec;
    long unused[3];
};
#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFREG 0100000
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
int stat(const char *path, struct stat *st) { return fail(sys2(SYS_stat, path, st)); }
int fstat(int fd, struct stat *st) { return fail(sys2(SYS_fstat, fd, st)); }

/* ---- the terminal ---- */
struct termios { unsigned int c_iflag, c_oflag, c_cflag, c_lflag; unsigned char c_line, c_cc[19]; };
#define TCGETS 0x5401
#define TCSETSW 0x5403
#define VINTR 0
#define VQUIT 1
#define VERASE 2
#define VKILL 3
#define VEOF 4
#define VTIME 5
#define VMIN 6
#define ICRNL 0400
#define IXON 02000
#define OPOST 01
#define ONLCR 04
#define TAB3 014000
#define ISIG 01
#define ICANON 02
#define ECHO 010
#define IEXTEN 0100000
int tcgetattr(int fd, struct termios *t) { return ioctl(fd, TCGETS, t); }
int tcsetattr(int fd, int when, struct termios *t) { (void)when; return ioctl(fd, TCSETSW, t); }
int isatty(int fd) { struct termios t; return tcgetattr(fd, &t) == 0; }

/* ---- signals ----
 * signal(n, f): f runs when signal n comes. SIG_DFL (0) puts back what the kernel does, SIG_IGN (1) ignores it.
 * the kernel needs a way back from f: restore_rt, which asks it to carry on where the signal hit */
typedef void (*sighandler_t)(int);
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGKILL 9
#define SIGTERM 15
#define SIGCHLD 17
void c_restore_rt(void);
__asm__(".text\n.global c_restore_rt\nc_restore_rt:\n mov $15,%rax\n syscall\n");
struct c_sigaction { sighandler_t handler; unsigned long flags; void (*restorer)(void); unsigned long mask; };
sighandler_t signal(int n, sighandler_t f)
{
    struct c_sigaction new = { f, 0x04000000 | 0x10000000 /* restorer, restart */, c_restore_rt, 0 }, old;
    if (fail(sys6(SYS_rt_sigaction, n, (long)&new, (long)&old, 8, 0, 0)) < 0) return SIG_ERR;
    return old.handler;
}
__attribute__((noreturn)) void _exit(int code) { for (;;) sys1(SYS_exit, code); }

/* ---- characters ---- */
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalpha(int c) { return (c | 32) >= 'a' && (c | 32) <= 'z'; }
int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isspace(int c) { return c == ' ' || (c >= '\t' && c <= '\r'); }
int isupper(int c) { return c >= 'A' && c <= 'Z'; }
int islower(int c) { return c >= 'a' && c <= 'z'; }
int isxdigit(int c) { return isdigit(c) || ((c | 32) >= 'a' && (c | 32) <= 'f'); }
int isprint(int c) { return c >= 32 && c < 127; }
int toupper(int c) { return islower(c) ? c - 32 : c; }
int tolower(int c) { return isupper(c) ? c + 32 : c; }

/* ---- strings ---- */
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) a++, b++; return (unsigned char)*a - (unsigned char)*b; }
int strncmp(const char *a, const char *b, size_t n)
{
    for (; n && *a && *a == *b; n--) a++, b++;
    return n ? (unsigned char)*a - (unsigned char)*b : 0;
}
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)); return r; }
char *strncpy(char *d, const char *s, size_t n) { size_t i = 0; for (; i < n && s[i]; i++) d[i] = s[i]; for (; i < n; i++) d[i] = 0; return d; }
char *strcat(char *d, const char *s) { strcpy(d + strlen(d), s); return d; }
char *strchr(const char *s, int c) { for (;; s++) { if (*s == (char)c) return (char *)s; if (!*s) return NULL; } }
char *strrchr(const char *s, int c) { const char *r = NULL; for (;; s++) { if (*s == (char)c) r = s; if (!*s) return (char *)r; } }
char *strstr(const char *h, const char *n)
{
    size_t l = strlen(n);
    for (; *h; h++) if (!strncmp(h, n, l)) return (char *)h;
    return l ? NULL : (char *)h;
}
char *strpbrk(const char *s, const char *set) { for (; *s; s++) if (strchr(set, *s)) return (char *)s; return NULL; }
void *memcpy(void *d, const void *s, size_t n) { char *a = d; const char *b = s; while (n--) *a++ = *b++; return d; }
void *memmove(void *d, const void *s, size_t n)
{
    char *a = d; const char *b = s;
    if (a < b) while (n--) *a++ = *b++;
    else while (n--) a[n] = b[n];
    return d;
}
void *memset(void *d, int c, size_t n) { char *a = d; while (n--) *a++ = c; return d; }
int memcmp(const void *x, const void *y, size_t n)
{
    const unsigned char *a = x, *b = y;
    for (; n; n--, a++, b++) if (*a != *b) return *a - *b;
    return 0;
}
void *memchr(const void *s, int c, size_t n) { const unsigned char *p = s; for (; n; n--, p++) if (*p == (unsigned char)c) return (void *)p; return NULL; }
char *strerror(int e)
{
    switch (e) {
    case ENOENT: return "no such file or directory";
    case EIO: return "input/output error";
    case EBADF: return "bad file descriptor";
    case ENOMEM: return "out of memory";
    case EACCES: return "permission denied";
    case EEXIST: return "file exists";
    case ENOTDIR: return "not a directory";
    case EISDIR: return "is a directory";
    case EINVAL: return "invalid argument";
    case ERANGE: return "out of range";
    }
    return "error";
}

/* ---- memory ----
 * every block straight from mmap, its size kept in front of it. never fragments, wastes a bit for small blocks */
void *malloc(size_t n)
{
    size_t len = (n + 16 + 4095) & ~4095UL;
    long p = sys6(SYS_mmap, 0, len, PROT_READ | PROT_WRITE, 0x22 /* private, anonymous */, -1, 0);
    if (p < 0 && p > -4096) { errno = ENOMEM; return NULL; }
    *(size_t *)p = len;
    return (char *)p + 16; /* 16: what a long double or an SSE value wants */
}
void free(void *p) { if (p) { char *h = (char *)p - 16; sys2(SYS_munmap, h, *(size_t *)h); } }
void *calloc(size_t n, size_t m) { return malloc(n * m); } /* mmap memory comes zeroed */
void *realloc(void *p, size_t n)
{
    if (!p) return malloc(n);
    size_t old = *(size_t *)((char *)p - 16) - 16;
    if (n <= old) return p;
    void *q = malloc(n);
    if (q) { memcpy(q, p, old); free(p); }
    return q;
}
char *strdup(const char *s) { char *d = malloc(strlen(s) + 1); return d ? strcpy(d, s) : NULL; }

/* ---- numbers from text ---- */
static int digit_of(int c) { return isdigit(c) ? c - '0' : isalpha(c) ? (c | 32) - 'a' + 10 : 99; }
unsigned long long strtoull(const char *s, char **end, int base)
{
    const char *p = s; unsigned long long v = 0; int neg = 0, any = 0;
    while (isspace(*p)) p++;
    if (*p == '-' || *p == '+') neg = *p++ == '-';
    if ((base == 0 || base == 16) && p[0] == '0' && (p[1] | 32) == 'x' && isxdigit(p[2])) { p += 2; base = 16; }
    else if (base == 0) base = *p == '0' ? 8 : 10;
    for (; digit_of(*p) < base; p++, any = 1) v = v * base + digit_of(*p);
    if (end) *end = (char *)(any ? p : s);
    return neg ? -v : v;
}
long long strtoll(const char *s, char **end, int base) { return (long long)strtoull(s, end, base); }
unsigned long strtoul(const char *s, char **end, int base) { return strtoull(s, end, base); }
long strtol(const char *s, char **end, int base) { return strtoull(s, end, base); }
int atoi(const char *s) { return strtol(s, NULL, 10); }
long atol(const char *s) { return strtol(s, NULL, 10); }

long double ldexpl(long double x, int e)
{
    while (e > 0) { x *= 2; e--; }
    while (e < 0) { x /= 2; e++; }
    return x;
}
double ldexp(double x, int e) { return ldexpl(x, e); }
/* decimal and hex floating point. good to the last digit or so, not correctly rounded in every case */
long double strtold(const char *s, char **end)
{
    const char *p = s; long double v = 0; int neg = 0, any = 0, ex = 0;
    while (isspace(*p)) p++;
    if (*p == '-' || *p == '+') neg = *p++ == '-';
    if (p[0] == '0' && (p[1] | 32) == 'x') {
        p += 2;
        for (; isxdigit(*p); p++, any = 1) v = v * 16 + digit_of(*p);
        if (*p == '.') for (p++; isxdigit(*p); p++, any = 1) { v = v * 16 + digit_of(*p); ex -= 4; }
        if (any && (*p | 32) == 'p') { char *e2; long x = strtol(p + 1, &e2, 10); if (e2 != p + 1) { ex += x; p = e2; } }
        v = ldexpl(v, ex);
    } else {
        for (; isdigit(*p); p++, any = 1) v = v * 10 + (*p - '0');
        if (*p == '.') for (p++; isdigit(*p); p++, any = 1) { v = v * 10 + (*p - '0'); ex--; }
        if (any && (*p | 32) == 'e') { char *e2; long x = strtol(p + 1, &e2, 10); if (e2 != p + 1) { ex += x; p = e2; } }
        for (; ex > 0; ex--) v *= 10;
        for (; ex < 0; ex++) v /= 10;
    }
    if (end) *end = (char *)(any ? p : s);
    return neg ? -v : v;
}
double strtod(const char *s, char **end) { return strtold(s, end); }
float strtof(const char *s, char **end) { return strtold(s, end); }
double atof(const char *s) { return strtod(s, NULL); }

/* ---- sorting ---- */
static void swap_bytes(char *a, char *b, size_t n) { while (n--) { char t = *a; *a++ = *b; *b++ = t; } }
void qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    /* shell sort: short, no recursion, fine for the sizes a compiler sorts */
    char *b = base; size_t gap, i, j;
    for (gap = n / 2; gap; gap /= 2)
        for (i = gap; i < n; i++)
            for (j = i; j >= gap && cmp(b + (j - gap) * size, b + j * size) > 0; j -= gap)
                swap_bytes(b + (j - gap) * size, b + j * size, size);
}

/* ---- jumps ---- */
typedef long jmp_buf[8];
int setjmp(jmp_buf b);
__attribute__((noreturn)) void longjmp(jmp_buf b, int v);
__asm__(".text\n.global setjmp\n.global _setjmp\nsetjmp:\n_setjmp:\n"
        " mov %rbx,(%rdi)\n mov %rbp,8(%rdi)\n mov %r12,16(%rdi)\n mov %r13,24(%rdi)\n mov %r14,32(%rdi)\n"
        " mov %r15,40(%rdi)\n lea 8(%rsp),%rdx\n mov %rdx,48(%rdi)\n mov (%rsp),%rdx\n mov %rdx,56(%rdi)\n"
        " xor %eax,%eax\n ret\n"
        ".global longjmp\nlongjmp:\n"
        " mov %esi,%eax\n test %eax,%eax\n jnz 1f\n inc %eax\n1:\n"
        " mov (%rdi),%rbx\n mov 8(%rdi),%rbp\n mov 16(%rdi),%r12\n mov 24(%rdi),%r13\n mov 32(%rdi),%r14\n"
        " mov 40(%rdi),%r15\n mov 48(%rdi),%rsp\n jmp *56(%rdi)\n");

/* ---- time ---- */
struct tm { int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst; };
/* there are no time zones here: local time is UTC */
struct tm *localtime(const time_t *t)
{
    static struct tm r;
    long s = *t, days = s / 86400, rem = s % 86400, y = 1970;
    static const int md[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    r.tm_hour = rem / 3600; r.tm_min = rem / 60 % 60; r.tm_sec = rem % 60;
    r.tm_wday = (days + 4) % 7;
    for (;;) {
        int len = (y % 4 == 0 && (y % 100 || y % 400 == 0)) ? 366 : 365;
        if (days < len) break;
        days -= len; y++;
    }
    r.tm_year = y - 1900; r.tm_yday = days;
    for (r.tm_mon = 0; ; r.tm_mon++) {
        int len = md[r.tm_mon] + (r.tm_mon == 1 && (y % 4 == 0 && (y % 100 || y % 400 == 0)));
        if (days < len) break;
        days -= len;
    }
    r.tm_mday = days + 1; r.tm_isdst = 0;
    return &r;
}
struct tm *gmtime(const time_t *t) { return localtime(t); }
struct timeval { long tv_sec, tv_usec; };
int gettimeofday(struct timeval *tv, void *tz) { (void)tz; return fail(sys2(SYS_gettimeofday, tv, 0)); }

/* ---- files with buffers ---- */
typedef struct FILE { int fd, write, err, eof, n, pos; char buf[4096]; } FILE;
static FILE files_std[3] = { { 0 }, { 1, 1 }, { 2, 1 } };
FILE *stdin = &files_std[0], *stdout = &files_std[1], *stderr = &files_std[2];
#define EOF (-1)
#define BUFSIZ 4096

int fflush(FILE *f)
{
    if (f && f->write && f->n) {
        int off = 0;
        while (off < f->n) { ssize_t w = write(f->fd, f->buf + off, f->n - off); if (w <= 0) { f->err = 1; break; } off += w; }
        f->n = 0;
    }
    return f && f->err ? EOF : 0;
}
static int open_flags(const char *mode)
{
    int f = mode[0] == 'r' ? O_RDONLY : mode[0] == 'w' ? O_WRONLY | O_CREAT | O_TRUNC : O_WRONLY | O_CREAT | O_APPEND;
    if (strchr(mode, '+')) f = (f & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
    return f;
}
FILE *fdopen(int fd, const char *mode)
{
    FILE *f = calloc(1, sizeof *f);
    if (!f) return NULL;
    f->fd = fd; f->write = mode[0] != 'r' || strchr(mode, '+');
    return f;
}
FILE *fopen(const char *path, const char *mode)
{
    int fd = open(path, open_flags(mode), 0666);
    return fd < 0 ? NULL : fdopen(fd, mode);
}
FILE *freopen(const char *path, const char *mode, FILE *f)
{
    int fd;
    fflush(f);
    if (!path) return f;
    if ((fd = open(path, open_flags(mode), 0666)) < 0) return NULL;
    close(f->fd);
    f->fd = fd; f->write = mode[0] != 'r' || strchr(mode, '+'); f->n = f->pos = f->err = f->eof = 0;
    return f;
}
int fclose(FILE *f)
{
    int r = fflush(f);
    close(f->fd);
    if (f < files_std || f > files_std + 2) free(f);
    return r;
}
size_t fwrite(const void *p, size_t size, size_t n, FILE *f)
{
    const char *s = p; size_t len = size * n, i;
    for (i = 0; i < len; i++) {
        f->buf[f->n++] = s[i];
        if (f->n == BUFSIZ || (f == stdout && s[i] == '\n') || f == stderr) fflush(f);
    }
    if (f == stderr) fflush(f);
    return f->err ? 0 : n;
}
int fputc(int c, FILE *f) { unsigned char ch = c; return fwrite(&ch, 1, 1, f) ? ch : EOF; }
int putc(int c, FILE *f) { return fputc(c, f); }
int putchar(int c) { return fputc(c, stdout); }
int fputs(const char *s, FILE *f) { return fwrite(s, 1, strlen(s), f) ? 0 : EOF; }
int puts(const char *s) { fputs(s, stdout); return putchar('\n'); }
int fgetc(FILE *f)
{
    if (f->pos == f->n) {
        if (f == stdin) fflush(stdout);
        ssize_t r = read(f->fd, f->buf, BUFSIZ);
        if (r <= 0) { if (r < 0) f->err = 1; else f->eof = 1; f->n = f->pos = 0; return EOF; }
        f->n = r; f->pos = 0;
    }
    return (unsigned char)f->buf[f->pos++];
}
int getc(FILE *f) { return fgetc(f); }
int getchar(void) { return fgetc(stdin); }
size_t fread(void *p, size_t size, size_t n, FILE *f)
{
    char *d = p; size_t len = size * n, i; int c;
    for (i = 0; i < len && (c = fgetc(f)) != EOF; i++) d[i] = c;
    return size ? i / size : 0;
}
char *fgets(char *s, int n, FILE *f)
{
    int i = 0, c;
    while (i < n - 1 && (c = fgetc(f)) != EOF) { s[i++] = c; if (c == '\n') break; }
    s[i] = 0;
    return i ? s : NULL;
}
int feof(FILE *f) { return f->eof; }
int ferror(FILE *f) { return f->err; }
int fileno(FILE *f) { return f->fd; }
int fseek(FILE *f, long off, int whence)
{
    fflush(f);
    if (whence == SEEK_CUR && !f->write) off -= f->n - f->pos;
    f->n = f->pos = 0; f->eof = 0;
    return lseek(f->fd, off, whence) < 0 ? -1 : 0;
}
long ftell(FILE *f)
{
    long p = lseek(f->fd, 0, SEEK_CUR);
    return p < 0 ? -1 : f->write ? p + f->n : p - (f->n - f->pos);
}
int remove(const char *path) { return unlink(path); }

/* ---- printf ----
 * flags - + space # 0, width and precision (numbers or *), sizes hh h l ll z j t L,
 * conversions d i u o x X c s p f F e E g G and %% */
struct out { char *buf; size_t cap, len; FILE *f; };
static void out_c(struct out *o, char c)
{
    if (o->f) fputc(c, o->f);
    else if (o->len + 1 < o->cap) o->buf[o->len] = c;
    o->len++;
}
static void out_pad(struct out *o, char c, int n) { while (n-- > 0) out_c(o, c); }
static void out_field(struct out *o, const char *pre, const char *s, int len, int width, int left, int zero)
{
    int plen = strlen(pre), pad = width - plen - len;
    if (!left && !zero) out_pad(o, ' ', pad);
    while (*pre) out_c(o, *pre++);
    if (!left && zero) out_pad(o, '0', pad);
    while (len-- > 0) out_c(o, *s++);
    if (left) out_pad(o, ' ', pad);
}
/* a long double as digits: fixed (f), exponent (e) or the shorter of both (g) */
static int fmt_float(char *b, long double v, int prec, char conv, int alt)
{
    int n = 0, e = 0, i, upper = conv == 'E' || conv == 'G' || conv == 'F';
    char c = conv | 32;
    if (v != v) { strcpy(b, upper ? "NAN" : "nan"); return 3; }
    if (v > 1e4900L || v < -1e4900L) { strcpy(b, upper ? "INF" : "inf"); return 3; }
    if (prec < 0) prec = 6;
    if (v != 0) { long double a = v < 0 ? -v : v; while (a >= 10) { a /= 10; e++; } while (a < 1) { a *= 10; e--; } }
    if (c == 'g') {
        if (prec == 0) prec = 1;
        c = (e < -4 || e >= prec) ? 'e' : 'f';
        prec = c == 'e' ? prec - 1 : prec - 1 - e;
        if (prec < 0) prec = 0;
        if (!alt) alt = -1; /* -1: drop the trailing zeros */
    }
    long double a = v < 0 ? -v : v, r = 0.5L;
    if (c == 'e') { if (v != 0) { for (i = 0; i < e; i++) a /= 10; for (i = 0; i > e; i--) a *= 10; } }
    for (i = 0; i < prec; i++) r /= 10;
    a += r;
    if (c == 'e' && a >= 10) { a /= 10; e++; }
    long double ip = 1; int idig = 1;
    while (ip * 10 <= a) { ip *= 10; idig++; }
    for (i = 0; i < idig; i++) { int d = (int)(a / ip); if (d > 9) d = 9; b[n++] = '0' + d; a -= d * ip; ip /= 10; }
    if (prec > 0 || alt > 0) b[n++] = '.';
    for (i = 0; i < prec; i++) { a *= 10; int d = (int)a; if (d > 9) d = 9; b[n++] = '0' + d; a -= d; }
    if (alt < 0 && prec > 0) { while (b[n - 1] == '0') n--; if (b[n - 1] == '.') n--; }
    if (c == 'e') {
        b[n++] = upper ? 'E' : 'e'; b[n++] = e < 0 ? '-' : '+';
        if (e < 0) e = -e;
        if (e >= 100) b[n++] = '0' + e / 100;
        b[n++] = '0' + e / 10 % 10; b[n++] = '0' + e % 10;
    }
    b[n] = 0;
    return n;
}
static int format(struct out *o, const char *fmt, va_list ap)
{
    for (; *fmt; fmt++) {
        if (*fmt != '%') { out_c(o, *fmt); continue; }
        int left = 0, plus = 0, space = 0, alt = 0, zero = 0, width = 0, prec = -1, size = 0;
        for (;; fmt++) {
            if (fmt[1] == '-') left = 1; else if (fmt[1] == '+') plus = 1; else if (fmt[1] == ' ') space = 1;
            else if (fmt[1] == '#') alt = 1; else if (fmt[1] == '0') zero = 1; else break;
        }
        fmt++;
        if (*fmt == '*') { width = va_arg(ap, int); if (width < 0) { left = 1; width = -width; } fmt++; }
        else while (isdigit(*fmt)) width = width * 10 + *fmt++ - '0';
        if (*fmt == '.') {
            fmt++; prec = 0;
            if (*fmt == '*') { prec = va_arg(ap, int); fmt++; }
            else while (isdigit(*fmt)) prec = prec * 10 + *fmt++ - '0';
        }
        for (;; fmt++) {
            if (*fmt == 'h') size = size == -1 ? -2 : -1;
            else if (*fmt == 'l') size++;
            else if (*fmt == 'z' || *fmt == 'j' || *fmt == 't') size = 2;
            else if (*fmt == 'L') size = 3;
            else break;
        }
        char b[128], *s = b; int len = 0; const char *pre = "";
        switch (*fmt) {
        case 'd': case 'i': {
            long long v = size >= 1 ? va_arg(ap, long long) : va_arg(ap, int);
            if (size == -1) v = (short)v; else if (size == -2) v = (signed char)v;
            unsigned long long u = v < 0 ? -(unsigned long long)v : v;
            pre = v < 0 ? "-" : plus ? "+" : space ? " " : "";
            char *p = b + sizeof b; *--p = 0;
            do *--p = '0' + u % 10; while (u /= 10);
            if (prec == 0 && v == 0) p++;
            while (prec > (int)(b + sizeof b - 1 - p)) *--p = '0';
            if (prec >= 0) zero = 0;
            out_field(o, pre, p, strlen(p), width, left, zero);
            continue;
        }
        case 'u': case 'o': case 'x': case 'X': case 'p': {
            unsigned long long u;
            int base = *fmt == 'o' ? 8 : *fmt == 'u' ? 10 : 16;
            const char *dig = *fmt == 'X' ? "0123456789ABCDEF" : "0123456789abcdef";
            if (*fmt == 'p') { u = (uintptr_t)va_arg(ap, void *); alt = 1; }
            else u = size >= 1 ? va_arg(ap, unsigned long long) : va_arg(ap, unsigned int);
            if (size == -1) u = (unsigned short)u; else if (size == -2) u = (unsigned char)u;
            if (alt && base == 16 && u) pre = *fmt == 'X' ? "0X" : "0x";
            char *p = b + sizeof b; *--p = 0;
            do *--p = dig[u % base]; while (u /= base);
            if (prec == 0 && p[0] == '0' && !p[1] && *fmt != 'p') p++;
            while (prec > (int)(b + sizeof b - 1 - p)) *--p = '0';
            if (alt && base == 8 && *p != '0') *--p = '0';
            if (prec >= 0) zero = 0;
            out_field(o, pre, p, strlen(p), width, left, zero);
            continue;
        }
        case 'c': b[0] = va_arg(ap, int); len = 1; break;
        case 's': s = va_arg(ap, char *); if (!s) s = "(null)"; len = strlen(s); if (prec >= 0 && prec < len) len = prec; break;
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': {
            long double v = size == 3 ? va_arg(ap, long double) : va_arg(ap, double);
            pre = v < 0 ? "-" : plus ? "+" : space ? " " : "";
            len = fmt_float(b, v, prec, *fmt, alt);
            out_field(o, pre, b, len, width, left, zero);
            continue;
        }
        case '%': b[0] = '%'; len = 1; break;
        default: out_c(o, '%'); out_c(o, *fmt); continue;
        }
        out_field(o, "", s, len, width, left, 0);
    }
    return o->len;
}
int vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap)
{
    struct out o = { buf, cap, 0, NULL };
    int n = format(&o, fmt, ap);
    if (cap) buf[o.len < cap ? o.len : cap - 1] = 0;
    return n;
}
int vsprintf(char *buf, const char *fmt, va_list ap) { return vsnprintf(buf, (size_t)-1 / 2, fmt, ap); }
int vfprintf(FILE *f, const char *fmt, va_list ap) { struct out o = { NULL, 0, 0, f }; return format(&o, fmt, ap); }
int vprintf(const char *fmt, va_list ap) { return vfprintf(stdout, fmt, ap); }
int snprintf(char *buf, size_t cap, const char *fmt, ...) { va_list ap; va_start(ap, fmt); int n = vsnprintf(buf, cap, fmt, ap); va_end(ap); return n; }
int sprintf(char *buf, const char *fmt, ...) { va_list ap; va_start(ap, fmt); int n = vsprintf(buf, fmt, ap); va_end(ap); return n; }
int fprintf(FILE *f, const char *fmt, ...) { va_list ap; va_start(ap, fmt); int n = vfprintf(f, fmt, ap); va_end(ap); return n; }
int printf(const char *fmt, ...) { va_list ap; va_start(ap, fmt); int n = vfprintf(stdout, fmt, ap); va_end(ap); return n; }

/* ---- the end of a program ---- */
__attribute__((noreturn)) void exit(int code) { fflush(stdout); fflush(stderr); _exit(code); }
__attribute__((noreturn)) void abort(void) { fputs("abort\n", stderr); _exit(134); }
#define assert(x) ((x) ? (void)0 : (fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #x), abort()))

/* ---- paths ---- */
/* the absolute path, with . and .. taken out (links aren't followed, there are none to follow here) */
char *realpath(const char *path, char *out)
{
    char tmp[PATH_MAX], *r = out ? out : malloc(PATH_MAX), *p, *seg;
    size_t n = 0;
    if (!r) return NULL;
    if (path[0] == '/') strcpy(tmp, path);
    else { if (!getcwd(tmp, sizeof tmp)) return NULL; strcat(tmp, "/"); strcat(tmp, path); }
    r[0] = 0;
    for (p = tmp; *p; ) {
        while (*p == '/') p++;
        seg = p;
        while (*p && *p != '/') p++;
        size_t l = p - seg;
        if (l == 0 || (l == 1 && seg[0] == '.')) continue;
        if (l == 2 && seg[0] == '.' && seg[1] == '.') { while (n && r[--n] != '/'); r[n] = 0; continue; }
        r[n++] = '/'; memcpy(r + n, seg, l); n += l; r[n] = 0;
    }
    if (!n) strcpy(r, "/");
    return r;
}

/* ---- tcc's helper for va_arg ----
 * under tcc every va_arg calls __va_arg(ap, kind, size, align): kind 0 = a general register, 1 = an SSE register,
 * 2 = the stack. the layout is the x86-64 ABI's: 6 general registers (48 bytes), then 8 SSE ones of 16 */
#ifdef __TINYC__
void *__va_arg(__builtin_va_list ap, int kind, int size, int align)
{
    size = (size + 7) & ~7; align = (align + 7) & ~7;
    if (kind == 0 && ap->gp_offset + size <= 48) { ap->gp_offset += size; return ap->reg_save_area + ap->gp_offset - size; }
    if (kind == 1 && size <= 8 && ap->fp_offset < 176) { ap->fp_offset += 16; return ap->reg_save_area + ap->fp_offset - 16; }
    ap->overflow_arg_area = (char *)(((uintptr_t)ap->overflow_arg_area + align - 1) & -(uintptr_t)align);
    ap->overflow_arg_area += size;
    return ap->overflow_arg_area - size;
}
#endif

/* ---- the environment and running programs ---- */
char **environ;
char *getenv(const char *name)
{
    size_t l = strlen(name); char **e;
    for (e = environ; e && *e; e++) if (!strncmp(*e, name, l) && (*e)[l] == '=') return *e + l + 1;
    return NULL;
}
/* execl(path, arg0, arg1, ..., 0): execve with the arguments written out */
int execl(const char *path, const char *arg, ...)
{
    char *argv[64]; int n = 0; va_list ap;
    argv[n++] = (char *)arg;
    va_start(ap, arg);
    while (n < 63 && (argv[n] = va_arg(ap, char *))) n++;
    va_end(ap);
    argv[n] = NULL;
    return execve(path, argv, environ);
}
/* execve, looking through $PATH when the name has no / in it */
int execvp(const char *file, char *const argv[])
{
    char path[PATH_MAX]; const char *p = getenv("PATH"), *q;
    if (strchr(file, '/')) return execve(file, argv, environ);
    if (!p) p = "/bin";
    for (; *p; p = *q ? q + 1 : q) {
        q = strchr(p, ':'); if (!q) q = p + strlen(p);
        snprintf(path, sizeof path, "%.*s/%s", (int)(q - p), p, file);
        execve(path, argv, environ);
    }
    return -1;
}

/* ---- start-up ----
 * the kernel leaves argc, argv and the environment on the stack, then jumps to _start */
int main(); /* no prototype: int main(void), main(argc, argv) and main(argc, argv, envp) all work */
void c_start(long *sp)
{
    int argc = (int)sp[0];
    char **argv = (char **)(sp + 1);
    environ = argv + argc + 1;
    exit(main(argc, argv, environ));
}
#ifndef C_NO_START
__asm__(".text\n.global _start\n_start:\n xor %rbp,%rbp\n mov %rsp,%rdi\n and $-16,%rsp\n call c_start\n hlt\n");
#endif

#endif
