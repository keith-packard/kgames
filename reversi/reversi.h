/*
 *	reversi.h
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

extern int legal ();
extern int hasmove ();
extern void dispError ();
extern void dispInit ();
extern void dispGrid ();
extern void fini ();
extern int computer ();
extern int seek ();
extern void move ();
extern int hint ();
extern int seek ();
extern int count ();
extern void setup ();
extern void display ();
extern void replay ();
extern int playGame ();
extern void dispEnd ();
extern void dispTurn ();
extern void dispMove ();
extern void dispHint ();
extern void dispHelp ();
extern void dispNoGrid ();
extern void dispScore ();
extern void dispNoScore ();
extern void dispNoHelp ();
extern void copy ();
extern void doHint ();
extern void undo ();
extern void domove ();
extern void checkInput ();

extern int rand ();
extern void srand ();
extern int getpid ();
