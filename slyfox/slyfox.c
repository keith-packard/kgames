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
 * Author:  Dave Lemke, Network Computing Devices
 *
 */

#include	<stdlib.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/Shell.h>
#include	<X11/Xos.h>
#include	<Xkw/KCommand.h>
#include	<X11/Xaw/Box.h>
#include	<X11/Xaw/Dialog.h>
#include	<Xkw/KLabel.h>
#include	<Xkw/KMenuButton.h>
#include	<Xkw/KSimpleMenu.h>
#include	<Xkw/KSmeBSB.h>
#include	<X11/Xaw/AsciiText.h>
#include	<X11/Xaw/Cardinals.h>
#include	<Xkw/Cards.h>
#include	<Xkw/Layout.h>
#include	<X11/Xutil.h>
#include	<Xkw/CardsUtil.h>
#include	<Xkw/Message.h>
#include	"KSlyFox-res.h"

Widget      toplevel;
Widget      frame;
Widget      deck;
Widget      draw;
Widget      aces;
Widget      kings;
Widget      row1,
            row2,
            row3,
            row4;
Widget      message;
Widget      deckCount;
Widget      reserveCount;
Widget      menuBar;
Widget      fileMenuButton;
Widget      fileMenu;
Widget      newGame;
Widget      deal;
Widget      undo;
Widget      hint;
Widget      score;
Widget      gameState;

#define	NUM_ROWS	4
#define	NUM_COLS	5

#define NUM_ACES 4
#define NUM_KINGS 4
#define NUM_CARDS   104

CardStackRec deckStack;
CardStackRec drawStack;
CardStackRec tmpStack;
CardStackRec stackStacks[NUM_ROWS * NUM_COLS];
CardStackRec aceStacks[NUM_ACES],
            kingStacks[NUM_KINGS];

CardRec     rawcards[NUM_CARDS];

#define	RESERVE_COUNT	20

#define	GAME_DEAL	1
#define	GAME_RESERVE	2

int         game_state;
int         reserveCountNumber;

#define	STACK_TYPE_ACE		1
#define	STACK_TYPE_KING		2
#define	STACK_TYPE_RESERVE	3
#define	STACK_TYPE_DEAL		4
#define	STACK_TYPE_DRAW		5

CardStackPtr fromStack;
CardPtr     fromCard;
int         dealNumber;
Bool        initialDeal;
Bool        hard = False;

static void
ShowGameState(void);

static void
ShowReserveCount(void);

static void
ShowDealNumber(void);

typedef struct _slyfoxResources {
    int         animationSpeed;
}           SlyFoxResources, *SlyFoxResourcesPtr;

SlyFoxResources slyfoxResources;

static void
InitStacks (void)
{
    int         col;
    int         row;
    CardStackPtr stack;
    Widget      w;
    int         dt;

    if (hard)
	dt = CardDisplayTop;
    else
	dt = CardDisplaySome;
    for (row = 0, stack = stackStacks; row < NUM_ROWS; row++) {
	switch (row) {
	case 0:
	    w = row1;
	    break;
	case 1:
	    w = row2;
	    break;
	case 2:
	    w = row3;
	    break;
	case 3:
	    w = row4;
	    break;
	}
	for (col = 0; col < NUM_COLS; col++, stack++) {
	    CardInitStack(stack, w, CardsNone, False, col, dt);
	    stack->basePosition = 0;
	}
    }
    for (col = 0; col < NUM_ACES; col++) {
	CardInitStack(&aceStacks[col],
		      aces, CardsEmpty, False, 0, CardDisplayTop);
	aceStacks[col].basePosition = col;
    }
    for (col = 0; col < NUM_KINGS; col++) {
	CardInitStack(&kingStacks[col],
		      kings, CardsEmpty, False, 0, CardDisplayTop);
	kingStacks[col].basePosition = col;
    }
    CardInitStack(&deckStack,
		  deck, CardsEmpty, False, 0, CardDisplayBottom);
    CardInitStack(&drawStack,
		  draw, CardsEmpty, False, 0, CardDisplayBottom);
    CardInitStack(&tmpStack,
		  NULL, CardsEmpty, False, 0, CardDisplayBottom);
}

static void
GenerateCards (void)
{
    CardPtr     card;

    card = rawcards;
    CardGenerateStandardDeck(card);
    card += 52;
    CardGenerateStandardDeck(card);

    rawcards[51].next = &rawcards[52];
    rawcards[52].prev = &rawcards[51];
    deckStack.first = &rawcards[0];
    deckStack.last = &rawcards[NUM_CARDS - 1];
}

