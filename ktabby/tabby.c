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
# include	<X11/Xaw/Dialog.h>
# include	<Xkw/KLabel.h>
# include	<Xkw/KMenuButton.h>
# include	<Xkw/KSimpleMenu.h>
# include	<Xkw/KSmeBSB.h>
# include	<X11/Xaw/AsciiText.h>
# include	<X11/Xaw/Cardinals.h>
# include	<Xkw/Cards.h>
# include	<Xkw/Layout.h>
# include	<X11/Xutil.h>
# include	<Xkw/CardsUtil.h>
# include	<Xkw/Message.h>
# include	"KTabby-res.h"

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
Widget	    pileAll;
Widget	    tail;

#define NUM_DECK    1
#define NUM_STACKS  4
#define NUM_PILES   4
#define NUM_CARDS   52

CardStackRec	deckStack;
CardStackRec	stackStacks[NUM_STACKS];
CardStackRec	pileStacks[NUM_PILES];
CardStackRec	tailStack;

CardRec		rawcards[NUM_CARDS];

int		dealNumber;

typedef struct _tabbyResources {
    int		animationSpeed;
} TabbyResources, *TabbyResourcesPtr;

TabbyResources tabbyResources;

static void
InitStacks (void)
{
    int		    col;

    CardInitStack (&deckStack, deck, CardsEmpty, False, 0, CardDisplayBottom);
    for (col = 0; col < NUM_STACKS; col++)
    {
	CardInitStack (&stackStacks[col],
		       stacks, CardsNone, False, col, CardDisplayAll);
    }
    CardInitStack (&tailStack, tail, CardsEmpty, False, 0, CardDisplayAll);
    for (col = 0; col < NUM_PILES; col++)
    {
	CardInitStack (&pileStacks[col],
		       piles, CardsEmpty, False, col, CardDisplayTop);
    }
}

static void
GenerateCards (void)
{
    CardGenerateStandardDeck (rawcards);
    deckStack.first = &rawcards[0];
    deckStack.last = &rawcards[NUM_CARDS-1];
}

#define FIRST_ROWS  1

static void
FirstDeal (void)
{
    int	    row, col;

    for (row = 0; row < FIRST_ROWS; row++)
    {
	for (col = 0; col < NUM_STACKS; col++)
	    CardMove (&deckStack, deckStack.last, &stackStacks[col], False);
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

static int
ComputeScore (void)
{
    int	    col;
    int	    score;
    CardPtr card;

    score = 0;
    for (col = 0; col < NUM_PILES; col++)
    {
	for (card = pileStacks[col].first; card; card = card->next)
	{
	    if (card->card.rank < CardsJack)
		score += (int) card->card.rank;
	    else
		score += 10;
	}
    }
    return score;
}

static void
DisplayStacks (void)
{
    int		    col;

    CardDisplayStack (&deckStack);

    for (col = 0; col < NUM_PILES; col++)
	CardDisplayStack (&pileStacks[col]);

    for (col = 0; col < NUM_STACKS; col++)
	CardDisplayStack (&stackStacks[col]);
    CardDisplayStack (&tailStack);
    CardsUpdateDisplay (deck);
    CardsUpdateDisplay (piles);
    CardsUpdateDisplay (stacks);
    CardsUpdateDisplay (tail);
}

/* User interface functions */

#define DEAL_COUNT  3

static void
Deal (void)
{
    int	    col;

    if (!deckStack.last)
    {
	Message (message, "No more cards in the deck.");
	return;
    }
    for (col = 0; col < NUM_STACKS; col++)
    {
	if (!deckStack.last)
	    break;
	CardMove (&deckStack, deckStack.last, &stackStacks[col], True);
	CardTurn (stackStacks[col].last, CardFaceUp, True);
    }
}

static void
NewGame (void)
{
    CardsRemoveAllCards (deck);
    CardsRemoveAllCards (piles);
    CardsRemoveAllCards (stacks);
    CardsRemoveAllCards (tail);
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
    Message (message, "Current position scores %d out of 340.",
	     ComputeScore ());
}

static void
Quit (void)
{
    exit (0);
}

static Boolean
IsLegalPilePlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    if (to_stack->last)
	return False;
    if (from_card->card.rank == CardsKing &&
	CardInOrder (from_card)->card.rank == CardsAce)
	return True;
    return False;
}

static CardStackPtr
FindPilePlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    CardPtr	    from_card;
    int		    i;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = CardInReverseOrder (from_stack->last);
    else
	return NULL;
    for (i = 0; i < NUM_PILES; i++)
    {
	if (IsLegalPilePlay (from_stack, from_card, &pileStacks[i]))
	{
	    *from_cardp = from_card;
	    return &pileStacks[i];
	}
    }
    return NULL;
}

