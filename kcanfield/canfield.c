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
# include	<Xkw/KLabel.h>

Widget	    toplevel;
Widget	    frame;

Widget	    stockWidget;
Widget	    talonWidget;
Widget	    tableauWidget;
Widget	    foundationWidget;

Widget	    logo;

Widget	    message;
Widget	    menuBar;
Widget	    fileMenuButton;
Widget	    fileMenu;
Widget	    newGame;
Widget	    undo;
Widget	    hint;
Widget	    score;
Widget	    pileAll;
Widget	    baseRankWidget;

#define NUM_TABLEAU	4
#define NUM_FOUNDATION	4
#define NUM_CARDS	52

CardsRank	baseRank;
CardStackRec	stock;
CardStackRec	talon;
CardStackRec	deck;
CardStackRec	tableau[NUM_TABLEAU];
CardStackRec	foundation[NUM_FOUNDATION];
CardRec		rawcards[NUM_CARDS];

#define INIT_STOCK  13

CardStackPtr	fromStack;
CardPtr		fromCard;
int		dealNumber;

typedef struct _canfieldResources {
    int		animationSpeed;
} CanfieldResources, *CanfieldResourcesPtr;

CanfieldResources canfieldResources;

static void
InitStacks (void)
{
    int		    col;

    CardInitStack (&stock, stockWidget, CardsEmpty, False, 0, CardDisplayTop);
    CardInitStack (&deck, talonWidget, CardsEmpty, False, 0, CardDisplayBottom);
    CardInitStack (&talon, talonWidget, CardsEmpty, False, 1, CardDisplayTop);
    for (col = 0; col < NUM_TABLEAU; col++)
    {
	CardInitStack (&tableau[col],
		       tableauWidget, CardsNone, False, col, CardDisplayAll);
    }
    for (col = 0; col < NUM_FOUNDATION; col++)
    {
	CardInitStack (&foundation[col],
		       foundationWidget, CardsEmpty, False, col, CardDisplayTop);
    }
}

static void
GenerateCards (void)
{
    CardGenerateStandardDeck (rawcards);
    deck.first = &rawcards[0];
    deck.last = &rawcards[NUM_CARDS-1];
}

static void
FirstDeal (void)
{
    int		row, col;

    CardMove (&deck, deck.last, &foundation[0], False);
    CardTurn (foundation[0].last, CardFaceUp, False);
    baseRank = foundation[0].last->card.rank;
    Message (baseRankWidget, "Base rank is %s", CardsRankName (baseRank));
    for (row = 0; row < INIT_STOCK; row++)
    {
	CardMove (&deck, deck.last, &stock, False);
	CardTurn (stock.last, CardFaceUp, False);
    }
    for (col = 0; col < NUM_TABLEAU; col++)
    {
	CardMove (&deck, deck.last, &tableau[col], False);
	CardTurn (tableau[col].last, CardFaceUp, False);
    }
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
    for (col = 0; col < NUM_FOUNDATION; col++)
	for (card = foundation[col].first; card; card = card->next)
	    score += 5;
    return score;
}

static void
DisplayStacks (void)
{
    int		    col;

    CardDisplayStack (&stock);
    CardDisplayStack (&talon);
    CardDisplayStack (&deck);
    for (col = 0; col < NUM_TABLEAU; col++)
	CardDisplayStack (&tableau[col]);

    for (col = 0; col < NUM_FOUNDATION; col++)
	CardDisplayStack (&foundation[col]);

    CardsUpdateDisplay (stockWidget);
    CardsUpdateDisplay (talonWidget);
    CardsUpdateDisplay (tableauWidget);
    CardsUpdateDisplay (foundationWidget);
}

/* User interface functions */

#define DEAL_COUNT  3

static void
ResetTalon (void)
{
    CardPtr c;

    while ((c = talon.last))
    {
	CardMove (&talon, talon.last, &deck, True);
	CardTurn (deck.last, CardFaceDown, True);
    }
}

static void
Deal (void)
{
    int	    deal;

    for (deal = 0; deal < DEAL_COUNT; deal++)
    {
	if (!deck.last)
	{
	    Message (message, "No more cards in the deck.");
	    return;
	}
	CardMove (&deck, deck.last, &talon, True);
	CardTurn (talon.last, CardFaceUp, True);
    }
}