static void
GetKingsAndAces(void)
{
    CardPtr     card;
    int         i,
                na = 0,
                nk = 0;
    CardsSuit   aces_found[NUM_ACES],
                kings_found[NUM_KINGS];

    /* walk thru deck, pulling out aces & kings, tossing rest on tmp */
    while ((card = deckStack.last)) {
	if (card->card.rank == CardsAce) {
	    for (i = 0; i < na; i++) {
		if (aces_found[i] == card->card.suit)
		    goto other;
	    }
	    if (i == na) {
		aces_found[na] = card->card.suit;
		CardMove(&deckStack, card, &aceStacks[na], False);
		CardTurn(card, CardFaceUp, False);
		na++;
	    }
	} else if (card->card.rank == CardsKing) {
	    for (i = 0; i < nk; i++) {
		if (kings_found[i] == card->card.suit)
		    goto other;
	    }
	    if (i == nk) {
		kings_found[nk] = card->card.suit;
		CardMove(&deckStack, card, &kingStacks[nk], False);
		CardTurn(card, CardFaceUp, False);
		nk++;
	    }
	} else {
    other:
	    CardMove(&deckStack, card, &tmpStack, False);
	}
    }

    /* move tmp back to deck */
    while ((card = tmpStack.last)) {
	CardMove(&tmpStack, card, &deckStack, False);
    }
}

static void
FirstDeal (void)
{
    int         row,
                col;
    CardStackPtr stack;

    GetKingsAndAces();
    for (row = 0, stack = stackStacks; row < NUM_ROWS; row++) {
	for (col = 0; col < NUM_COLS; col++, stack++) {
	    CardMove(&deckStack, deckStack.last, stack, False);
	    CardTurn(stack->last, CardFaceUp, False);
	}
    }
    dealNumber = (52 * 2) - (NUM_ROWS * NUM_COLS) - 8;
    ShowDealNumber();
    game_state = GAME_DEAL;
    ShowGameState();
    reserveCountNumber = 0;
    ShowReserveCount();
    initialDeal = True;
}

/*
 * Compute a somewhat arbitrary evaluation function for the position:
 *    2 point per card sitting atop next higher card in same suit
 *   10 per card turned face up
 *   15 extra for each column where all cards have been revealed
 *   50 per completed suit removed (note this costs 12*2 for cards in seq)
 * If all columns are either empty or contain completed suits, then those
 * suits also count 50 (including the 24 for the 12 cards that are atop
 * higher cards), plus an extra 2 for each suit after the first three.
 * Thus the only way to get 1000 points is to win with all eight suits
 * still in the tableau.
 */

static int
ComputeScore (void)
{
    int         score = 0;
    int         i;
    CardStackPtr stack;
    CardPtr     card;

    /*
     * just count all the cards in the foundations.  pretty wimpy, but it
     * works
     */
    for (i = 0; i < NUM_ACES; i++) {
	stack = &aceStacks[i];
	card = stack->last;
	while (card) {
	    score++;
	    card = card->prev;
	}
    }
    for (i = 0; i < NUM_KINGS; i++) {
	stack = &kingStacks[i];
	card = stack->last;
	while (card) {
	    score++;
	    card = card->prev;
	}
    }

    return (score);
}

static void
DisplayStacks (void)
{
    int         col;
    CardPtr     card;
    CardStackPtr stack;
    int         viscount;

    CardDisplayStack(&drawStack);
    CardDisplayStack(&deckStack);

    for (col = 0; col < NUM_ACES; col++)
	CardDisplayStack(&aceStacks[col]);
    for (col = 0; col < NUM_KINGS; col++)
	CardDisplayStack(&kingStacks[col]);

    for (col = 0; col < (NUM_ROWS * NUM_COLS); col++) {
	stack = &stackStacks[col];
	if (!hard) {
	    viscount = 0;
	    for (card = stack->last; card;) {
		if (viscount++ < 6)
		    card->shouldBeUp = True;
		else
		    card->shouldBeUp = False;
		card = card->prev;
	    }
	}
	CardDisplayStack(stack);
    }
    CardsUpdateDisplay(deck);
    CardsUpdateDisplay(draw);
    CardsUpdateDisplay(aces);
    CardsUpdateDisplay(kings);
    CardsUpdateDisplay(row1);
    CardsUpdateDisplay(row2);
    CardsUpdateDisplay(row3);
    CardsUpdateDisplay(row4);
}

