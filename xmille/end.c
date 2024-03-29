/*	$NetBSD: end.c,v 1.7 2003/08/07 09:37:25 agc Exp $	*/

/*
 * Copyright (c) 1982, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

# include	"mille.h"

/*
 * @(#)end.c	1.1 (Berkeley) 4/1/82
 */

/*
 *	print out the score as if it was final, and add the totals for
 * the end-of-games points to the user who deserves it (if any).
 */
void
finalscore(PLAY *pp)
{
	int	tot, num;

	num = pp - Player;
	for (tot = 4; tot <= 8; tot++)
		InScore(tot, num, "   0  ");
	if (pp->mileage == End) {
		InScore(4, num, " 400  ");
		tot = SC_TRIP;
		if (pp->nummiles[C_200] == 0) {
			InScore(5, num, " 300  ");
			tot = SC_TRIP + SC_SAFE;
		}
		if (Topcard <= Deck) {
			InScore (6, num, " 300  ");
			tot += SC_DELAY;
		}
		if (End == 1000) {
			InScore (7, num, " 200  ");
			tot += SC_EXTENSION;
		}
		if (Player[other(num)].mileage == 0) {
			InScore (8, num, " 500  ");
			tot += SC_SHUT_OUT;
		}
		pp->total += tot;
		pp->hand_tot += tot;
	}
}

# ifdef EXTRAP
static int	Last_tot[2];	/* last tot used for extrapolate	*/

/*
 *	print out the score as if it was final, and add the totals for
 * the end-of-games points to the user who deserves it (if any).
 */
void
extrapolate(PLAY *pp)
{

	int		x, num, tot, count;

#ifdef NOTYET
	num = pp - Player;
	tot += SC_TRIP + SC_DELAY + SC_EXT;
	x = num * 6 + 21 + 3;
	for (tot = 5; tot <= 9; tot++)
		mvaddch(tot, x, '0');
	x -= 2;
	pp = &Player[other(num)];
	for (count = 0, tot = 0; tot < NUM_SAFE; tot++)
		if (pp->safety[tot] != S_PLAYED)
			count += SC_SAFE;
	mvprintw(3, x, "%3d", count);
	tot += count;
	if (count == 400) {
		mvaddstr(4, x, "30");
		tot += SC_ALL_SAFE;
	}
	pp = &Player[num];
	for (count = 0, tot = 0; tot < NUM_SAFE; tot++)
		if (pp->safety[tot] != S_PLAYED)
			count += SC_COUP / 10;
	mvprintw(4, x - 1, "%3d", count);
	tot += count;
	tot += 1000 - pp->mileage;
	mvaddstr(5, x, "40");
	mvaddstr(7, x, "30");
	mvaddstr(8, x, "20");
	if (pp->nummiles[C_200] == 0) {
		mvaddstr(6, x, "30");
		tot = SC_TRIP + SC_SAFE;
	}
	if (Player[other(num)].mileage == 0) {
		mvaddstr(9, x, "50");
		tot += SC_SHUT_OUT;
	}
	pp->total += tot;
	pp->hand_tot += tot;
	Last_tot[num] = tot;
#endif
}

void
undoex(void)
{

	PLAY	*pp;
	int	i;

	i = 0;
	for (pp = Player; pp < &Player[2]; pp++) {
		pp->total -= Last_tot[i];
		pp->hand_tot -= Last_tot[i++];
	}
}
# endif