static void
NewGame (void)
{
    CardsRemoveAllCards (stockWidget);
    CardsRemoveAllCards (talonWidget);
    CardsRemoveAllCards (tableauWidget);
    CardsRemoveAllCards (foundationWidget);
    fromStack = 0;
    fromCard = 0;
    InitStacks ();
    GenerateCards ();
    CardShuffle (&deck, False);
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
    Message (message, "Current position scores %d out of 260.",
	     ComputeScore ());
}

static void
Quit (void)
{
    exit (0);
}

static Boolean
CanfieldCardIsInOrder (CardPtr a, CardPtr b)
{
    return a->face == b->face &&
	(a->card.rank + 1 == b->card.rank ||
	 (a->card.rank == CardsKing && b->card.rank == CardsAce)) &&
	b->card.rank != baseRank;
}

static Boolean
CanfieldCardIsInSuitOrder (CardPtr a, CardPtr b)
{
    return a->card.suit == b->card.suit && CanfieldCardIsInOrder (a,b);
}

static Boolean
IsRedSuit (CardsSuit suit)
{
    return suit == CardsHeart || suit == CardsDiamond;
}

static Boolean
CanfieldCardIsInAlternatingSuitOrder (CardPtr a, CardPtr b)
{
    return IsRedSuit(a->card.suit) != IsRedSuit (b->card.suit) &&
           CanfieldCardIsInOrder (a,b);
}

#if 0
static CardPtr
CanfieldCardInSuitOrder (CardPtr card)
{
    while (card->next && CanfieldCardIsInSuitOrder (card->next, card))
	card = card->next;
    return card;
}
#endif

static CardPtr
CanfieldCardInAlternatingSuitOrder (CardPtr card)
{
    while (card->next && CanfieldCardIsInAlternatingSuitOrder (card->next, card))
	card = card->next;
    return card;
}

static CardPtr
CanfieldCardInReverseAlternatingSuitOrder (CardPtr card)
{
    while (card->prev && CanfieldCardIsInAlternatingSuitOrder (card, card->prev))
	card = card->prev;
    return card;
}

static Boolean
IsLegalFoundationPlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    (void) from_stack;
    if (from_card->next)
	return False;
    if (from_card->card.rank == baseRank)
    {
	if (to_stack->last == NULL)
	    return True;
    }
    else
    {
	if (to_stack->last != NULL &&
	    CanfieldCardIsInSuitOrder (to_stack->last, from_card))
	    return True;
    }
    return False;
}

static CardStackPtr
FindFoundationPlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    int		    i;
    CardStackPtr    to_stack;
    CardPtr	    from_card;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = from_stack->last;
    else
	return NULL;
    for (i = 0; i < NUM_FOUNDATION; i++)
    {
	to_stack = &foundation[i];
	if (IsLegalFoundationPlay (from_stack, from_card, to_stack))
	{
	    *from_cardp = from_card;
	    return to_stack;
	}
    }
    return NULL;
}

static Boolean
IsLegalRegularPlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    CardPtr to_card;

    (void) from_stack;
    to_card = to_stack->last;
    if (to_card && CanfieldCardIsInAlternatingSuitOrder (from_card, to_card))
	return True;
    return False;
}

static CardStackPtr
FindRegularPlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    int		    i;
    int		    col = from_stack - tableau;
    CardStackPtr    to_stack;
    CardPtr	    from_card;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = CanfieldCardInReverseAlternatingSuitOrder (from_stack->last);
    else
	return NULL;
    for (i = 0; i < NUM_TABLEAU; i++)
    {
	if (i == col)
	    continue;
	to_stack = &tableau[i];
	if (IsLegalRegularPlay (from_stack, from_card, to_stack))
	{
	    *from_cardp = from_card;
	    return to_stack;
	}
    }
    return NULL;
}

static Boolean
IsLegalEmptyPlay (CardStackPtr from_stack, CardPtr from_card, CardStackPtr to_stack)
{
    (void) from_card;
    if ((from_stack == &stock || stock.last == NULL) &&
	to_stack->last == NULL)
	return True;
    return False;
}

