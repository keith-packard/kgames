/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

# include	<ctype.h>
# include	<stdarg.h>
# include	<stdio.h>
# include	"deck.h"
# include	"cribbage.h"

# define	LINESIZE		128

char		linebuf[ LINESIZE ];

char		*rankname[ RANKS ]	= { "ACE", "TWO", "THREE", "FOUR",
					    "FIVE", "SIX", "SEVEN", "EIGHT",
					    "NINE", "TEN", "JACK", "QUEEN",
					    "KING" };

char            *rankchar[ RANKS ]      = { "A", "2", "3", "4", "5", "6", "7",
					    "8", "9", "T", "J", "Q", "K" };

char            *suitname[ SUITS ]      = { "SPADES", "HEARTS", "DIAMONDS",
					    "CLUBS" };

char            *suitchar[ SUITS ]      = { "S", "H", "D", "C" };


/*
 * msgcard:
 *	Call msgcrd in one of two forms
 */
int
msgcard(CARD c, BOOLEAN brief)
{
	if (brief)
		return msgcrd(c, TRUE, (char *) NULL, TRUE);
	else
		return msgcrd(c, FALSE, " of ", FALSE);
}



/*
 * msgcrd:
 *	Print the value of a card in ascii
 */
int
msgcrd(CARD c, BOOLEAN brfrank, char *mid, BOOLEAN brfsuit)
{
	if (c.rank == EMPTY || c.suit == EMPTY)
	    return FALSE;
	if (brfrank)
	    addmsg("%1.1s", rankchar[c.rank]);
	else
	    addmsg(rankname[c.rank]);
	if (mid != NULL)
	    addmsg(mid);
	if (brfsuit)
	    addmsg("%1.1s", suitchar[c.suit]);
	else
	    addmsg(suitname[c.suit]);
	return TRUE;
}

/*
 * getuchar:
 *	Reads and converts to upper case
 */
int
getuchar(void)
{
	register int		c;

	c = UIReadChar ();
	if (islower(c))
	    c = toupper(c);
	UIEchoChar (c);
	return c;
}

/*
 * number:
 *	Reads in a decimal number and makes sure it is between "lo" and
 *	"hi" inclusive.
 */
int
number(int lo, int hi, char *prompt)
{
	register char		*p;
	register int		sum;

	sum = 0;
	for (;;) {
	    msg(prompt);
	    if(!(p = getline()) || *p == '\0') {
		msg(quiet ? "Not a number" : "That doesn't look like a number");
		continue;
	    }
	    sum = 0;

	    if (!isdigit(*p))
		sum = lo - 1;
	    else
		while (isdigit(*p)) {
		    sum = 10 * sum + (*p - '0');
		    ++p;
		}

	    if (*p != ' ' && *p != '\t' && *p != '\0')
		sum = lo - 1;
	    if (sum >= lo && sum <= hi)
		return sum;
	    if (sum == lo - 1)
		msg("that doesn't look like a number, try again --> ");
	    else
		msg("%d is not between %d and %d inclusive, try again --> ",
								sum, lo, hi);
	}
}

/*
 * msg:
 *	Display a message at the top of the screen.
 */
char		Msgbuf[BUFSIZ] = { '\0' };

static int	Newpos = 0;

/* VARARGS1 */
void
msg(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    doadd(fmt, args);
    va_end(args);
    endmsg(TRUE);
}

/*
 * addmsg:
 *	Add things to the current message
 */
/* VARARGS1 */
void
addmsg(char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    doadd(fmt, args);
    va_end (args);
}

/*
 * endmsg:
 *	Display a new msg.
 */

void
endmsg(BOOLEAN newline)
{
    int		linelen, msglen, len;
    char	*mp;
    
    linelen = UIGetMessageSize ();
    msglen = strlen (Msgbuf);
    mp = Msgbuf;
    do 
    {
	len = msglen;
	if (msglen > linelen)
	{
	    for (len = linelen; len >= 0; len--)
		if (mp[len] == ' ')
		    break;
	    while (mp[len] == ' ')
		len++;
	    mp[len-1] = '\0';
	}
	UIMessage (mp, newline);
	newline = TRUE;
	mp += len;
	msglen -= len;
    } while (msglen);
    Newpos = 0;
}


/*
 * doadd:
 *	Perform an add onto the message buffer
 */
void
doadd(char *fmt, va_list args)
{
    vsprintf (&Msgbuf[Newpos], fmt, args);
    Newpos = strlen(Msgbuf);
}

/*
 * getline:
 *      Reads the next line up to '\n' or EOF.  Multiple spaces are
 *	compressed to one space; a space is inserted before a ','
 */
char *
getline(void)
{
    UIReadLine (linebuf, LINESIZE);
    return linebuf;
}

/*
 * quit:
 *	Leave the program, cleaning things up as we go.
 */

void
quit(int status)
{
    UIFinish ();
    exit(status);
}
