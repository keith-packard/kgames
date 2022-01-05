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
# include	<stdio.h>
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
# include	"KMontana-res.h"

Widget	    toplevel;
Widget	    frame;
Widget	    cards;
Widget	    message;
Widget	    menuBar;
Widget	    fileMenuButton;
Widget	    fileMenu;
Widget	    deal;
Widget	    dealDisplay;
Widget	    newGame;
Widget	    undo;
Widget	    hint;
Widget	    score;

#define NUM_DEALS   2

#define NUM_ROWS    4
#define NUM_COLS    13
#define NUM_CARDS   52

#define MAX_SCORE 336

CardStackRec	cardStacks[NUM_CARDS];
CardStackRec	deckStack;
CardRec		rawcards[NUM_CARDS];
CardsRank	maxrank[NUM_ROWS];

CardStackPtr	position[NUM_CARDS];

CardStackPtr	fromStack;
CardPtr		fromCard;
int		dealNumber;
int		currentScore;
CardStackPtr	hintStack;

CardStackPtr	emptyStacks[4];

typedef struct _montanaResources {
    int		animationSpeed;
    int		searchDepth;
} MontanaResources, *MontanaResourcesPtr;

MontanaResources montanaResources;

#define ForAllCardVars	int row, col; CardStackPtr  stack
#define ForAllCards	for (row = 0, stack = cardStacks; row < NUM_ROWS; row++) \
			    for (col = 0; col < NUM_COLS; col++, stack++)

static void
InitStacks (void)
{
    ForAllCardVars;

    ForAllCards
    {
	CardInitStack (stack, cards, CardsNone, True, row,
		       CardDisplayAll);
	stack->basePosition = col;
    }
    CardInitStack (&deckStack, (Widget) 0, CardsNone,
		   True, 0, CardDisplayNone);
    for (row = 0; row < NUM_ROWS; row++)
	maxrank[row] = CardsAce;
    currentScore = 0;
}

static void
GenerateCards (void)
{
    CardGenerateStandardDeck (rawcards);
    deckStack.first = &rawcards[0];
    deckStack.last = &rawcards[NUM_CARDS-1];
}

static void
RemoveAces (Boolean remember)
{
    ForAllCardVars;
    int	    i;

    i = 0;
    ForAllCards
    {
	if (stack->last->card.rank == CardsAce)
	{
	    CardMove (stack, stack->last, &deckStack, remember);
	    emptyStacks[i++] = stack;
	}
    }
}

static void
SetPosition (CardStackPtr stack)
{
    int	    i;

    i = stack->last - rawcards;
    position[i] = stack;
}

static int
ComputeScore (void);

static void
UnHint (void);

static void
FirstDeal (void)
{
    ForAllCardVars;

    ForAllCards
    {
	CardMove (&deckStack, deckStack.last, stack, False);
	SetPosition (stack);
	CardTurn (stack->last, CardFaceUp, False);
    }
    RemoveAces (False);
    (void) ComputeScore ();
}

static CardStackPtr
NotInOrder (int row)
{
    CardStackPtr    stack;
    CardPtr	    card;
    int		    col;
    CardsRank	    rank;
    CardsSuit	    suit;

    stack = &cardStacks[row * NUM_COLS];
    card = stack->last;
    col = 0;
    if (card && card->card.rank == Cards2) {
	rank = Cards3;
	suit = card->card.suit;
	stack++;
	for (col = 1; col < NUM_COLS; col++, stack++, rank++)
	{
	    card = stack->last;
	    if (!card || card->card.rank != rank || card->card.suit != suit)
		break;
	}
    }
    return stack;
}

static int
ComputeScore (void)
{
    int		    row;
    CardStackPtr    stack, notInOrder;
    CardPtr	    card;
    int		    score = 0;

    for (row = 0; row < NUM_ROWS; row++)
    {
	stack = &cardStacks[row * NUM_COLS];
	notInOrder = NotInOrder (row);
	maxrank[row] = CardsAce;
	if (stack != notInOrder)
	    maxrank[row] = notInOrder[-1].last->card.rank;
	while (stack != notInOrder) {
	    card = stack->last;
	    if (card->card.rank < CardsJack)
		score += (int) card->card.rank;
	    else
		score += 10;
	    stack++;
	}
    }
    currentScore = score;
    if (dealNumber > NUM_DEALS)
	score -= 100 * (dealNumber - NUM_DEALS);
    return score;
}

