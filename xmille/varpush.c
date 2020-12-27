/*
 * Copyright (c) 1982, 1993
 *	The Regents of the University of California.  All rights reserved.
 */

# include	"mille.h"

/*
 * @(#)varpush.c	1.1 (Berkeley) 4/1/82
 */

/*
 *	push variables around via the routine func() on the file
 * channel file.  func() is either read or write.
 */
int
varpush(int file, int (*func)(int, void *, size_t))
{
	int	temp;
	int	fail = 0;

	if ((*func)(file, &Debug, sizeof Debug) != sizeof Debug) fail++;
	if ((*func)(file, &Finished, sizeof Finished) != sizeof Finished) fail++;
	if ((*func)(file, &Order, sizeof Order) != sizeof Order) fail++;
	if ((*func)(file, &End, sizeof End) != sizeof End) fail++;
	if ((*func)(file, &On_exit, sizeof On_exit) != sizeof On_exit) fail++;
	if ((*func)(file, &Handstart, sizeof Handstart) != sizeof Handstart) fail++;
	if ((*func)(file, &Numgos, sizeof Numgos) != sizeof Numgos) fail++;
	if ((*func)(file,  Numseen, sizeof Numseen) != sizeof Numseen) fail++;
	if ((*func)(file, &Play, sizeof Play) != sizeof Play) fail++;
	if ((*func)(file, &WIndow, sizeof WIndow) != sizeof WIndow) fail++;
	if ((*func)(file,  Deck, sizeof Deck) != sizeof Deck) fail++;
	if ((*func)(file, &Discard, sizeof Discard) != sizeof Discard) fail++;
	if ((*func)(file,  Player, sizeof Player) != sizeof Player) fail++;
	if (func == (int (*)(int, void *, size_t)) read) {
		if (read(file, &temp, sizeof temp) != sizeof temp) fail++;
		Topcard = &Deck[temp];
	}
	else {
		temp = Topcard - Deck;
		if (write(file, &temp, sizeof temp) != sizeof temp) fail++;
	}
	if (fail)
		return FALSE;
	return TRUE;
}