static Boolean
IsLegalRegularPlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    CardPtr to_card;

    to_card = to_stack->last;
    if (to_card && CardIsInRingOrder (from_card, to_card))
	return True;
    return False;
}

static CardStackPtr
FindRegularPlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    int		    i;
    int		    col = from_stack - stackStacks;
    CardStackPtr    to_stack;
    CardPtr	    from_card;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = CardInReverseRingOrder (from_stack->last);
    else
	return NULL;
    for (i = 0; i < NUM_STACKS; i++)
    {
	if (i == col)
	    continue;
	to_stack = &stackStacks[i];
	if (IsLegalRegularPlay (from_stack, from_card, to_stack))
	{
	    *from_cardp = from_card;
	    return to_stack;
	}
    }
    return NULL;
}

static Boolean
IsLegalTailPlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    if (to_stack->last == NULL)
	return True;
    return False;
}

static CardStackPtr
FindTailPlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    CardPtr from_card;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = CardInReverseRingOrder (from_stack->last);
    else
	return NULL;
    if (IsLegalTailPlay (from_stack, from_card, &tailStack))
    {
	*from_cardp = from_card;
	return &tailStack;
    }
    return NULL;
}

static Boolean
IsLegalEmptyPlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    if (to_stack->last == NULL)
	return True;
    return False;
}

static CardStackPtr
FindEmptyPlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    int		    i;
    int		    col = from_stack - stackStacks;
    CardStackPtr    to_stack;
    CardPtr	    from_card;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = CardInReverseRingOrder (from_stack->last);
    else
	return NULL;
    for (i = 0; i < NUM_STACKS; i++)
    {
	if (i == col)
	    continue;
	to_stack = &stackStacks[i];
	if (IsLegalEmptyPlay (from_stack, from_card, to_stack))
	{
	    *from_cardp = from_card;
	    return to_stack;
	}
    }
    return NULL;
}

