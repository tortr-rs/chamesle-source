/* c.h: the whole libc of Chamesle Source Linux, in one file. x86-64 Linux only.
 * a program includes it once and gets everything: the start-up code, system calls, strings, memory, output.
 * build: cc -static -nostdlib -ffreestanding -fno-stack-protector -fno-builtin -no-pie prog.c -o prog */
#ifndef C_H
#define C_H

typedef unsigned long size_t;
typedef long ssize_t;
typedef long off_t;
typedef int pid_t;
#define NULL ((void *)0)

/* system calls: number in rax, arguments in rdi rsi rdx r10 r8 r9, the kernel clobbers rcx and r11 */
static long sys6(long n, long a, long b, long c, long d, long e, long f)
{
    register long r10 __asm__("r10") = d, r8 __asm__("r8") = e, r9 __asm__("r9") = f;
    long ret;
    __asm__ volatile ("syscall" : "=a"(ret) : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9)
                      : "rcx", "r11", "memory");
    return ret;
}
#define sys0(n) sys6(n, 0, 0, 0, 0, 0, 0)
#define sys1(n, a) sys6(n, (long)(a), 0, 0, 0, 0, 0)
#define sys2(n, a, b) sys6(n, (long)(a), (long)(b), 0, 0, 0, 0)
#define sys3(n, a, b, c) sys6(n, (long)(a), (long)(b), (long)(c), 0, 0, 0)

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_mmap 9
#define SYS_munmap 11
#define SYS_fork 57
#define SYS_execve 59
#define SYS_exit 60
#define SYS_wait4 61
#define SYS_mkdir 83
#define SYS_mount 165
#define SYS_reboot 169

ssize_t read(int fd, void *buf, size_t n) { return sys3(SYS_read, fd, buf, n); }
ssize_t write(int fd, const void *buf, size_t n) { return sys3(SYS_write, fd, buf, n); }
int open(const char *path, int flags, int mode) { return sys3(SYS_open, path, flags, mode); }
int close(int fd) { return sys1(SYS_close, fd); }
pid_t fork(void) { return sys0(SYS_fork); }
int execve(const char *path, char *const argv[], char *const envp[]) { return sys3(SYS_execve, path, argv, envp); }
pid_t waitpid(pid_t pid, int *status, int options) { return sys6(SYS_wait4, pid, (long)status, options, 0, 0, 0); }
int mkdir(const char *path, int mode) { return sys2(SYS_mkdir, path, mode); }
int mount(const char *src, const char *dst, const char *type, unsigned long flags, const void *data)
{
    return sys6(SYS_mount, (long)src, (long)dst, (long)type, flags, (long)data, 0);
}
__attribute__((noreturn)) void exit(int code) { for (;;) sys1(SYS_exit, code); }

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0100
#define O_TRUNC 01000

/* strings */
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) a++, b++; return (unsigned char)*a - (unsigned char)*b; }
int strncmp(const char *a, const char *b, size_t n)
{
    for (; n && *a && *a == *b; n--) a++, b++;
    return n ? (unsigned char)*a - (unsigned char)*b : 0;
}
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)); return r; }
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

/* memory: every block straight from mmap, its size kept just before it. simple, never fragments, a bit wasteful */
void *malloc(size_t n)
{
    size_t len = (n + sizeof(size_t) + 4095) & ~4095UL;
    long p = sys6(SYS_mmap, 0, len, 3 /* read, write */, 0x22 /* private, anonymous */, -1, 0);
    if (p < 0 && p > -4096) return NULL;
    *(size_t *)p = len;
    return (size_t *)p + 1;
}
void free(void *p) { if (p) { size_t *h = (size_t *)p - 1; sys2(SYS_munmap, h, *h); } }
void *realloc(void *p, size_t n)
{
    if (!p) return malloc(n);
    size_t old = ((size_t *)p)[-1] - sizeof(size_t);
    if (n <= old) return p;
    void *q = malloc(n);
    if (q) { memcpy(q, p, old); free(p); }
    return q;
}

/* output: strings and numbers, no printf yet */
void puts_fd(int fd, const char *s) { write(fd, s, strlen(s)); }
void puts(const char *s) { puts_fd(1, s); write(1, "\n", 1); }
void putnum(long v)
{
    char b[24]; int i = 23; unsigned long u = v < 0 ? -(unsigned long)v : (unsigned long)v;
    b[i] = 0;
    do b[--i] = '0' + u % 10; while (u /= 10);
    if (v < 0) b[--i] = '-';
    puts_fd(1, b + i);
}

/* start-up: the kernel leaves argc, argv and the environment on the stack */
char **environ;
int main(int argc, char **argv);
__attribute__((used)) void c_start(long *sp)
{
    int argc = (int)sp[0];
    char **argv = (char **)(sp + 1);
    environ = argv + argc + 1;
    exit(main(argc, argv));
}
__asm__(".text\n.global _start\n_start:\n xor %rbp,%rbp\n mov %rsp,%rdi\n and $-16,%rsp\n call c_start\n hlt\n");

#endif