/* User interface functions */

static void
ShowGameState(void)
{
    if (game_state == GAME_DEAL) {
	Message(gameState, "Game state is Deal.");
    } else if (game_state == GAME_RESERVE) {
	Message(gameState, "Game state is Reserve.");
    }
}

static void
ShowReserveCount(void)
{
    Message(reserveCount, "Played to Reserve: %d", reserveCountNumber);
}

static void
ShowDealNumber(void)
{
    Message(deckCount, "Cards left: %d", dealNumber);
}

static void
ResetReserveCount(void *closure)
{
    reserveCountNumber = (int) (intptr_t) closure;
    ShowReserveCount();
}

static void
ResetDealNumber(void *closure)
{
    dealNumber = (int) (intptr_t) closure;
    ShowDealNumber();
}

static void
ResetGameState(void *closure)
{
    game_state = (int) (intptr_t) closure;
    ShowGameState();
}

static void
Deal(Bool autoplay)
{
    if (!deckStack.last) {
	if (!autoplay) {
	    Message(message, "No more cards in the deck.");
	}
	return;
    }
    if (drawStack.last) {
	if (!autoplay) {
	    Message(message, "Must play %P to reserve first.",
		    &drawStack.last->card);
	}
	return;
    }
    CardMove(&deckStack, deckStack.last, &drawStack, True);
    CardTurn(drawStack.last, CardFaceUp, True);
    CardRecordHistoryCallback(ResetDealNumber, (void *) (intptr_t) dealNumber);
    --dealNumber;
    ShowDealNumber();
}

static void
FillSpace(CardStackPtr to_stack)
{
    if (!initialDeal)		/* only do this for the first deal */
	return;
    if (!deckStack.last)
	return;
    if (to_stack->last)		/* only if its a space */
	return;
    CardMove(&deckStack, deckStack.last, to_stack, True);
    CardTurn(to_stack->last, CardFaceUp, True);
    CardRecordHistoryCallback(ResetDealNumber, (void *) (intptr_t) dealNumber);
    --dealNumber;
    ShowDealNumber();
}

static void
NewGame (void)
{
    CardsRemoveAllCards(deck);
    CardsRemoveAllCards(aces);
    CardsRemoveAllCards(kings);
    CardsRemoveAllCards(row1);
    CardsRemoveAllCards(row2);
    CardsRemoveAllCards(row3);
    CardsRemoveAllCards(row4);
    CardsRemoveAllCards(draw);
    fromStack = 0;
    fromCard = 0;
    InitStacks();
    GenerateCards();
    CardShuffle(&deckStack, False);
    FirstDeal();
    CardInitHistory();
    DisplayStacks();
}

static void
Undo (void)
{
    if (!CardUndo())
	Message(message, "Nothing to undo.");
    DisplayStacks();
}

static void
Score (void)
{
    Message(message, "Current position scores %d out of 104.",
	    ComputeScore());
}

static void
Quit (void)
{
    exit(0);
}

CardStackPtr
FindFinishPlay(from_card)
    CardPtr     from_card;
{
    int         i;
    CardStackPtr to_stack;

    for (i = 0; i < NUM_ACES; i++) {
	to_stack = &aceStacks[i];
	if (to_stack->last && CardIsInSuitOrder(to_stack->last, from_card)) {
	    return to_stack;
	}
    }
    for (i = 0; i < NUM_KINGS; i++) {
	to_stack = &kingStacks[i];
	if (to_stack->last && CardIsInSuitOrder(from_card, to_stack->last)) {
	    return to_stack;
	}
    }
    return NULL;
}

