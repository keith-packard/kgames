/*
 * Copyright © 2020 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <time.h>
#include "revers.h"

boardT	board, saveBoard;
int	saved;
int	savePlayer;
int	atend;
int	atbegin;
int	level;
int	player;
extern int	maxlev, movex, movey;
int	x, y;
int	com;
int	gotsignal;
char	sbuf[80];
char	ebuf[80];
int	sdebug = 0, mdebug = 0;
int	record = 0;
FILE	*rfile;
int	first = WHITE;
int	defcom = BLACK;
int	showScore = 1;

struct move	saveGame[64];
struct move	*saveP;

static void
caught (int sig)
{
	(void) sig;
	gotsignal++;
	signal (SIGINT, caught);
}

int
main (int argc, char **argv)
{
	signal (SIGINT, caught);
	level = 2;
	dispInit (argc, argv);
	srandom (time(NULL));
	while (*++argv && **argv == '-') {
		while (*++*argv) {
			switch (**argv) {
			case 'b':
				defcom = BLACK;
				break;
			case 'w':
				defcom = WHITE;
				break;
			case '1':
				if (!*++*argv)
					continue;
				if (**argv == WHITE)
					first = WHITE;
				else
					first = BLACK;
				break;
			case 'g':
				dispGrid ();
				break;
			case 's':
				showScore = 1;
			}
		}
	}
	do {
		if (rfile)
			fclose (rfile);
		rfile = 0;
		player = first;
		com = defcom;
		atend = 0;
		atbegin = 1;
		setup ();
		saved = 0;
		saveP = saveGame;
		display (board);
		if (*argv) {
			replay (*argv);
			++argv;
		}
	} while (playGame());
	dispEnd ();
	return 0;
}

void
setup (void)
{
	register int	i,j;

	for (i = 1; i <= SIZE; i++)
		for (j = 1; j <= SIZE; j++)
			board[i][j] = 0;
	board[4][4] = WHITE;
	board[4][5] = BLACK;
	board[5][4] = BLACK;
	board[5][5] = WHITE;
}

void
replay (char *file)
{
	int	x, y, p;
	if (rfile)
		fclose (rfile);
	if ((rfile = fopen (file, "r")) == NULL) {
		sprintf (ebuf, "could not open %s", file);
		dispError (ebuf);
		return;
	}
	while (fscanf (rfile, "%d: %d, %d\n", &p, &x, &y) == 3) {
		if (x == -1 && y == -1) {
			player = p;
			continue;
		}
		if (!hasmove (player, board)) {
			player = -player;
			if (!hasmove (player, board))
				return;
		}
		if (p != player) {
			sprintf (ebuf, "not %s's turn\n",
			    player == WHITE? "white":"black");
			dispError (ebuf);
			return;
		}
		if (!legal (p, x, y, board)) {
			sprintf(ebuf, "illegal move: %d, %d\n", x, y);
			dispError (ebuf);
			return;
		}
		move (p, x, y, board);
		atbegin = 0;
		player = -player;
		display (board);
	}
	fclose (rfile);
	rfile = 0;
}

void
domove (int x, int y)
{
	if (1 <= x && x <= SIZE &&
	    1 <= y && y <= SIZE &&
	    legal (player, x, y, board)) {
		copy (saveBoard, board);
		savePlayer = player;
		++saved;
		move (player, x, y, board);
		atbegin = 0;
		if (record)
			fprintf (rfile, "%d: %d,%d\n",
			    player, x, y);
		saveP->x = x;
		saveP->y = y;
		saveP->p = player;
		++saveP;
		player = -player;
		display (board);
	} else {
		sprintf (ebuf, "illegal move: %c %d",
			y+'a'-1, x);
		dispError (ebuf);
	}
}

void
checkInput (void)
{
	if (!atend) {
		loop:	;
		dispTurn (player);
		if (!hasmove (player, board)) {
			if (!hasmove (-player, board)) {
				fini (board);
				if (com == 0)
					com = BLACK;
				++atend;
				dispTurn (EMPTY);
				return;
			} else {
				if (player == WHITE)
					dispError ("white has no move");
				else
					dispError ("black has no move");
				player = -player;
			}
		}
		if (com == 0 || com == player) {
			dispError ("thinking...");
			if (computer (player, board, level)) {
				atbegin = 0;
				display (board);
				dispMove (movex, movey, player);
				saveP->x = movex;
				saveP->y = movey;
				saveP->p = player;
				++saveP;
				if (record)
					fprintf (rfile, "%d: %d,%d\n",
					    player, movex, movey);
				player = -player;
				if (gotsignal && com != 0)
					gotsignal = 0;
			}
			if (gotsignal && com == 0) {
				com = -player;
				gotsignal = 0;
			}
			goto loop;
		}
	}
}

void
undo (void)
{
	if (saved) {
		copy (board, saveBoard);
		player = savePlayer;
		saved = 0;
		display (board);
	}
}

void
doHint (void)
{
	if (hasmove (player, board)) {
		hint (player, board, level);
		dispHint (movex, movey, player);
	}
}
