/*
 * $NCD$
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */

# include	<X11/Intrinsic.h>
# include	<X11/StringDefs.h>
# include	<X11/Xos.h>
# include	<X11/Xaw/Label.h>
# include	<X11/Xaw/Cardinals.h>
# include	"Cards.h"
# include	<X11/Xutil.h>
# include	<stdarg.h>
# include	<stdio.h>
# include	"Message.h"

char *
CardsSuitName (CardsSuit s)
{
    switch (s) {
    case CardsSpade:
	return "Spade";
    case CardsHeart:
	return "Heart";
    case CardsDiamond:
	return "Diamond";
    case CardsClub:
	return "Club";
    default:
	return "???";
    }
}

char *
CardsRankName (CardsRank r)
{
    switch (r) {
    case CardsAce:
	return "Ace";
    case Cards2:
	return "Deuce";
    case Cards3:
	return "Three";
    case Cards4:
	return "Four";
    case Cards5:
	return "Five";
    case Cards6:
	return "Six";
    case Cards7:
	return "Seven";
    case Cards8:
	return "Eight";
    case Cards9:
	return "Nine";
    case Cards10:
	return "Ten";
    case CardsJack:
	return "Jack";
    case CardsQueen:
	return "Queen";
    case CardsKing:
	return "King";
    default:
	return "???";
    }
}

static char *
MessageCard (char *s, CardsCardPtr c)
{
    if (c->suit > CardsSpade || c->rank > CardsKing)
        sprintf (s, "empty stack");
    else
        sprintf (s, "%s of %ss", CardsRankName (c->rank), CardsSuitName (c->suit));
    return s + strlen(s);
}

static char *
MessageShortCard (char *s, CardsCardPtr c)
{
    char    suit;

    suit = *CardsSuitName(c->suit) - ('A' - 'a');
    if (Cards2 <= c->rank && c->rank <= Cards10)
	sprintf (s, "%d%c", c->rank, suit);
    else
	sprintf (s, "%c%c", *CardsRankName(c->rank), suit);
    return s + strlen(s);
}

static char *
MessageInt (char *s, int i)
{
    sprintf (s, "%d", i);
    return s + strlen(s);
}

static char MessageBuffer[1024];
static char *MessagePtr;

void
MessageAppendV (const char *format, va_list args)
{
    char	*m;

    m = MessagePtr;
    while (*format) {
	if (*format == '%') switch (*++format) {
	case 's':
	    strcpy (m, va_arg(args, char *));
	    m += strlen (m);
	    break;
	case 'c':
	    *m++ = va_arg(args, int);
	    break;
	case 'd':
	    m = MessageInt (m, va_arg(args, int));
	    break;
	case 'p':
	    m = MessageShortCard (m, va_arg(args, CardsCardPtr));
	    break;
	case 'P':
	    m = MessageCard (m, va_arg(args, CardsCardPtr));
	    break;
	default:
	    *m++ = *format;
	} else
	    *m++ = *format;
	format++;
    }
    MessagePtr = m;
}

void
MessageStart (void)
{
    MessagePtr = MessageBuffer;
}

void
MessageEnd (Widget w)
{
    Arg	arg[1];

    *MessagePtr = '\0';
    XtSetArg (arg[0], XtNlabel, MessageBuffer);
    XtSetValues (w, arg, 1);
}

void
MessageAppend (const char *format, ...)
{
    va_list args;

    va_start (args, format);
    MessageAppendV (format, args);
    va_end (args);
}

void
MessageV (Widget w, const char *format, va_list args)
{
    MessageStart ();
    MessageAppendV (format, args);
    MessageEnd (w);
}

void
Message (Widget w, const char *format, ...)
{
    va_list args;

    va_start (args, format);
    MessageV(w, format, args);
    va_end (args);
}