void
Play(from_stack, from_card, to_stack, to_type)
    CardStackPtr from_stack;
    CardPtr     from_card;
    CardStackPtr to_stack;
    int         to_type;
{
    switch (game_state) {
    case GAME_DEAL:
	if (to_stack == from_stack) {	/* single click */
	    if (to_type == STACK_TYPE_ACE || to_type == STACK_TYPE_KING) {
		Message(message, "Can't move %P.", &from_card->card);
		return;
	    }
	    if ((to_stack = FindFinishPlay(from_card))) {
		CardMove(from_stack, from_card, to_stack, True);
		FillSpace(from_stack);
	    } else {
		Message(message, "No place to play the %P.",
			&from_card->card);
	    }
	} else if (to_type == STACK_TYPE_ACE) {
	    if (CardIsInSuitOrder(to_stack->last, from_card)) {
		CardMove(from_stack, from_card, to_stack, True);
		FillSpace(from_stack);
	    } else {
		Message(message, "Can't play %P on %P.",
			&from_card->card, &to_stack->last->card);
	    }
	} else if (to_type == STACK_TYPE_KING) {
	    if (CardIsInSuitOrder(from_card, to_stack->last)) {
		CardMove(from_stack, from_card, to_stack, True);
		FillSpace(from_stack);
	    } else {
		Message(message, "Can't play %P on %P.",
			&from_card->card, &to_stack->last->card);
	    }
	} else if (to_type == STACK_TYPE_RESERVE) {
	    Message(message, "Can't move cards around in reserve.");
	}
	break;
    case GAME_RESERVE:
	if ((from_stack != to_stack) && (from_stack != &drawStack)) {
	    Message(message, "Must play from waste.");
	    return;
	}
	if (to_stack == from_stack) {
	    from_stack = &drawStack;
	    from_card = drawStack.last;
	    if (to_type == STACK_TYPE_DRAW) {
		if (!(to_stack = FindFinishPlay(from_card))) {
		    Message(message, "I'm not going to guess.");
		    return;
		}
	    } else if (to_type == STACK_TYPE_RESERVE) {
		if (!drawStack.last) {
		    Message(message, "Deal next card first.");
		    return;
		}
	    } else if (to_type == STACK_TYPE_ACE) {
		if (!CardIsInSuitOrder(to_stack->last, from_card)) {
		    Message(message, "Can't move %P onto %P.",
			    &from_card->card, &to_stack->last->card);
		    return;
		}
	    } else if (to_type == STACK_TYPE_KING) {
		if (!CardIsInSuitOrder(from_card, to_stack->last)) {
		    Message(message, "Can't move %P onto %P.",
			    &from_card->card, &to_stack->last->card);
		    return;
		}
	    }
	}
	CardMove(from_stack, from_card, to_stack, True);
	if (to_type == STACK_TYPE_RESERVE) {
	    CardRecordHistoryCallback(ResetReserveCount, (void *) (intptr_t) reserveCountNumber);
	    ++reserveCountNumber;
	    ShowReserveCount();
	}
	if ((reserveCountNumber == RESERVE_COUNT) ||
		(!deckStack.last && !drawStack.last)) {
	    CardRecordHistoryCallback(ResetReserveCount, (void *) (intptr_t) reserveCountNumber);
	    CardRecordHistoryCallback(ResetGameState, (void *) (intptr_t) game_state);
	    reserveCountNumber = 0;
	    game_state = GAME_DEAL;
	    ShowGameState();
	} else {
	    Deal(True);		/* place next card for them */
	}
	break;
    }
    DisplayStacks();
}

static void
StartDeal(void)
{
    initialDeal = False;
    Message(message, "");
    CardRecordHistoryCallback(ResetGameState, (void *) (intptr_t) game_state);
    game_state = GAME_RESERVE;
    ShowGameState();
    Deal(False);
    CardNextHistory();
    DisplayStacks();
}

static void
FindAMove (void)
{
    int         row,
                col;
    CardStackPtr stack,
                to_stack;
    CardPtr     card;

    if (game_state == GAME_RESERVE) {
	Message(message, "No clue.");
	return;
    }
    for (row = 0, stack = stackStacks; row < NUM_ROWS; row++) {
	for (col = 0; col < NUM_COLS; col++, stack++) {
	    card = stack->last;
	    if (!card)
		continue;
	    to_stack = FindFinishPlay(card);
	    if (to_stack) {
		Message(message, "Move %P to %P.",
			&card->card, &to_stack->last->card);
		return;
	    }
	}
    }
    if (deckStack.last)
	Message(message, "Time to start dealing.");
    else
	Message(message, "Its all over.");
}

static void
Restore (void)
{
    Message(message, "Restore not implemented");
}

static void
Save (void)
{
    Message(message, "Save not implemented");
}

static void
Expand(CardStackPtr stack)
{
    CardPtr     card,
                t;

    if ((card = stack->first)) {
	MessageStart();
	MessageAppend("Column contains:");
	while (card) {
	    if (card->face == CardFaceUp) {
		MessageAppend(" %p", &card->card);
		t = CardInSuitOrder(card);
		if (t != card && t != card->next) {
		    card = t;
		    MessageAppend("-%p", &card->card);
		}
	    }
	    card = card->next;
	}
	MessageAppend(".");
	MessageEnd(message);
    } else
	Message(message, "Column is empty");
}

