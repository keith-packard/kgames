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

#ifndef _CARDS_UTIL_H_
#define _CARDS_UTIL_H_
#include "Xkw/Animate.h"
#include "Xkw/Cards.h"

typedef struct _Card *CardPtr;

typedef enum _CardFace { CardFaceUp, CardFaceDown } CardFace;

typedef enum _CardDisplay { 
    CardDisplayTop, CardDisplayBottom, CardDisplayAll, 
    CardDisplaySome, CardDisplayNone
} CardDisplay;

typedef struct _Card {
    CardPtr	    next, prev;
    CardsCardRec    card;
    CardsCardRec    display;
    CardFace	    face;
    Boolean	    shouldBeUp;
    Boolean	    isUp;
    Widget	    widget;
    int		    row, col;
    XtPointer	    data;
} CardRec;

typedef struct _CardStack  *CardStackPtr;

typedef struct _CardStack {
    Widget	    widget;
    Boolean	    horizontal;
    CardDisplay	    display;
    int		    position;
    int		    basePosition;
    CardPtr	    first, last;
    CardRec	    empty;
} CardStackRec;

#define New(t) (t *) malloc(sizeof (t))
#define Dispose(p)  free((char *) p)
#define Some(t,n)   (t*) malloc(sizeof(t) * n)
#define More(p,t,n) ((p)? (t *) realloc((char *) p, sizeof(t)*n):Some(t,n))

Boolean	CardIsInOrder (CardPtr, CardPtr);
Boolean	CardIsInSuitOrder (CardPtr, CardPtr);
Boolean	CardIsInAlternatingSuitOrder (CardPtr, CardPtr);
CardPtr	CardInOrder (CardPtr);
CardPtr	CardInSuitOrder (CardPtr);
CardPtr	CardInAlternatingSuitOrder (CardPtr);

CardPtr	CardInReverseOrder (CardPtr);
CardPtr	CardInReverseSuitOrder (CardPtr);
CardPtr	CardInReverseAlternatingSuitOrder (CardPtr);

void	CardDisplayStack (CardStackPtr);

void	CardTurn (CardPtr, CardFace, Boolean);
void	CardMove (CardStackPtr, CardPtr, CardStackPtr, Boolean);
void	CardMoveCards (CardStackPtr, CardPtr, CardPtr, CardStackPtr, CardPtr, Boolean);
void	CardRecordHistoryCallback (void (*)(), char *);
Boolean	CardUndo (void);

void	CardInitStack (CardStackPtr, Widget, CardsSuit, Boolean, Boolean, int);
void	CardGenerateStandardDeck (CardPtr);
void	CardShuffle (CardStackPtr);
void	CardInitHistory (void);
int	CardNextHistory (void);

#endif /* _CARDS_UTIL_H_ */
