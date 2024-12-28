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

# include	<stdlib.h>
# include	<X11/Intrinsic.h>
# include	<X11/StringDefs.h>
# include	<X11/Shell.h>
# include	<X11/Xos.h>
# include	<Xkw/KCommand.h>
# include	<X11/Xaw/Box.h>
# include	<X11/Xaw/AsciiText.h>
# include	<X11/Xaw/Cardinals.h>
# include	<Xkw/KLabel.h>
# include	<Xkw/KCommand.h>
# include	<Xkw/KMenuButton.h>
# include	<Xkw/KSimpleMenu.h>
# include	<Xkw/KSmeBSB.h>
# include	<Xkw/Cards.h>
# include	<Xkw/Layout.h>
# include	<X11/Xutil.h>
# include	<Xkw/CardsUtil.h>
# include	<Xkw/Message.h>
# include	"KSpider-res.h"

Widget	    toplevel;
Widget	    frame;
Widget	    deck;
Widget	    piles;
Widget	    stacks;
Widget	    message;
Widget	    menuBar;
Widget	    fileMenuButton;
Widget	    fileMenu;
Widget	    newGame;
Widget	    undo;
Widget	    hint;
Widget	    score;

#define NUM_STACKS  10
#define NUM_PILES   8
#define NUM_CARDS   104

CardStackRec	deckStack;
CardStackRec	stackStacks[NUM_STACKS];
CardStackRec	pileStacks[NUM_PILES];

CardRec		rawcards[NUM_CARDS];

CardStackPtr	fromStack;
CardPtr		fromCard;
int		dealNumber;

typedef struct _spiderResources {
    int		animationSpeed;
    Boolean    	squishCards;
} SpiderResources, *SpiderResourcesPtr;

SpiderResources spiderResources;

static void
InitStacks (void)
{
    int	    col;
    CardDisplay	    display;

    for (col = 0; col < NUM_STACKS; col++)
    {
	display = CardDisplayAll;
	if (spiderResources.squishCards)
	    display = CardDisplaySome;
	CardInitStack (&stackStacks[col],
		       stacks, CardsNone, False, col, display);
    }
    for (col = 0; col < NUM_PILES; col++)
    {
	CardInitStack (&pileStacks[col],
		       piles, CardsEmpty, False, col, CardDisplayTop);
    }
    CardInitStack (&deckStack,
		   deck, CardsEmpty, False, 0, CardDisplayBottom);
}

static void
GenerateCards (void)
{
    int		    i;
    CardPtr	    card;

    card = rawcards;
    for (i = 0; i < 2; i++)
    {
	CardGenerateStandardDeck (card);
	card += 52;
    }
    rawcards[51].next = &rawcards[52];
    rawcards[52].prev = &rawcards[51];
    deckStack.first = &rawcards[0];
    deckStack.last = &rawcards[NUM_CARDS-1];
}

#define FIRST_ROWS  6

static void
FirstDeal (void)
{
    int	    row, col;

    for (row = 0; row < FIRST_ROWS; row++)
    {
	for (col = 0; col < NUM_STACKS; col++)
	{
	    if (row == FIRST_ROWS-1 && col % 3 != 0)
		continue;
	    CardMove (&deckStack, deckStack.last, &stackStacks[col], False);
	}
    }
    for (col = 0; col < NUM_STACKS; col++)
	CardTurn (stackStacks[col].last, CardFaceUp, False);
    dealNumber = 0;
}

static void
CheckStackTop (CardStackPtr stack)
{
    if (stack->last && stack->last->face == CardFaceDown)
	CardTurn (stack->last, CardFaceUp, True);
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
    int		    score = 0;
    int		    col;
    CardStackPtr    stack;
    CardPtr	    card;
    int		    numPiles = 0;

    score = 44 * 10;		/* score if all cards flipped */

    for (col = 0; col < NUM_PILES; col++)
	if (pileStacks[col].first)
	    score += 50;

    for (col = 0; col < NUM_STACKS; col++)
    {
	stack = &stackStacks[col];
	if ((card = stack->first))
	{
	    if (card->face == CardFaceUp)
	    {
		score += 15;
		if (card->card.rank == CardsKing &&
		    CardInSuitOrder (card) == stack->last &&
		    stack->last->card.rank == CardsAce)
		{
		    score += 50;
		    numPiles++;
		    if (numPiles > 3)
			score += 2;
		    card = 0;
		}
	    }
	}
	else
	    score += 15;
	while (card)
	{
	    if (card->face == CardFaceUp)
	    {
		if (card->prev && CardIsInSuitOrder (card, card->prev))
		    score += 2;
	    }
	    else
		score -= 10;	/* still Facedown */
	    card = card->next;
	}
    }

    return (score);
}

