/*
 * putchar.s from the ex 1.1 tape, written in C: the PDP-11 assembly
 * isn't something any machine today runs. it is what ex_print.c says it was
 * ("Put in assembly language to save space"): output goes through the
 * pointers Putchar, Outchar and Pline, and these swap them.
 */

#include "ex.h"

int	normchar(), termchar(), normline(), listchar(), numbline();

int	(*Putchar)() = normchar;
int	(*Outchar)() = termchar;
int	(*Pline)() = normline;

putNFL()
{

	putnl();
	flush();
}

putchar(c)
{

	return ((*Putchar)(c));
}

outchar(c)
{

	return ((*Outchar)(c));
}

pline(i)
{

	return ((*Pline)(i));
}

setlist()
{
	register int (*P)();

	P = Putchar;
	Putchar = listchar;
	return (P);
}

setnorm()
{
	register int (*P)();

	P = Putchar;
	Putchar = normchar;
	return (P);
}

setnumb()
{

	Pline = numbline;
}

setCNL()
{

	setcount();
	newline();
}

setNAEOL()
{

	setnoaddr();
	eol();
}

setoutt()
{

	Outchar = termchar;
}
