/*
 * v6.h: the few calls ex 1.1 made that Version 6 Unix spelled differently
 * from c.h. the build puts it in front of every 1977 file (ex's and libX's
 * ttycap.c alike). v6.c has them.
 */

/* the 1977 buffered output (putc on obuf), input (fopen and getc in ttycap.c) and stat */
#define	putc(c, b)	v6putc(c, b)
#define	fflush(b)	v6fflush(b)
#define	fopen(n, b)	v6fopen(n, b)
#define	getc(b)		v6getc(b)
#define	stat(n, b)	v6stat(n, b)
#define	fstat(f, b)	v6fstat(f, b)
#define	printf		v6printf

/* the V6 linker read 7 characters of a name: ex calls vrestore, and the vrestor in ex_vmain.c was the same */
#define	vrestore	vrestor

/*
 * setexit() marks where reset() comes back to: setjmp and longjmp, with one
 * place to go back to. getexit and resexit keep the old place while a
 * function sets its own, and put it back when it is done.
 */
extern	long v6exit[8];
#define	setexit()	setjmp(v6exit)
#define	reset()		longjmp(v6exit, 1)
#define	getexit(o)	v6getexit()
#define	resexit(o)	v6resexit()
