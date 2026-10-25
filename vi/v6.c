/* v6.c: what ex 1.1 expected from Version 6 Unix and its PDP-11 library, on top of c.h.
 *
 * one more thing from the PDP-11: there, ints and pointers were the same 16 bits, and ex passes pointers through
 * ints all over (every function without a type returns int). here pointers are 64 bits, but nothing is lost as long
 * as every address ex sees is under 2 GB. a program built static at a fixed address has its code, data and sbrk
 * heap down there already. the stack and the arguments aren't, so _start below moves them, like ex's own crt0 did */
/* sys.c includes c.h before this, with ex's own strcpy, putchar and the rest renamed out of the way */

int putchar(); /* ex's: through Putchar */

/* ---- start-up ---- */
extern char **xargv0, *G;
extern int xargc0;
__attribute__((aligned(16), used)) char v6stack[1 << 20];
static char v6glob[2048] __attribute__((aligned(16))); /* ex_glob.h's struct Glob: the arguments the "next" command globs */
static char v6args[4096], *v6argv[64];
int main();
void v6quit(int code);

__attribute__((used)) static void v6start(long *sp)
{
    int argc = sp[0], i;
    char **argv = (char **)(sp + 1), *p = v6args;
    environ = argv + argc + 1;
    for (i = 0; i < argc && i < 63 && p + c_strlen(argv[i]) + 1 < v6args + sizeof v6args; i++) {
        v6argv[i] = c_strcpy(p, argv[i]);
        p += c_strlen(p) + 1;
    }
    v6argv[i] = NULL;
    xargc0 = i;
    xargv0 = v6argv;
    G = v6glob;
    v6quit(main());
}
__asm__(".text\n.global _start\n_start:\n xor %rbp,%rbp\n mov %rsp,%rdi\n lea v6stack+1048576(%rip),%rsp\n"
        " call v6start\n hlt\n");

/* quitting ex. as PID 1 it is the last program running, and when PID 1 exits Linux panics: so quitting there
 * writes out what the disks still hold and turns the machine off */
void v6quit(int code)
{
    if (getpid() == 1) {
        sys6(162 /* sync */, 0, 0, 0, 0, 0, 0);
        sys6(169 /* reboot */, 0xfee1dead, 672274793, 0x4321fedc /* power off */, 0, 0, 0);
    }
    exit(code);
}

/* ---- setexit and reset ---- */
long v6exit[8];
static long v6saved[16][8];
static int v6nsaved;
void v6getexit(void) { memcpy(v6saved[v6nsaved++], v6exit, sizeof v6exit); }
void v6resexit(void) { memcpy(v6exit, v6saved[--v6nsaved], sizeof v6exit); }

/* ---- the terminal ----
 * gtty and stty: three ints. the speeds, the erase and kill characters, and the flags, of which ex uses these */
#define V6_XTABS 02
#define V6_ECHO 010
#define V6_CRMOD 020
#define V6_RAW 040
int gtty(int fd, int *v)
{
    struct termios t;
    if (tcgetattr(fd, &t) < 0) return -1;
    v[0] = 0;
    v[1] = t.c_cc[VERASE] | t.c_cc[VKILL] << 8;
    v[2] = (t.c_lflag & ECHO ? V6_ECHO : 0) | (t.c_iflag & ICRNL ? V6_CRMOD : 0) |
           (t.c_lflag & ICANON ? 0 : V6_RAW) | ((t.c_oflag & TAB3) == TAB3 ? V6_XTABS : 0);
    return 0;
}
int stty(int fd, int *v)
{
    struct termios t;
    if (tcgetattr(fd, &t) < 0) return -1;
    t.c_cc[VERASE] = v[1] & 0377;
    t.c_cc[VKILL] = v[1] >> 8 & 0377;
    if (v[2] & V6_ECHO) t.c_lflag |= ECHO; else t.c_lflag &= ~ECHO;
    if (v[2] & V6_CRMOD) { t.c_iflag |= ICRNL; t.c_oflag |= OPOST | ONLCR; }
    else { t.c_iflag &= ~ICRNL; t.c_oflag &= ~ONLCR; }
    if (v[2] & V6_XTABS) t.c_oflag |= TAB3; else t.c_oflag &= ~TAB3;
    if (v[2] & V6_RAW) { t.c_lflag &= ~(ICANON | ISIG | IEXTEN); t.c_iflag &= ~IXON; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0; }
    else t.c_lflag |= ICANON | ISIG | IEXTEN;
    return tcsetattr(fd, 0, &t);
}

/* ttyn says which terminal this is, and /etc/htmp (hget...) its type and its user's home. there is no /etc/htmp:
 * the type is $TERM, which the kernel sets for PID 1, and the user is root, at home in / */
static char v6term[32], v6home[64];
int ttyn(int fd) { return isatty(fd) ? 'c' : 'x'; }
int hget(int tty) { (void)tty; return 0; }
char *hsgettype(void)
{
    char *t = getenv("TERM");
    c_strcpy(v6term, t && c_strlen(t) < sizeof v6term ? t : "un");
    return v6term;
}
char *hgethome(void) { return c_strcpy(v6home, "/"); }
int hgetuid(void) { return 0; }