/* Callbacks to user interface functions */

static void
DeckCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
#ifdef old
    Message(message, "");
    if (game_state != GAME_RESERVE) {
	Message(message, "Hit Deal to change game state to Reserve.");
	return;
    }
    Deal(False);
    CardNextHistory();
    DisplayStacks();
#else
    StartDeal();
#endif
}

static void
StackCallback (Widget w, XtPointer closure, XtPointer data)
{
    CardsInputPtr input = (CardsInputPtr) data;
    CardStackPtr stack;
    CardPtr     card;
    String      type;
    int         to_type;

    Message(message, "");
    if (w == row1) {
	stack = &stackStacks[input->col];
	to_type = STACK_TYPE_RESERVE;
    } else if (w == row2) {
	stack = &stackStacks[NUM_COLS + input->col];
	to_type = STACK_TYPE_RESERVE;
    } else if (w == row3) {
	stack = &stackStacks[2 * NUM_COLS + input->col];
	to_type = STACK_TYPE_RESERVE;
    } else if (w == row4) {
	stack = &stackStacks[3 * NUM_COLS + input->col];
	to_type = STACK_TYPE_RESERVE;
    } else if (w == draw) {
	stack = &drawStack;
	to_type = STACK_TYPE_DEAL;
    } else if (w == aces) {
	stack = &aceStacks[input->row];
	to_type = STACK_TYPE_ACE;
    } else if (w == kings) {
	stack = &kingStacks[input->row];
	to_type = STACK_TYPE_KING;
    }
    card = stack->last;
    if (*input->num_params) {
	type = *input->params;
	if (!strcmp(type, "source")) {
	    if (game_state == GAME_DEAL) {
		if (to_type == STACK_TYPE_ACE || to_type == STACK_TYPE_KING) {
		    Message(message, "Can't move %P.",
			    &card->card);
		    return;
		}
	    }
	    fromStack = stack;
	    if (fromStack->last)
		fromCard = fromStack->last;
	    else if (game_state == GAME_DEAL) {
		Message(message, "Selected stack is empty.");
	    }
	} else if (!strcmp(type, "dest")) {
	    if (fromCard || game_state == GAME_RESERVE) {
		Play(fromStack, fromCard, stack, to_type);
		fromCard = NULL;
		CardNextHistory();
		DisplayStacks();
	    }
	} else if (!strcmp(type, "expand")) {
	    Expand(stack);
	}
    }
}


static void
NewGameCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    NewGame ();
}

static void
QuitCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Quit ();
}

static void
ScoreCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Score ();
}

static void
UndoCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Undo ();
}

static void
FindAMoveCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    FindAMove ();
}

static void
RestoreCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Restore ();
}

static void
SaveCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Save ();
}

static void
StartDealCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    StartDeal ();
}

/* actions to user interface functions */

static void UndoAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Undo ();
}

static void NewGameAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    NewGame ();
}

static void ScoreAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Score ();
}

static void QuitAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Quit ();
}

static void FindAMoveAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    FindAMove ();
}

static void RestoreAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Restore ();
}

static void SaveAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Save ();
}

static void StartDealAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    StartDeal ();
}

XtActionsRec actions[] = {
    { "slyfoxUndo", UndoAction, },
    { "slyfoxNewGame", NewGameAction, },
    { "slyfoxScore", ScoreAction, },
    { "slyfoxQuit", QuitAction, },
    { "slyfoxFindAMove", FindAMoveAction, },
    { "slyfoxRestore", RestoreAction, },
    { "slyfoxSave", SaveAction, },
    { "slyfoxStartDeal", StartDealAction, },
};

struct menuEntry {
    char       *name;
    void    (*function)(Widget w, XtPointer closure, XtPointer data);
};

struct menuEntry fileMenuEntries[] = {
    { "restore", RestoreCallback, },
    { "save", SaveCallback, },
    { "quit", QuitCallback, },
};

static Widget
CreateMenu (Widget parent, char *name, struct menuEntry *entries, int count)
{
    Widget      menu;
    Widget      entry;
    int         i;

    menu = XtCreatePopupShell(name, ksimpleMenuWidgetClass,
			      parent, NULL, ZERO);
    for (i = 0; i < count; i++) {
	entry = XtCreateManagedWidget(entries[i].name,
				      ksmeBSBObjectClass, menu, NULL, ZERO);
	XtAddCallback(entry, XtNcallback, entries[i].function, NULL);
    }
    return menu;
}