static void
DisplayStacks (void)
{
    int		    col;
    CardPtr	    card, c;
    CardStackPtr    stack;

    CardDisplayStack (&deckStack);

    for (col = 0; col < NUM_PILES; col++)
	CardDisplayStack (&pileStacks[col]);

    for (col = 0; col < NUM_STACKS; col++)
    {
	stack = &stackStacks[col];
	if (spiderResources.squishCards)
	{
	    for (card = stack->last; card;)
	    {
		card->shouldBeUp = True;
		if (card->face == CardFaceDown)
		{
		    while (card->prev && card->prev->face == CardFaceDown)
		    {
			card->shouldBeUp = False;
			card = card->prev;
		    }
		}
		else
		{
		    c = CardInReverseSuitOrder (card);
		    if (c != card)
		    {
			while ((card = card->prev) != c)
			    card->shouldBeUp = False;
			card->shouldBeUp = True;
		    }
		}
		card = card->prev;
	    }
	}
	CardDisplayStack (stack);
    }
    CardsUpdateDisplay (deck);
    CardsUpdateDisplay (piles);
    CardsUpdateDisplay (stacks);
}

/* User interface functions */

static void
ResetDealNumber (void *closure)
{
    dealNumber = (int) (intptr_t) closure;
}

static void
Deal (void)
{
    int		    col;

    for (col = 0; col < NUM_STACKS; col++)
    {
	if (!deckStack.last)
	{
	    Message (message, "No more cards in the deck.");
	    return;
	}
	CardMove (&deckStack, deckStack.last, &stackStacks[col], True);
	CheckStackTop (&stackStacks[col]);
    }
    CardRecordHistoryCallback (ResetDealNumber, (char *) (intptr_t) dealNumber);
    Message (message, "Dealt hand %d of 5.", ++dealNumber);
}

static void
NewGame (void)
{
    CardsRemoveAllCards (deck);
    CardsRemoveAllCards (piles);
    CardsRemoveAllCards (stacks);
    fromStack = 0;
    fromCard = 0;
    InitStacks ();
    GenerateCards ();
    CardShuffle (&deckStack, False);
    FirstDeal ();
    CardInitHistory ();
    DisplayStacks ();
}

static void
Undo (void)
{
    if (!CardUndo ())
	Message (message, "Nothing to undo.");
    DisplayStacks ();
}

static void
Score (void)
{
    Message (message, "Current position scores %d out of 1000.",
	     ComputeScore ());
}

static void
Quit (void)
{
    exit (0);
}

static CardStackPtr
FindFinishPlay (CardStackPtr from_stack, CardPtr from_card)
{
    int		    i;
    CardStackPtr    to_stack;

    (void) from_stack;
    if (from_card->card.rank == CardsKing &&
	CardInSuitOrder (from_card)->card.rank == CardsAce)
    {
	for (i = 0; i < NUM_PILES; i++)
	{
	    to_stack = &pileStacks[i];
	    if (to_stack->last == NULL)
		return to_stack;
	}
    }
    return NULL;
}

static CardStackPtr
FindInSuitPlay (CardStackPtr from_stack, CardPtr from_card)
{
    int		    i;
    int		    col = from_stack - stackStacks;
    CardStackPtr    to_stack;
    CardPtr	    card;

    for (i = 0; i < NUM_STACKS; i++)
    {
	if (i == col)
	    continue;
	to_stack = &stackStacks[i];
	card = to_stack->last;
	if (card && CardIsInSuitOrder (from_card, card))
	    return to_stack;
    }
    return NULL;
}

static CardStackPtr
FindNotInSuitPlay (CardStackPtr from_stack, CardPtr from_card)
{
    int		    i;
    int		    col = from_stack - stackStacks;
    CardStackPtr    to_stack;
    CardPtr	    card;

    for (i = 0; i < NUM_STACKS; i++)
    {
	if (i == col)
	    continue;
	to_stack = &stackStacks[i];
	card = to_stack->last;
	if (card && CardIsInOrder (from_card, card))
	    return to_stack;
    }
    return NULL;
}

static CardStackPtr
FindEmptyPlay (CardStackPtr from_stack, CardPtr from_card)
{
    int		    i;
    int		    col = from_stack - stackStacks;
    CardStackPtr    to_stack;

    (void) from_card;
    for (i = 0; i < NUM_STACKS; i++)
    {
	if (i == col)
	    continue;
	to_stack = &stackStacks[i];
	if (to_stack->last == NULL)
	    return to_stack;
    }
    return NULL;
}

