/*
 *	revers.h
 *
 *	include file for game program
 */

#include <stdio.h>

# define	SIZE	8

typedef char	boardE;

typedef short	scoreT;

typedef boardE	boardT[SIZE+2][SIZE+2];

typedef boardT	*boardP;

struct move {
	int	p, x, y;
};

# define	EMPTY	0
# define	WHITE	1
# define	BLACK	-1

int
legal (int player, int x, int y, boardT board);

int
hasmove (int player, boardT board);

void
dispError (char *s);

void
dispInit(int argc, char **argv);

void
dispGrid (void);

void
fini (boardT board);

int
computer (int player, boardT board, int level);

int
seek (int player, boardT board, int level, int moved, int best);

void
move (int player, int x, int y, boardT board);

int
hint (int player, boardT board, int level);

int
seek (int player, boardT board, int level, int moved, int best);

int
count (int player, boardT board);

void
setup (void);

void
display (boardT board);

void
replay (char *file);

int
playGame (void);

void
dispEnd (void);

void
dispTurn (int player);

void
dispMove (int x, int y, int player);

void
dispHint (int x, int y, int player);

void dispHelp (void);

extern
void dispNoGrid (void);

void
dispScore (boardT board);

void
dispNoScore (void);

void
dispNoHelp (void);

void
copy(boardT next, boardT board);

void
doHint (void);

void
undo (void);

void
domove (int x, int y);

void
checkInput (void);