static void
Play (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    if (from_card && from_card->face == CardFaceDown)
    {
	Message (message, "Card not turned up.");
	return;
    }
    if (from_card && CardInRingOrder (from_card)->next != NULL)
    {
	Message (message, "Cards not in order.");
	return;
    }
    if (to_stack != from_stack)
    {
	if (to_stack->widget == stacks)
	{
	    if (!from_card)
		from_card = CardInReverseRingOrder (from_stack->last);
	    if (!IsLegalRegularPlay (from_stack, from_card, to_stack) &&
		!IsLegalEmptyPlay (from_stack, from_card, to_stack))
	    {
		if (!to_stack->last)
		    Message (message, "Can't move the %P to an empty pile.",
			     &from_card->card);
		else
		    Message (message, "Can't move the %P to the %P.",
			     &from_card->card, &to_stack->last->card);
		return;
	    }
	}
	else if (to_stack->widget == tail)
	{
	    if (!from_card)
		from_card = CardInReverseRingOrder (from_stack->last);
	    if (!IsLegalTailPlay (from_stack, from_card, to_stack))
	    {
		Message (message, "Can't move the %P to the tail.",
			 &from_card->card);
		return;
	    }
	}
	else if (to_stack->widget == piles)
	{
	    if (!from_card)
		from_card = CardInReverseRingOrder (from_stack->last);
	    if (!IsLegalPilePlay (from_stack, from_card, to_stack))
	    {
		Message (message, "Can't move the %P to a pile.",
			 &from_card->card);
		return;
	    }
	}
	else
	{
	    Message (message, "Can't move cards back to the deck.");
	    return;
	}
    } else {
	if (!from_card && !from_stack->last)
	{
	    Message (message, "No cards there.");
	    return;
	}
	if (!(to_stack = FindPilePlay (from_stack, &from_card)) &&
	    !(to_stack = FindRegularPlay (from_stack, &from_card)) &&
	    !(to_stack = FindEmptyPlay (from_stack, &from_card)) &&
	    !(to_stack = FindTailPlay (from_stack, &from_card)))
	{
	    Message (message, "Not a valid move.");
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

static Boolean
AlreadyInOrder (CardPtr a, CardPtr b)
{
    if (a && b && CardIsInRingOrder (a,b))
	return True;
    return False;
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
	from_card = CardInReverseRingOrder (from_stack->last); \
	if (!already (from_card, from_card->prev)) \
	    to_stack = func(from_stack, &from_card); \
	else \
	    goodenough[col] = True; \
    } \

#define FindOne(func) \
    for (col = 0; !to_stack && col < NUM_STACKS; col++) {\
	from_stack = &stackStacks[col]; \
	if (!from_stack->last) continue; \
	from_card = from_stack->last; \
	to_stack = func(from_stack, &from_card); \
    } \

    FindOneCheck (AlreadyInOrder, FindRegularPlay);
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
    FindOne (FindPilePlay);
    if (to_stack) {
	Message (message, "Move the %P to the finish.", &from_card->card);
	return;
    }
    FindOne (FindTailPlay);
    if (to_stack) {
	Message (message, "Move the %P to the tail.", &from_card->card);
	return;
    }
    if (deckStack.last) {
	Message (message, "Deal.");
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
PileAll (void)
{
    CardStackPtr    from_stack;
    CardPtr	    from_card;
    CardStackPtr    to_stack;
    int		    col;
    Boolean	    done = False;

    Message (message, "");
    do {
	to_stack = 0;
	FindOne (FindPilePlay);
	if (to_stack)
	{
	    Play (from_stack, from_card, to_stack);
	    done = True;
	    CheckStackTop (from_stack);
	    CardNextHistory ();
	    DisplayStacks ();
	}
    } while (to_stack);
    if (!done)
	Message (message, "No cards to pile.");
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
		t = CardInRingOrder (card);
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
    if (w == tail)
        return &tailStack;
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
    stack = WidgetToStack(w, input->col);
    startStack = WidgetToStack(input->start.w, input->start.col);

    if (!startStack || !stack)
	return;

    switch (input->action) {
    case HandActionStart:
	break;
    case HandActionDrag:
	break;
    case HandActionStop:
	if (startStack == &deckStack) {
	    if (stack == &deckStack) {
		Deal ();
		CardNextHistory ();
		DisplayStacks ();
	    }
	}
	else if (stack == &deckStack) {
	}
	else if (startStack && stack && startStack->last)
        {
	    if (startStack != stack) {
		for (card = startStack->last; card; card = card->prev)
		    if (card->isUp && card->row == input->start.row)
			break;
            }
            Play (startStack, card, stack);
            CheckStackTop (startStack);
            CardNextHistory ();
            DisplayStacks ();
	}
        break;
    case HandActionExpand:
        Expand (stack);
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

static void
PileAllCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    PileAll ();
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

static void PileAllAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    PileAll ();
}

XtActionsRec	actions[] = {
    { "tabbyUndo",	UndoAction, },
    { "tabbyNewGame",	NewGameAction, },
    { "tabbyScore",	ScoreAction, },
    { "tabbyQuit",	QuitAction, },
    { "tabbyFindAMove",	FindAMoveAction, },
    { "tabbyRestore",	RestoreAction, },
    { "tabbySave",	SaveAction, },
    { "tabbyPileAll",	PileAllAction, },
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

#define offset(field) XtOffsetOf(TabbyResources, field)

XtResource resources[] = {
    { "animationSpeed", "AnimationSpeed", XtRInt, sizeof (int),
     offset(animationSpeed), XtRImmediate, (XtPointer) -1},
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

    toplevel = XkwInitialize ("KTabby", options, XtNumber(options),
			      &argc, argv, True, defaultResources);

    XtGetApplicationResources (toplevel, (XtPointer)&tabbyResources, resources,
			       XtNumber (resources), NULL, 0);

    AnimateSetSpeed (tabbyResources.animationSpeed);

    XtAddActions (actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable ("<Message>WM_PROTOCOLS: tabbyQuit()"));
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
    pileAll = XtCreateManagedWidget ("pileAll", kcommandWidgetClass,
				   menuBar, NULL, ZERO);
    XtAddCallback(pileAll, XtNcallback, PileAllCallback, NULL);
    deck = XtCreateManagedWidget ("deck", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (deck, XtNinputCallback, InputCallback, NULL);
    piles = XtCreateManagedWidget ("piles", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (piles, XtNinputCallback, InputCallback, NULL);
    stacks = XtCreateManagedWidget ("stacks", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (stacks, XtNinputCallback, InputCallback, NULL);
    tail = XtCreateManagedWidget ("tail", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (tail, XtNinputCallback, InputCallback, NULL);
    message = XtCreateManagedWidget ("message", klabelWidgetClass, frame, NULL, 0);
    srandom (getpid () ^ time ((long *) 0));
    NewGame ();
    XtRealizeWidget (toplevel);
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);

    XtMainLoop ();
}