static void
DisplayStacks (void)
{
    ForAllCardVars;

    ForAllCards
	CardDisplayStack (stack);
    CardDisplayStack (&deckStack);
    CardsUpdateDisplay (cards);
}

typedef struct _emptySave {
    struct _emptySave	*next;
    CardStackPtr    empty;
    int		    i;
} EmptySaveRec, *EmptySavePtr;

EmptySavePtr	emptySave;

#define RankScore(r)	((r) > Cards10 ? 10 : (int) (r))

static Boolean
MontanaUndo (void)
{
    EmptySavePtr    e;
    CardStackPtr    stack;
    CardsRank	    rank;
    CardsSuit	    suit;

    e = emptySave;
    if (!e)
	return False;
    stack = e->empty;
    rank = stack->basePosition + 2;
    suit = stack->last->card.suit;
    if (!CardUndo())
	return False;
    if (maxrank[stack->position] == rank) {
	maxrank[stack->position] = rank - 1;
	do
	{
	    currentScore -= RankScore (rank);
	    rank++;
	    stack++;
	} while (stack->last &&
		 stack->last->card.rank == rank &&
		 stack->last->card.suit == suit);
    }
    SetPosition(emptyStacks[e->i]);
    emptyStacks[e->i] = e->empty;
    emptySave = e->next;
    Dispose (e);
    return True;
}

static void
MontanaDo (CardStackPtr from_stack, CardStackPtr to_stack)
{
    EmptySavePtr    e;
    int		    i;
    CardPtr	    card;
    CardsRank	    rank;
    CardsSuit	    suit;
    CardStackPtr    stack;

    card = from_stack->last;
    CardMove (from_stack, card, to_stack, True);
    rank = to_stack->basePosition + 2;
    if (rank - 1 == maxrank[to_stack->position])
    {
	suit = card->card.suit;
	rank = rank;
	stack = to_stack;
	do
	{
	    currentScore += RankScore (rank);
	    rank++;
	    stack++;
	} while (stack->last &&
		 stack->last->card.rank == rank &&
	         stack->last->card.suit == suit);
	maxrank[to_stack->position] = rank - 1;
    }
    SetPosition (to_stack);
    e = New(EmptySaveRec);
    for (i = 0; i < 4; i++)
    {
	if (emptyStacks[i] == to_stack)
	{
	    e->i = i;
	    e->empty = to_stack;
	    e->next = emptySave;
	    emptySave = e;
	    emptyStacks[i] = from_stack;
	    break;
	}
    }
    CardNextHistory ();
}


/* User interface functions */

static void
ResetDealNumber (void *closure)
{
    dealNumber = (int) (intptr_t) closure;
    (void) ComputeScore ();
}

static void
CheckWin(void)
{
    int score = ComputeScore();
    if (dealNumber > NUM_DEALS)
        score += 100 * (dealNumber - NUM_DEALS);
    if (score == MAX_SCORE)
        Message(message, "We have a winner!");
    else
        Message(message, "");
}

static void
Deal (void)
{
    ForAllCardVars;
    CardPtr	    card;
    CardStackPtr    notInOrder;

    UnHint ();
    if (dealNumber >= NUM_DEALS)
	Message (message, "No more than %d redeals allowed.", NUM_DEALS);
    stack = cardStacks;
    for (row = 0; row < NUM_ROWS; row++)
    {
	notInOrder = NotInOrder (row);
	col = notInOrder - stack;
	stack = notInOrder;
	for (; col < NUM_COLS; col++, stack++)
	{
	    card = stack->last;
	    if (card)
		CardMove (stack, card, &deckStack, True);
	}
    }
    CardShuffle (&deckStack, True);
    ForAllCards
    {
	if (!stack->last)
	{
	    CardMove (&deckStack, deckStack.last, stack, True);
	    SetPosition (stack);
	}
    }
    RemoveAces (True);
    (void) ComputeScore ();
    CardRecordHistoryCallback (ResetDealNumber, (char *) (intptr_t) dealNumber);

    ++dealNumber;
    Message (message, "Dealt %d of %d.", dealNumber, NUM_DEALS);
    Message (dealDisplay, "Deal: %d", dealNumber);

    CardSetAnimate (False);
    DisplayStacks ();
    CardSetAnimate (True);
    CardNextHistory ();
    CheckWin();
}

