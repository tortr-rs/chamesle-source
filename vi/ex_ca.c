#
/*
 * Ex - a text editor
 * Bill Joy September 1977
 */

#include "ex.h"
#include "ex_tty.h"

/*
canCA()
{

	switch (TTY) {

	case 'ca':
	case 'cH':
		return (1);
	}
	return (0);
}

*/

cgoto()
{
	static char result[16];
	register char *cp;

	switch (TTY) {

	case ('c' | 'a' << 8):
		strcpy(result, "\033=xx");
		result[2] = destline + ' ';
		result[3] = destcol + ' ';
		break;
#ifdef UNIMP
	case ('c' | 'H' << 8):
		strcpy(result, "\033&axxcyyY");
		result[3] = destcol / 10 + '0';
		result[4] = destcol % 10 + '0';
		result[6] = destline / 10 + '0';
		result[7] = destline % 10 + '0';
		break;
#endif
	case ('l' | 'i' << 8):
		/*
		 * Chamesle Source: the Linux console, an ANSI
		 * terminal. ESC [ line ; column H, from 1.
		 */
		cp = result;
		*cp++ = '\033';
		*cp++ = '[';
		cp = cgonum(cp, destline + 1);
		*cp++ = ';';
		cp = cgonum(cp, destcol + 1);
		*cp++ = 'H';
		*cp = 0;
		break;
	default:
		strcpy(result, "OOPS");
		break;
	}
	return (result);
}

cgonum(cp, n)
	register char *cp;
	register int n;
{

	if (n >= 100)
		*cp++ = n / 100 + '0';
	if (n >= 10)
		*cp++ = n / 10 % 10 + '0';
	*cp++ = n % 10 + '0';
	return (cp);
}
