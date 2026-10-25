#
/*
 * Ex - a text editor
 * Bill Joy UCB September 1977
 */

#include "ex.h"
#include "ex_tty.h"

char	tspace[64], **Tspace;
char	ttycap[];

int	COLUMNS = 1000;
int	LINES = 24;

setterm(type)
	char *type;
{

	Setterm(type, 1);
}

Setterm(type, printerr)
	char *type;
	int printerr;
{
	char buf[512], *cgoto();
	register int unknown;
	extern struct varbl varbls[];

	if (type[0] == 0)
		type = "un";
	unknown = 0;
	if (tgetent(buf, type) != 1) {
		unknown = printerr;
		strcpy(buf, "un|unknown:0:0:?");
	}
	TTY = buf[0] | (buf[1] << 8);
/*
	tgetmodes(MODES);
*/
	LINES = tgetnum("li");
	if (LINES <= 0)
		LINES = 24;
	/*
	 * Chamesle Source: the Linux console is as big as its screen, so
	 * the kernel's size goes before ttycap's. visual holds 100 lines and
	 * 160 columns: on a wider screen ex starts the next line at 160.
	 */
	if (ttysize(0) > 0)
		LINES = ttysize(0) > 100 ? 100 : ttysize(0);
	varbls[WINDOW].vvalue = varbls[WINDOW].vdefault = LINES - 1;
	varbls[SCROLL].vvalue = varbls[SCROLL].vdefault = LINES / 2;
	COLUMNS = tgetnum("co");
	if (ttysize(1) > 0)
		COLUMNS = ttysize(1) > 160 ? 160 : ttysize(1);
	if (COLUMNS <= 20)
		COLUMNS = 1000;
/*
	IT = tgetflag("it");
*/
	CA = tgetflag("ca");
	if (cgoto()[0] == 'O')
		CA = 0;
	AM = tgetflag("am");
	BS = tgetflag("bs");
	OS = tgetflag("os");
	Tspace = tspace;
	CLEAR = Tgetstr("cl");
	NDSPACE = Tgetstr("nd");
	UPLINE = Tgetstr("up");
/*
	AL = Tgetstr("al");
	DL = Tgetstr("dl");
*/
	CE = Tgetstr("ce");
/*
	CD = Tgetstr("cd");
*/
	strcpy(ttytype, longname(buf, type));
	termreset();
	if (unknown)
		error("%s: Unknown terminal type@- do \"!ttytype -n\" for a list of valid types", type);
}

longname(bp, def)
	register char *bp;
	char *def;
{
	register char *cp;

	while (*bp && *bp != ':' && *bp != '|')
		bp++;
	if (*bp == '|') {
		bp++;
		cp = bp;
		while (*cp && *cp != ':' && *cp != '|')
			cp++;
		*cp = 0;
		return (bp);
	}
	return (def);
}

Tgetstr(cp)
	char *cp;
{

	tgetstr(cp, &Tspace);
}