static void
NewGame (void)
{
    CardsRemoveAllCards (cards);
    fromStack = 0;
    fromCard = 0;
    hintStack = 0;
    dealNumber = 0;
    InitStacks ();
    GenerateCards ();
    CardShuffle (&deckStack, False);
    FirstDeal ();
    CardInitHistory ();
    DisplayStacks ();
    Message (message, "Keith's Montana version 1.0");
    Message (dealDisplay, "No deals.");
}

static void
Undo (void)
{
    if (!MontanaUndo ())
    {
	Message (message, "Nothing to undo.");
	return;
    }
    Message (message, "Undo.");
    CardSetAnimate (False);
    DisplayStacks ();
    CardSetAnimate (True);
}

static void
Score (void)
{
    Message (message, "Current position scores %d out of %d.",
	     ComputeScore (), MAX_SCORE);
}

static void
Quit (void)
{
    exit (0);
}

static CardStackPtr
FindPlay (CardStackPtr from_stack)
{
    ForAllCardVars;
    CardPtr card = from_stack->last;

    if (!card)
	return NULL;
    if (card->card.rank == Cards2)
    {
	for (row = 0, stack = cardStacks;
	     row < NUM_ROWS;
	     row++, stack += NUM_COLS)
	{
	    if (!stack->last)
		return stack;
	}
    }
    else
    {
	ForAllCards
	{
	    if (col < NUM_COLS - 1 &&
		!stack[1].last &&
		stack->last && CardIsInSuitOrder (stack->last, card))
	    {
		return stack + 1;
	    }
	}
    }
    return NULL;
}

static CardStackPtr
FindReverse (CardStackPtr to_stack)
{
    ForAllCardVars;
    CardPtr card;

    card = to_stack->last;
    if (!card)
	return NULL;
    ForAllCards
	if (stack->last && CardIsInSuitOrder (card, stack->last))
	    return stack;
    return NULL;
}

static CardStackPtr
FindForward (CardStackPtr from_stack)
{
    ForAllCardVars;
    CardPtr card;

    card = from_stack->last;
    if (!card)
	return NULL;
    ForAllCards
	if (stack->last && CardIsInSuitOrder (stack->last, card))
	    return stack;
    return NULL;
}

static void
Play (CardStackPtr from_stack, CardStackPtr to_stack)
{
    int		    col;
    CardPtr	    card;

    UnHint ();
    card = from_stack->last;
    if (!card)
    {
	Message (message, "Can't move spaces around.");
	return;
    }
    if (from_stack == to_stack)
    {
	to_stack = FindPlay (from_stack);
	if (!to_stack)
	{
	    Message (message, "No place to move %P.", &from_stack->last->card);
	    return;
	}
    }
    else
    {
	col = to_stack->basePosition;
	if (to_stack->last)
        {
	    Message (message, "Target must be empty.");
	    return;
	}
	if (col == 0 && card->card.rank != Cards2)
	{
	    Message (message, "Only deuces can be moved to column 1.");
	    return;
	}
	if (col > 0 && !to_stack[-1].last)
	{
	    Message (message, "Can't move next to an empty space.");
	    return;
	}
	if (col > 0 && !CardIsInSuitOrder (to_stack[-1].last, card))
	{
	    Message (message, "Can't move the %P next to the %P.",
		     &card->card, &to_stack[-1].last->card);
	    return;
	}
    }
    MontanaDo (from_stack, to_stack);
    CheckWin();
}

#define MAX_MOVES   16