/* the size of the terminal from the kernel: ttysize(0) its lines, ttysize(1) its columns, 0 if it doesn't know */
int ttysize(int which)
{
    unsigned short ws[4];
    if (ioctl(1, 0x5413 /* TIOCGWINSZ */, ws) < 0) return 0;
    return ws[which];
}

/* ---- files ---- */
/* the entry ttycap.c found: a libX global, defined in a file that isn't on the tape */
char *tbuf;

/* seek: whence 0, 1 and 2 in bytes, 3, 4 and 5 the same in blocks of 512 */
int seek(int fd, int off, int whence)
{
    long o = off;
    if (whence >= 3) { o *= 512; whence -= 3; }
    return lseek(fd, o, whence) < 0 ? -1 : 0;
}

/* stat, as ex_io.h's struct stb has it */
struct stb {
    char minor, major;
    int inumber, flags;
    char nlinks, uid, gid, size0;
    int size1;
    char dminor, dmajor;
    int addr2[7];
    long actime, modtime;
};
static void v6stb(struct stat *s, struct stb *b)
{
    memset(b, 0, sizeof *b);
    b->minor = s->st_dev & 0377; b->major = s->st_dev >> 8 & 0377;
    b->inumber = s->st_ino;
    b->flags = s->st_mode & 0177777;
    b->nlinks = s->st_nlink; b->uid = s->st_uid; b->gid = s->st_gid;
    b->size0 = s->st_size >> 16; b->size1 = s->st_size & 0177777;
    b->dminor = s->st_rdev & 0377; b->dmajor = s->st_rdev >> 8 & 0377;
    b->actime = s->st_atime; b->modtime = s->st_mtime;
}
int v6stat(char *name, struct stb *b)
{
    struct stat s;
    if (stat(name, &s) < 0) { memset(b, 0, sizeof *b); return -1; }
    v6stb(&s, b);
    return 0;
}
int v6fstat(int fd, struct stb *b)
{
    struct stat s;
    if (fstat(fd, &s) < 0) { memset(b, 0, sizeof *b); return -1; }
    v6stb(&s, b);
    return 0;
}

/* the buffered input of ttycap.c: int ibuf[259], the file, how many characters are left, where they are */
int v6fopen(char *name, int *ibuf)
{
    ibuf[0] = open(name, O_RDONLY);
    ibuf[1] = ibuf[2] = 0;
    return ibuf[0];
}
int v6getc(int *ibuf)
{
    char *buf = (char *)&ibuf[3];
    if (ibuf[1] <= 0) {
        int n = read(ibuf[0], buf, 512);
        if (n <= 0) return -1;
        ibuf[1] = n; ibuf[2] = 0;
    }
    ibuf[1]--;
    return buf[ibuf[2]++] & 0377;
}

/* the buffered output of ex.h's obuf */
struct obuf { int fildes, nunused; char *xfree; char buff[512]; };
int v6fflush(struct obuf *b)
{
    if (b->xfree && b->xfree > b->buff) write(b->fildes, b->buff, b->xfree - b->buff);
    b->xfree = b->buff;
    b->nunused = 512;
    return 0;
}
int v6putc(int c, struct obuf *b)
{
    if (b->xfree == NULL || b->nunused <= 0) v6fflush(b);
    *b->xfree++ = c;
    b->nunused--;
    return c;
}

/* ---- printf ----
 * the PDP-11 printf ex was linked with prints through putchar, ex's own. it knows %d %o %c %s and %l (a long),
 * a width, and %c prints both characters of a two-character constant. arguments are read as longs: a K&R call
 * passes an int where a pointer was meant, and under 2 GB its low 32 bits are the pointer */
static void v6num(unsigned long u, int neg, int base, int width)
{
    char b[24], *p = b + sizeof b;
    do *--p = "0123456789abcdef"[u % base]; while (u /= base);
    if (neg) *--p = '-';
    for (width -= b + sizeof b - p; width > 0; width--) putchar(' ');
    while (p < b + sizeof b) putchar(*p++);
}
int v6printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        int width = 0, isl = 0, c;
        long v;
        if (*fmt != '%') { putchar(*fmt); continue; }
        fmt++;
        if (*fmt == '-') fmt++;
        while (isdigit(*fmt)) width = width * 10 + *fmt++ - '0';
        if (*fmt == 'l') { isl = 1; if (fmt[1] == 'd' || fmt[1] == 'o' || fmt[1] == 'u') fmt++; }
        v = *fmt && strchr("dluocs", *fmt) ? va_arg(ap, long) : 0;
        switch (*fmt) {
        case 'd': case 'l':
            if (!isl) v = (int)v;
            v6num(v < 0 ? -v : v, v < 0, 10, width);
            break;
        case 'u': v6num(isl ? (unsigned long)v : (unsigned int)v, 0, 10, width); break;
        case 'o': v6num(isl ? (unsigned long)v : (unsigned int)v, 0, 8, width); break;
        case 'c':
            c = v & 0377; if (c) putchar(c);
            c = v >> 8 & 0377; if (c) putchar(c);
            break;
        case 's': {
            char *s = (char *)(unsigned long)(unsigned int)v;
            int n = c_strlen(s);
            for (width -= n; width > 0; width--) putchar(' ');
            while (*s) putchar(*s++);
            break;
        }
        case 0: fmt--; break;
        default: putchar(*fmt); break;
        }
    }
    va_end(ap);
    return 0;
}
