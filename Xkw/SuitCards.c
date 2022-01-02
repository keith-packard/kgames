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
# include	<X11/Xutil.h>
# include	<Xkw/Cards.h>
# include	<Xkw/SuitCards.h>

void
SuitCardsInit (SuitCardsPtr	s,
	       CardStackPtr	under,
	       Widget		widget,
	       CardsSuit	emptySuit,
	       Boolean		horizontal,
	       int		row,
	       int		col,
	       CardDisplay	display)
{
    int		    position;
    int		    basePosition;
    CardsSuit	    suit;
    CardStackPtr    stack;

    (void) under;
    if (horizontal)
    {
	position = col;
	basePosition = row;
    }
    else
    {
	position = row;
	basePosition = col;
    }
    for (suit = CardsClub; suit <= CardsSpade; suit++)
    {
	stack = &s->suits[CardsSuitToInt(suit)];
	CardInitStack (stack, widget, emptySuit,
		       !horizontal, position, display);
	stack->basePosition = basePosition;
	position++;
    }
    s->aceHigh = True;
}

static Boolean
SuitRankGreater (CardsRank a, CardsRank b, Boolean aceHigh)
{
    if (a == b)
	return False;
    if (a == CardsAce)
	return aceHigh;
    if (b == CardsAce)
	return !aceHigh;
    return a > b;
}

static void
SuitCardsMoveFromStack (CardStackPtr	from_stack,
			CardPtr	   	card,
			SuitCardsPtr    to_suit,
			Boolean		remember)
{
    CardStackPtr    to_stack;
    CardPtr	    to_card, next_card;
    CardsSuit	    suit;
    CardsRank	    rank;

    suit = card->card.suit;
    rank = card->card.rank;
    to_stack = &to_suit->suits[CardsSuitToInt(suit)];
    to_card = 0;
    for (next_card = to_stack->first; next_card; next_card = next_card->next)
    {
	if (SuitRankGreater (rank, next_card->card.rank, to_suit->aceHigh))
	    break;
	to_card = next_card;
    }
    CardMoveCards (from_stack, card, card, to_stack, to_card, remember);
}

void
SuitCardsMove (SuitCardsPtr	from_suit,
	       CardPtr	    	card,
	       SuitCardsPtr	to_suit,
	       Boolean		remember)
{
    CardStackPtr    from_stack;
    CardsSuit	    suit;

    suit = card->card.suit;
    from_stack = &from_suit->suits[CardsSuitToInt(suit)];
    SuitCardsMoveFromStack (from_stack, card, to_suit, remember);
}

void
SuitCardsMoveToStack (SuitCardsPtr	from_suit,
		      CardPtr		card,
		      CardStackPtr	to_stack,
		      CardPtr		to_card,
		      Boolean		remember)
{
    CardStackPtr    from_stack;
    CardsSuit	    suit;

    suit = card->card.suit;
    from_stack = &from_suit->suits[CardsSuitToInt(suit)];
    CardMoveCards (from_stack, card, card, to_stack, to_card, remember);
}

void
SuitCardsDisplay (SuitCardsPtr s)
{
    CardsSuit	suit;

    for (suit = CardsClub; suit <= CardsSpade; suit++)
	CardDisplayStack (&s->suits[CardsSuitToInt(suit)]);
}

CardPtr
SuitCardsHandInputToCard (SuitCardsPtr s, HandInputPtr input)
{
    CardsSuit	    suit;
    int		    suitPosition;
    CardStackPtr    stack;
    CardPtr	    card;

    if (s->suits[CardsClub].horizontal)
	suitPosition = input->current.row;
    else
	suitPosition = input->current.col;
    suit = IntToCardsSuit (suitPosition - s->suits[CardsClub].position);
    stack = &s->suits[suit];
    for (card = stack->last; card; card = card->prev)
	if (card->shouldBeUp &&
	    card->row == input->current.row && card->col == input->current.col)
	    return card;
    return 0;
}