#define offset(field) XtOffsetOf(SlyFoxResources, field)

XtResource  resources[] = {
    {"animationSpeed", "AnimationSpeed", XtRInt, sizeof(int),
    offset(animationSpeed), XtRImmediate, (XtPointer) - 1},
};

XrmOptionDescRec options[] = {
    { "-smallCards",	"*Cards.smallCards",	XrmoptionNoArg, "True", },
    { "-squareCards",	"*Cards.roundCards",	XrmoptionNoArg, "False", },
    { "-noanimate",	"*animationSpeed",	XrmoptionNoArg, "0", },
    { "-animationSpeed",	"*animationSpeed",	XrmoptionSepArg, NULL },
};

int
main (int argc, char **argv)
{
    Atom        wm_delete_window;
    char       *adname = "KSlyFox";
    int         i;

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-hard")) {
	    adname = "KSlyFox-hard";
	    hard = True;
	}
    }

    toplevel = XkwInitialize(adname, options, XtNumber(options),
			     &argc, argv, True, defaultResources);

    Arg	args[1];
    XtSetArg(args[0], XtNinput, True);
    XtSetValues(toplevel, args, ONE);

    XtGetApplicationResources(toplevel, (XtPointer) & slyfoxResources, resources,
			      XtNumber(resources), NULL, 0);

    AnimateSetSpeed(slyfoxResources.animationSpeed);

    XtAddActions(actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable("<Message>WM_PROTOCOLS: slyfoxQuit()"));
    frame = XtCreateManagedWidget("frame", layoutWidgetClass, toplevel, NULL, 0);
    menuBar = XtCreateManagedWidget("menuBar", layoutWidgetClass, frame, NULL, 0);
    fileMenuButton = XtCreateManagedWidget("fileMenuButton",
					   kmenuButtonWidgetClass,
					   menuBar, NULL, ZERO);
    fileMenu = CreateMenu(fileMenuButton, "fileMenu",
			  fileMenuEntries, XtNumber(fileMenuEntries));
    newGame = XtCreateManagedWidget("newGame", kcommandWidgetClass,
				    menuBar, NULL, ZERO);
    XtAddCallback(newGame, XtNcallback, NewGameCallback, NULL);
    deal = XtCreateManagedWidget("deal", kcommandWidgetClass,
				 menuBar, NULL, ZERO);
    XtAddCallback(deal, XtNcallback, StartDealCallback, NULL);
    undo = XtCreateManagedWidget("undo", kcommandWidgetClass,
				 menuBar, NULL, ZERO);
    XtAddCallback(undo, XtNcallback, UndoCallback, NULL);
    hint = XtCreateManagedWidget("hint", kcommandWidgetClass,
				 menuBar, NULL, ZERO);
    XtAddCallback(hint, XtNcallback, FindAMoveCallback, NULL);
    score = XtCreateManagedWidget("score", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(score, XtNcallback, ScoreCallback, NULL);

    deck = XtCreateManagedWidget("deck", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(deck, XtNinputCallback, DeckCallback, NULL);
    deckCount = XtCreateManagedWidget("deckCount", klabelWidgetClass, frame, NULL, 0);
    reserveCount = XtCreateManagedWidget("reserveCount", klabelWidgetClass, frame, NULL, 0);
    draw = XtCreateManagedWidget("draw", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(draw, XtNinputCallback, StackCallback, NULL);
    gameState = XtCreateManagedWidget("gameState", klabelWidgetClass, frame, NULL, 0);

    aces = XtCreateManagedWidget("aces", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(aces, XtNinputCallback, StackCallback, NULL);
    kings = XtCreateManagedWidget("kings", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(kings, XtNinputCallback, StackCallback, NULL);
    row1 = XtCreateManagedWidget("row1", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(row1, XtNinputCallback, StackCallback, NULL);
    row2 = XtCreateManagedWidget("row2", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(row2, XtNinputCallback, StackCallback, NULL);
    row3 = XtCreateManagedWidget("row3", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(row3, XtNinputCallback, StackCallback, NULL);
    row4 = XtCreateManagedWidget("row4", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(row4, XtNinputCallback, StackCallback, NULL);

    message = XtCreateManagedWidget("message", klabelWidgetClass, frame, NULL, 0);

    srandom(getpid() ^ time((long *) 0));
    NewGame();
    XtRealizeWidget(toplevel);
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols(XtDisplay(toplevel), XtWindow(toplevel),
			   &wm_delete_window, 1);

    XtMainLoop();
}