static CardStackPtr
FindEmptyPlay (CardStackPtr from_stack, CardPtr *from_cardp)
{
    int		    i;
    int		    col = from_stack - tableau;
    CardStackPtr    to_stack;
    CardPtr	    from_card;

    if (*from_cardp)
	from_card = *from_cardp;
    else if (from_stack->last)
	from_card = CanfieldCardInReverseAlternatingSuitOrder (from_stack->last);
    else
	return NULL;
    for (i = 0; i < NUM_TABLEAU; i++)
    {
	if (i == col)
	    continue;
	to_stack = &tableau[i];
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
    if (from_card && CanfieldCardInAlternatingSuitOrder (from_card)->next != NULL)
    {
	Message (message, "Cards not in order.");
	return;
    }
    if (to_stack != from_stack)
    {
	if (to_stack->widget == tableauWidget)
	{
	    if (!from_card)
		from_card = CanfieldCardInReverseAlternatingSuitOrder (from_stack->last);
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
	else if (to_stack->widget == foundationWidget)
	{
	    if (!from_card)
		from_card = from_stack->last;
	    if (!IsLegalFoundationPlay (from_stack, from_card, to_stack))
	    {
		if (!to_stack->last)
		    Message (message, "Can't move the %P to an empty stack.",
			     &from_card->card);
		else
		    Message (message, "Can't move the %P to the %P.",
			     &from_card->card, &to_stack->last->card);
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
	if (!(to_stack = FindFoundationPlay (from_stack, &from_card)) &&
	    !(to_stack = FindRegularPlay (from_stack, &from_card)) &&
	    !(to_stack = FindEmptyPlay (from_stack, &from_card)))
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
    if (a && b && CanfieldCardIsInAlternatingSuitOrder (a,b))
	return True;
    return False;
}

static void
FindAMove (void)
{
    int		    col;
    CardStackPtr    from_stack, to_stack;
    CardPtr	    from_card;
    Boolean	    goodenough[NUM_TABLEAU];

    to_stack = NULL;
    for (col = 0; col < NUM_TABLEAU; col++)
	goodenough[col] = False;
#define FindOneCheck(already, func) \
    for (col = 0; !to_stack && col < NUM_TABLEAU; col++) {\
	if (goodenough[col]) continue; \
	from_stack = &tableau[col]; \
	if (!from_stack->last) continue; \
	from_card = CanfieldCardInReverseAlternatingSuitOrder (from_stack->last); \
	if (!already (from_card, from_card->prev)) \
	    to_stack = func(from_stack, &from_card); \
	else \
	    goodenough[col] = True; \
    } \
    if (!to_stack) { \
	from_stack = &talon; \
	from_card = from_stack->last; \
	if (from_card) \
	    to_stack = func(from_stack, &from_card);\
    }

#define FindOne(func) \
    for (col = 0; !to_stack && col < NUM_TABLEAU; col++) {\
	from_stack = &tableau[col]; \
	if (!from_stack->last) continue; \
	from_card = from_stack->last; \
	to_stack = func(from_stack, &from_card); \
    } \
    if (!to_stack) { \
	from_stack = &talon; \
	from_card = from_stack->last; \
	if (from_card) \
	    to_stack = func(from_stack, &from_card);\
    }

    FindOneCheck (AlreadyInOrder, FindRegularPlay);
    if (to_stack) {
	Message (message, "Move the %P to the %P.", &from_card->card,
		 &to_stack->last->card);
	return;
    }
    FindOneCheck (AlreadyEmpty, FindEmptyPlay);
    if (to_stack) {
	Message (message, "Move the %P to column %d", &from_card->card,
		 to_stack - tableau + 1);
	return;
    }
    FindOne (FindFoundationPlay);
    if (to_stack) {
	Message (message, "Move the %P to the finish.", &from_card->card);
	return;
    }
    if (deck.last) {
	Message (message, "Deal the next hand.");
    } else {
	Message (message, "Reset the deck.");
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
FoundationAll (void)
{
    CardStackPtr    from_stack;
    CardPtr	    from_card;
    CardStackPtr    to_stack;
    int		    col;
    Boolean	    done = False;

    Message (message, "");
    do {
	to_stack = 0;
	FindOne (FindFoundationPlay);
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

static void
TableauCallback (Widget w, XtPointer closure, XtPointer data)
{
    CardsInputPtr    input = (CardsInputPtr) data;
    CardStackPtr    stack = NULL;
    CardPtr	    card;
    String	    type;

    (void) closure;
    Message (message, "");
    if (w == tableauWidget)
	stack = &tableau[input->col];
    else if (w == talonWidget)
    {
	if (input->col == 0)
	    stack = &deck;
	else
	    stack = &talon;
    }
    else if (w == foundationWidget)
	stack = &foundation[input->col];
    else if (w == stockWidget)
	stack = &stock;
    for (card = stack->last; card; card = card->prev)
	if (card->isUp && card->row == input->row)
	    break;
    if (*input->num_params) {
	type = *input->params;
	if (!strcmp (type, "talon_source"))
	{
	    fromStack = stack;
	    fromCard = stack->last;
	}
	else if (!strcmp (type, "tableau_source"))
	{
	    fromStack = stack;
	    fromCard = 0;
	    if (!fromStack->last)
		Message (message, "Selected tableau is empty.");
	}
	else if (!strcmp (type, "card_source"))
	{
	    if (!card)
		Message (message, "No card selected.");
	    else
	    {
		fromStack = stack;
		fromCard = card;
	    }
	}
	else if (!strcmp (type, "dest"))
	{
	    if (fromStack)
	    {
		Play (fromStack, fromCard, stack);
		CheckStackTop (fromStack);
		fromStack = NULL;
		CardNextHistory ();
		DisplayStacks ();
	    }
	}
	else if (!strcmp (type, "expand"))
	{
	    Expand (stack);
	}
    }
}

static void
TalonCallback (Widget w, XtPointer closure, XtPointer data)
{
    CardsInputPtr    input = (CardsInputPtr) data;
    CardStackPtr    stack;

    (void) closure;
    Message (message, "");
    if (input->col == 0)
	stack = &deck;
    else
	stack = &talon;
    if (input->col == 0 && *input->num_params &&
	!strcmp (*input->params, "talon_source"))
    {
	if (!stack->last)
	    ResetTalon ();
	else
	    Deal ();
    }
    else
    {
	TableauCallback (w, closure, data);
    }
    CardNextHistory ();
    DisplayStacks ();
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
FoundationAllCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    FoundationAll ();
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

static void FoundationAllAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    FoundationAll ();
}

XtActionsRec	actions[] = {
    { "canfieldUndo",	UndoAction, },
    { "canfieldNewGame",	NewGameAction, },
    { "canfieldScore",	ScoreAction, },
    { "canfieldQuit",	QuitAction, },
    { "canfieldFindAMove",	FindAMoveAction, },
    { "canfieldRestore",	RestoreAction, },
    { "canfieldSave",	SaveAction, },
    { "canfieldFoundationAll",	FoundationAllAction, },
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
CreateMenu (Widget  parent,
	    char    *name,
	    struct menuEntry	*entries,
	    int	    count)
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

#define offset(field) XtOffsetOf(CanfieldResources, field)

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

#ifdef APPDEFAULTS
    setenv("XAPPLRESDIR", APPDEFAULTS, 1);
#endif

    toplevel = XtInitialize (argv[0], "KCanfield", options, XtNumber(options),
			     &argc, argv);

    Arg	args[1];
    XtSetArg(args[0], XtNinput, True);
    XtSetValues(toplevel, args, ONE);

    XtGetApplicationResources (toplevel, (XtPointer)&canfieldResources, resources,
			       XtNumber (resources), NULL, 0);

    AnimateSetSpeed (canfieldResources.animationSpeed);

    XtAddActions (actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable ("<Message>WM_PROTOCOLS: canfieldQuit()"));
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
    XtAddCallback(pileAll, XtNcallback, FoundationAllCallback, NULL);
    baseRankWidget = XtCreateManagedWidget ("baseRank", klabelWidgetClass,
					    menuBar, NULL, ZERO);
    logo = XtCreateManagedWidget ("logo", klabelWidgetClass, frame, NULL, ZERO);
    talonWidget = XtCreateManagedWidget ("talon", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (talonWidget, XtNinputCallback, TalonCallback, NULL);
    foundationWidget = XtCreateManagedWidget ("foundation", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (foundationWidget, XtNinputCallback, TableauCallback, NULL);
    tableauWidget = XtCreateManagedWidget ("tableau", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (tableauWidget, XtNinputCallback, TableauCallback, NULL);
    stockWidget = XtCreateManagedWidget ("stock", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (stockWidget, XtNinputCallback, TableauCallback, NULL);
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
