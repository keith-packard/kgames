/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)cribbage.h	5.1 (Berkeley) 5/30/85
 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

extern  CARD		deck[ CARDS ];		/* a deck */
extern  CARD		phand[ FULLHAND ];	/* player's hand */
extern  CARD		chand[ FULLHAND ];	/* computer's hand */
extern  CARD		crib[ CINHAND ];	/* the crib */
extern  CARD		turnover;		/* the starter */

extern  CARD		known[ CARDS ];		/* cards we have seen */
extern  int		knownum;		/* # of cards we know */

extern  int		pscore;			/* player's score */
extern  int		cscore;			/* comp's score */
extern  int		glimit;			/* points to win game */

extern  int		pgames;			/* player's games won */
extern  int		cgames;			/* comp's games won */
extern  int		gamecount;		/* # games played */
extern	int		Lastscore[2];		/* previous score for each */

extern  BOOLEAN		iwon;			/* if comp won last */
extern  BOOLEAN		explain;		/* player mistakes explained */
extern  BOOLEAN		rflag;			/* if all cuts random */
extern  BOOLEAN		quiet;			/* if suppress random mess */
extern	BOOLEAN		playing;		/* currently playing game */


extern  char		c_expl[];			/* string for explanation */
#define expl c_expl

#define PLAYER	    0
#define COMPUTER    1
#define TABLE	    2

#define getline	crib_getline

void
UIInit (int argc, char **argv);

void
UISuspend (void);

void
UIResume (void);

void
UIFinish (void);

void
UIInitBoard (void);

void
UIGameScore (int who, int num);

void
UIRefresh (void);

void
UIWait (void);

void
UIPause(void);

void
UIEraseHand (int who);

void
UIClearHand (int who);

void
UIPrintHand (CARD *h, int n, int who, BOOLEAN blank);

void
UIPrintCrib (int who, CARD *card, BOOLEAN blank);

void
UITableScore (int score, int n);

void
UIPrintPeg (int score, BOOLEAN on, int who);

void
ShowCursor (void);

void
HideCursor (void);

int
UIReadChar (void);

void
UIEchoChar (char c);

void
UIReadLine (char *buf, int len);

void
UIMessage (char *str, int newline);

int
UIGetMessageSize (void);

void
UIClearMsg (void);

int
UIGetPlayerCard (CARD *hand, int n, char *prompt);

void
msg(char *fmt, ...);

void
addmsg(char *fmt, ...);

int
msgcard(CARD c, BOOLEAN brief);

int
msgcrd(CARD c, BOOLEAN brfrank, char *mid, BOOLEAN brfsuit);

int
getuchar(void);

int
number(int lo, int hi, char *prompt);

void
msg(char *fmt, ...);

void
addmsg(char *fmt, ...);

void
endmsg(BOOLEAN newline);

void
doadd(char *fmt, va_list args);

char *
getline(void);

void
quit(int status);

void
makeboard(void);

void
gamescore(void);

void
game(void);

BOOLEAN
playhand(BOOLEAN mycrib);

int
deal(int mycrib);

void
discard(BOOLEAN mycrib);

BOOLEAN
cut(BOOLEAN mycrib, int pos);

BOOLEAN
peg(BOOLEAN mycrib);

void
prtable(int score);

BOOLEAN
score(BOOLEAN mycrib);

void
makedeck( CARD *d );

void
shuffle( CARD *d );

BOOLEAN
eq( CARD a, CARD b );

BOOLEAN
isone( CARD a, CARD *b, int n );

void
remove_card( CARD a, CARD *d, int n );

void
sorthand(CARD *h, int n);

int
cchose( CARD *h, int n, int s );

BOOLEAN
plyrhand(CARD *hand, char *s);

BOOLEAN
comphand(CARD *h, char *s);

BOOLEAN
chkscr(int *scr, int inc);

void
cdiscard( BOOLEAN mycrib );

BOOLEAN
anymove( CARD *hand, int n, int sum );

int
anysumto( CARD *hand, int n, int s, int t );

int
numofval( CARD *h, int n, int v );

void
makeknown( CARD *h, int n );

int
scorehand(CARD *hand, CARD starter, int n, BOOLEAN crb, BOOLEAN do_explain);

int
fifteens(CARD *hand, int n);

int
pairuns( CARD *h, int n );

int
pegscore( CARD crd, CARD *tbl, int n, int sum );

int
adjust( CARD *cb, CARD tnv );