static int
FindAllMoves (CardStackPtr moves[MAX_MOVES][2])
{
    int			i, j, k;
    CardStackPtr	to_stack;
    CardPtr		card;
    CardsSuit		suit;

    j = 0;
    for (i = 0; i < 4; i++)
    {
	to_stack = emptyStacks[i];
	if (!to_stack->basePosition)
	{
	    for (suit = CardsClub; suit <= CardsSpade; suit++)
	    {
		k = suit * 13 + 1;
		if (position[k]->basePosition)
		{
		    moves[j][0] = position[k];
		    moves[j][1] = to_stack;
    /*		PrintMove (moves[j]); */
		    j++;
		}
	    }
	}
	else if ((card = to_stack[-1].last) && card->card.rank != CardsKing)
	{
	    k = ((int) card->card.suit * 13) + (int) card->card.rank;
	    moves[j][0] = position[k];
	    moves[j][1] = to_stack;
/*            PrintMove (moves[j]); */
	    j++;
	}
    }
    return j;
}

typedef struct _Solve {
    struct _Solve   *next;
    int		    i;
    CardStackPtr    moves[MAX_MOVES][2];
} SolveRec, *SolvePtr;

static int
Seek (SolvePtr s, int *bestp, int levels)
{
    SolveRec	ns;
    int		i, best;
    int		max_score, score, max_i;

    max_i = -1;
    if (levels < s->i || s->i <= 0)
    {
	max_score = currentScore;
    }
    else
    {
	levels -= s->i;
	max_score = -1;
	for (i = 0; i < s->i; i++)
	{
	    MontanaDo (s->moves[i][0], s->moves[i][1]);
	    ns.i = FindAllMoves (ns.moves);
	    score = Seek (&ns, &best, levels);
	    if (score > max_score)
	    {
		max_score = score;
		max_i = i;
	    }
	    MontanaUndo ();
	}
    }
    *bestp = max_i;
    return max_score;
}

#define MAX_SEARCH   1000

static void
BestMove (void)
{
    SolveRec	s;
    int		score;
    int		best;
    CardPtr	card;

    s.i = FindAllMoves (s.moves);
    score = Seek (&s, &best, montanaResources.searchDepth);
    DisplayStacks ();
    if (best < 0)
    {
	Message (message, "No moves");
	return;
    }
    card = s.moves[best][0]->last;
    if (card->card.rank == Cards2)
	Message (message, "Play the %P in column %d (%d)",
		 &card->card, s.moves[best][1]->position, score);
    else
	Message (message, "Play the %P (%d)", &card->card, score);
}

static void
UnHint (void)
{
    if (hintStack)
    {
	CardTurn (hintStack->last, CardFaceUp, True);
	hintStack = 0;
    }
}

static void
Hint (CardStackPtr stack)
{
    hintStack = stack;
    CardTurn (stack->last, CardFaceDown, True);
}

static void
FindAMove (CardStackPtr stack, Boolean forward)
{
    CardStackPtr    hint_stack;

    UnHint ();
    if (!forward)
	hint_stack = FindReverse (stack);
    else
	hint_stack = FindForward (stack);
    if (hint_stack)
	Hint (hint_stack);
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

/* Callbacks to user interface functions */

static void
InputCallback (Widget w, XtPointer closure, XtPointer data)
{
    HandInputPtr    input = (HandInputPtr) data;
    CardStackPtr    stack = NULL;
    CardStackPtr    startStack = NULL;
    int             i;
    Boolean         hintForward;
    String          type = "";

    (void) closure;
    (void) w;
    Message (message, "");

    if (*input->num_params > 0)
        type = input->params[0];

    stack = &cardStacks[input->current.row * NUM_COLS + input->current.col];
    startStack = &cardStacks[input->start.row * NUM_COLS + input->start.col];

#define MOVE	0
#define HINT	1
#define UNHINT	2
#define	SELECT	3

    switch (input->action) {
    case HandActionStart:
	if (!stack->last) {
	    i = HINT;
            hintForward = True;
	} else
	    i = SELECT;
        break;
    case HandActionDrag:
        CardSetAnimate(False);
        i = MOVE;
        break;
    case HandActionClick:
        CardSetAnimate(True);
	if (hintStack)
	    i = UNHINT;
	else
	    i = MOVE;
        break;
    case HandActionExpand:
        i = HINT;
        if (!strcmp(type, "hint-previous"))
            hintForward = False;
        else
            hintForward = True;
        break;
    case HandActionUnexpand:
        i = UNHINT;
        break;
    }

    switch (i) {
    case HINT:
	if (!stack->last)
	{
	    if (input->current.col == 0)
		return;
	    stack = stack - 1;
	    hintForward = False;
	}
	FindAMove (stack, hintForward);
	break;
    case UNHINT:
	UnHint ();
	break;
    case SELECT:
	break;
    case MOVE:
	if (startStack)
	    Play (startStack, stack);
	break;
    }
    DisplayStacks ();
}

static void
DealCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Deal ();
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
HintCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    BestMove ();
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

static void DealAction (Widget w, XEvent *e, String *p, Cardinal *n)
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

static void BestMoveAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    BestMove ();
}