static void
Play (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    if (from_card->face == CardFaceDown)
    {
	Message (message, "Card not turned up.");
	return;
    }
    if (CardInSuitOrder (from_card)->next != NULL)
    {
	Message (message, "Cards not in order.");
	return;
    }
    if (to_stack != from_stack)
    {
	if (to_stack->last != NULL && !
	    CardIsInOrder (from_card, to_stack->last))
	{
	    Message (message, "Can't move the %P to the %P.",
		     &from_card->card, &to_stack->last->card);
	    return;
	}
    } else {
	if (!(to_stack = FindFinishPlay (from_stack, from_card)) &&
	    !(to_stack = FindInSuitPlay (from_stack, from_card)) &&
	    !(to_stack = FindNotInSuitPlay (from_stack, from_card)) &&
	    !(to_stack = FindEmptyPlay (from_stack, from_card)))
	{
	    Message (message, "Nowhere to move the %P.", &from_card->card);
	    return;
	}
    }
    CardMove (from_stack, from_card, to_stack, True);
}

static Boolean
AlreadyEmpty (CardPtr a, CardPtr b)
{
    (void) a;
    return !b;
}

static void
FindAMove (void)
{
    int		    col;
    CardStackPtr    from_stack, to_stack;
    CardPtr	    from_card;
    Boolean	    goodenough[NUM_STACKS];

    to_stack = NULL;
    for (col = 0; col < NUM_STACKS; col++)
	goodenough[col] = False;
#define FindOneCheck(already, func) \
    for (col = 0; !to_stack && col < NUM_STACKS; col++) {\
	if (goodenough[col]) continue; \
	from_stack = &stackStacks[col]; \
	if (!from_stack->last) continue; \
	from_card = CardInReverseSuitOrder (from_stack->last); \
	if (!from_card->prev || !already (from_card, from_card->prev)) \
	    to_stack = func(from_stack, from_card); \
	else \
	    goodenough[col] = True; \
    }
#define FindOne(func) \
    for (col = 0; !to_stack && col < NUM_STACKS; col++) {\
	from_stack = &stackStacks[col]; \
	if (!from_stack->last) continue; \
	from_card = CardInReverseSuitOrder (from_stack->last); \
	to_stack = func(from_stack, from_card); \
    }

    FindOneCheck (CardIsInSuitOrder, FindInSuitPlay);
    FindOneCheck (CardIsInOrder, FindNotInSuitPlay);
    if (to_stack) {
	Message (message, "Move the %P to the %P.", &from_card->card,
		 &to_stack->last->card);
	return;
    }
    FindOneCheck (AlreadyEmpty, FindEmptyPlay);
    if (to_stack) {
	Message (message, "Move the %P to column %d", &from_card->card,
		 to_stack - stackStacks + 1);
	return;
    }
    FindOne (FindFinishPlay);
    if (to_stack) {
	Message (message, "Move the %P to the finish.", &from_card->card);
	return;
    }
    if (deckStack.last) {
	Message (message, "Deal the next hand.");
    } else {
	Message (message, "It's all over.");
    }
}

static void
Restore (void)
{
    Message (message, "Restore not implemented");
}

static void
Save (void)
{
    Message (message, "Save not implemented");
}

static void
Expand (CardStackPtr stack)
{
    CardPtr card, t;

    if ((card = stack->first)) {
	MessageStart ();
	MessageAppend ("Column contains:");
	while (card) {
	    if (card->face == CardFaceUp)
	    {
		MessageAppend (" %p", &card->card);
		t = CardInSuitOrder (card);
		if (t != card && t != card->next)
		{
		    card = t;
		    MessageAppend ("-%p", &card->card);
		}
	    }
	    card = card->next;
	}
	MessageAppend (".");
	MessageEnd (message);
    }
    else
	Message (message, "Column is empty");
}

/* Callbacks to user interface functions */

static CardStackPtr
WidgetToStack(Widget w, int col)
{
    if (w == stacks)
	return &stackStacks[col];
    if (w == piles)
	return &pileStacks[col];
    if (w == deck)
	return &deckStack;
    return NULL;
}

