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

#include    <Xkw/CardsUtil.h>

typedef struct _SuitCards {
    Boolean	    aceHigh;
    CardStackRec    suits[4];
} SuitCardsRec, *SuitCardsPtr;

CardPtr
SuitCardsHandInputToCard (SuitCardsPtr, HandInputPtr);

void
SuitCardsInit (SuitCardsPtr	s,
	       CardStackPtr	under,
	       Widget		widget,
	       CardsSuit	emptySuit,
	       Boolean		horizontal,
	       int		row,
	       int		col,
	       CardDisplay	display);

void
SuitCardsDisplay (SuitCardsPtr s);

void
SuitCardsMove (SuitCardsPtr	from_suit,
	       CardPtr	    	card,
	       SuitCardsPtr	to_suit,
	       Boolean		remember);

void
SuitCardsMoveToStack (SuitCardsPtr	from_suit,
		      CardPtr		card,
		      CardStackPtr	to_stack,
		      CardPtr		to_card,
		      Boolean		remember);

CardPtr
SuitCardsHandInputToCard (SuitCardsPtr s, HandInputPtr input);