XtActionsRec	actions[] = {
    { "montanaUndo",	UndoAction, },
    { "montanaDeal",	DealAction, },
    { "montanaNewGame",	NewGameAction, },
    { "montanaScore",	ScoreAction, },
    { "montanaQuit",	QuitAction, },
    { "montanaRestore",	RestoreAction, },
    { "montanaSave",	SaveAction, },
    { "montanaBestMove",	BestMoveAction, },
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

#define offset(field) XtOffsetOf(MontanaResources, field)

XtResource resources[] = {
    { "animationSpeed", "AnimationSpeed", XtRInt, sizeof (int),
     offset(animationSpeed), XtRImmediate, (XtPointer) -1},
    { "searchDepth", "SearchDepth", XtRInt, sizeof (int),
     offset(searchDepth), XtRImmediate, (XtPointer) 40},
};

XrmOptionDescRec options[] = {
    { "-noanimate",	"*animationSpeed",	XrmoptionNoArg, "0", },
    { "-animationSpeed","*animationSpeed",	XrmoptionSepArg, NULL },
    { "-searchDepth",   "*searchDepth",         XrmoptionSepArg, NULL },
};

int
main (int argc, char **argv)
{
    Atom wm_delete_window;

    toplevel = XkwInitialize ("KMontana", options, XtNumber(options),
			      &argc, argv, True, defaultResources);

    XtGetApplicationResources (toplevel, (XtPointer)&montanaResources, resources,
			       XtNumber (resources), NULL, 0);

    AnimateSetSpeed (montanaResources.animationSpeed);

    XtAddActions (actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable ("<Message>WM_PROTOCOLS: montanaQuit()"));
    frame = XtCreateManagedWidget ("frame", layoutWidgetClass, toplevel, NULL, 0);
    menuBar = XtCreateManagedWidget ("menuBar", layoutWidgetClass, frame, NULL, 0);
    fileMenuButton = XtCreateManagedWidget ("fileMenuButton",
					    kmenuButtonWidgetClass,
					    menuBar, NULL, ZERO);
    fileMenu = CreateMenu (fileMenuButton, "fileMenu",
			   fileMenuEntries, XtNumber (fileMenuEntries));
    deal = XtCreateManagedWidget ("deal", kcommandWidgetClass,
				     menuBar, NULL, ZERO);
    XtAddCallback(deal, XtNcallback, DealCallback, NULL);
    dealDisplay = XtCreateManagedWidget ("dealDisplay", klabelWidgetClass,
					frame, NULL, ZERO);
    newGame = XtCreateManagedWidget ("newGame", kcommandWidgetClass,
				     menuBar, NULL, ZERO);
    XtAddCallback(newGame, XtNcallback, NewGameCallback, NULL);
    undo = XtCreateManagedWidget ("undo", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(undo, XtNcallback, UndoCallback, NULL);
    hint = XtCreateManagedWidget ("hint", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(hint, XtNcallback, HintCallback, NULL);
    score = XtCreateManagedWidget ("score", kcommandWidgetClass,
				   menuBar, NULL, ZERO);
    XtAddCallback(score, XtNcallback, ScoreCallback, NULL);
    cards = XtCreateManagedWidget ("cards", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback (cards, XtNinputCallback, InputCallback, NULL);
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