static void
InputCallback (Widget w, XtPointer closure, XtPointer data)
{
    HandInputPtr    input = (HandInputPtr) data;
    CardStackPtr    stack = NULL;
    CardStackPtr    startStack = NULL;
    CardPtr	    card = NULL;

    (void) closure;
    Message (message, "");
    stack = WidgetToStack(w, input->current.col);
    startStack = WidgetToStack(input->start.w, input->start.col);

    if (!startStack || !stack)
	return;

    switch (input->action) {
    case HandActionStart:
        break;
    case HandActionClick:
        CardSetAnimate(True);
	if (stack == &deckStack) {
            Deal ();
            CardNextHistory ();
            DisplayStacks ();
	}
	else if (stack == &deckStack) {
	}
	else if (startStack && stack && startStack->last)
        {
            card = stack->last;
            if (w == stacks)
                card = CardInReverseSuitOrder(card);

            if (!card)
                Message(message, "No card selected.");
            else {
                Play (stack, card, stack);
                CheckStackTop (stack);
                CardNextHistory ();
                DisplayStacks ();
            }
	}
        break;
    case HandActionDrag:
        CardSetAnimate(False);
	if (startStack == &deckStack || stack == &deckStack)
            break;
	if (startStack->last)
        {
            CardPtr     card = CardFromHandCard(input->start.private);

            if (!card)
                Message(message, "No card selected.");
            else if (input->start.w == piles) {
                Message(message, "Can't move cards back from pile.");
            }
            else
            {
                Play (startStack, card, stack);
                CheckStackTop (startStack);
                CardNextHistory ();
                DisplayStacks ();
            }
	}
        break;
    case HandActionExpand:
        Expand (stack);
        break;
    case HandActionUnexpand:
        break;
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

XtActionsRec	actions[] = {
    { "spiderUndo",	UndoAction, },
    { "spiderNewGame",	NewGameAction, },
    { "spiderScore",	ScoreAction, },
    { "spiderQuit",	QuitAction, },
    { "spiderFindAMove",	FindAMoveAction, },
    { "spiderRestore",	RestoreAction, },
    { "spiderSave",	SaveAction, },
};

struct menuEntry {
    char    *name;
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
    Widget  menu;
    Widget  entry;
    int	    i;

    menu = XtCreatePopupShell (name, ksimpleMenuWidgetClass,
			       parent, NULL, ZERO);
    for (i = 0; i < count; i++) {
	entry = XtCreateManagedWidget (entries[i].name,
				       ksmeBSBObjectClass, menu, NULL, ZERO);
	XtAddCallback (entry, XtNcallback, entries[i].function, NULL);
    }
    return menu;
}

#define offset(field) XtOffsetOf(SpiderResources, field)

XtResource resources[] = {
    { "animationSpeed", "AnimationSpeed", XtRInt, sizeof (int),
     offset(animationSpeed), XtRImmediate, (XtPointer) -1},
    { "squishCards", "SquishCards", XtRBoolean, sizeof (Boolean),
     offset(squishCards), XtRImmediate, (XtPointer) FALSE},
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
    Atom wm_delete_window;

    toplevel = XkwInitialize ("KSpider", options, XtNumber(options),
			      &argc, argv, True, defaultResources);

    XtGetApplicationResources (toplevel, (XtPointer)&spiderResources, resources,
			       XtNumber (resources), NULL, 0);

    AnimateSetSpeed (spiderResources.animationSpeed);

    XtAddActions (actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable ("<Message>WM_PROTOCOLS: spiderQuit()"));
    frame = XtCreateManagedWidget ("frame", layoutWidgetClass, toplevel, NULL, 0);
    menuBar = XtCreateManagedWidget ("menuBar", layoutWidgetClass, frame, NULL, 0);
    fileMenuButton = XtCreateManagedWidget ("fileMenuButton",
					    kmenuButtonWidgetClass,
					    menuBar, NULL, ZERO);
    fileMenu = CreateMenu (fileMenuButton, "fileMenu",
			   fileMenuEntries, XtNumber (fileMenuEntries));
    newGame = XtCreateManagedWidget ("newGame", kcommandWidgetClass,
				     menuBar, NULL, ZERO);
    XtAddCallback(newGame, XtNcallback, NewGameCallback, NULL);
    undo = XtCreateManagedWidget ("undo", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(undo, XtNcallback, UndoCallback, NULL);
    hint = XtCreateManagedWidget ("hint", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(hint, XtNcallback, FindAMoveCallback, NULL);
    score = XtCreateManagedWidget ("score", kcommandWidgetClass,
				   menuBar, NULL, ZERO);
    XtAddCallback(score, XtNcallback, ScoreCallback, NULL);
    deck = XtCreateManagedWidget ("deck", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (deck, XtNinputCallback, InputCallback, NULL);
    piles = XtCreateManagedWidget ("piles", cardsWidgetClass, frame, NULL, 0);
    stacks = XtCreateManagedWidget ("stacks", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (stacks, XtNinputCallback, InputCallback, NULL);
    message = XtCreateManagedWidget ("message", klabelWidgetClass, frame, NULL, 0);
    srandom (getpid () ^ time ((time_t *) 0));
    NewGame ();
    XtRealizeWidget (toplevel);
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);

    XkwSetCardIcon(toplevel);

    XtMainLoop ();
}
