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
 */

#include 	<stdlib.h>
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
#include	"MonteCarlo-res.h"

Widget      toplevel;
Widget      frame;
Widget      cards;
Widget      deck;
Widget      deckCount;
Widget      waste;
Widget      wasteLabel;
Widget      message;
Widget      menuBar;
Widget      fileMenuButton;
Widget      fileMenu;
Widget      deal;
Widget      newGame;
Widget      undo;
Widget      hint;
Widget      score;

#define NUM_DEALS   2

#define NUM_ROWS    5
#define NUM_COLS    5
#define NUM_CARDS   52

#define	MAX_SCORE	52

CardStackRec cardStacks[NUM_CARDS];
CardStackRec deckStack;
CardStackRec wasteStack;	/* matched cards */
CardRec     rawcards[NUM_CARDS];

CardStackPtr position[NUM_CARDS];

CardStackPtr fromStack;
CardPtr     fromCard;
int         currentScore;
int         cardsLeft;
CardStackPtr hintStack;

typedef struct _montecarloResources {
    int         animationSpeed;
}           MonteCarloResources, *MonteCarloResourcesPtr;

MonteCarloResources montecarloResources;

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#define ForAllCardVars	int row, col; CardStackPtr  stack
#define ForAllCards	for (row = 0, stack = cardStacks; row < NUM_ROWS; row++) \
			    for (col = 0; col < NUM_COLS; col++, stack++)

static void
InitStacks (void)
{
    ForAllCardVars;

    ForAllCards
    {
	CardInitStack(stack, cards, CardsNone, True, row,
		      CardDisplayAll);
	stack->basePosition = col;
    }
    CardInitStack(&deckStack, deck, CardsEmpty,
		  False, 0, CardDisplayBottom);
    CardInitStack(&wasteStack, (Widget) waste, CardsEmpty,
		  False, 0, CardDisplayTop);
    currentScore = 0;
}

static void
GenerateCards (void)
{
    CardGenerateStandardDeck(rawcards);
    deckStack.first = &rawcards[0];
    deckStack.last = &rawcards[NUM_CARDS - 1];
}

static void
SetPosition(CardStackPtr stack)
{
    int         i;

    i = stack->last - rawcards;
    position[i] = stack;
}

static void
FirstDeal (void)
{
    ForAllCardVars;

    ForAllCards
    {
	CardMove(&deckStack, deckStack.last, stack, False);
	SetPosition(stack);
	CardTurn(stack->last, CardFaceUp, False);
	cardsLeft--;
    }
    Message(deckCount, "%d", cardsLeft);
}

static int
ComputeScore (void)
{
    int         score;
    CardPtr     t;

    /* simple point-per-removal */
    score = 0;
    t = wasteStack.last;
    while (t) {
	score++;
	t = t->prev;
    }

    return score;
}

static void
DisplayStacks (void)
{
    ForAllCardVars;

    ForAllCards
	CardDisplayStack(stack);
    CardDisplayStack(&deckStack);
    CardDisplayStack(&wasteStack);
    CardsUpdateDisplay(cards);
}

static Boolean
MonteCarloUndo(void)
{
    if (!CardUndo())
	return False;
    DisplayStacks();
    return True;
}

static void
MonteCarloDo(CardStackPtr from_stack, CardStackPtr to_stack)
{
    CardPtr     card;

    card = from_stack->last;
    CardMove(from_stack, card, &wasteStack, True);
    card = to_stack->last;
    CardMove(to_stack, card, &wasteStack, True);
    CardNextHistory();
}


/* User interface functions */

static void
UnHint(void);

static CardStackPtr
NextStack(int r, int c)
{
    ForAllCardVars;

    for (row = r; row < NUM_ROWS; row++) {
	for (col = c; col < NUM_COLS; col++, stack++) {
	    stack = &cardStacks[row * NUM_COLS + col];
	    if (stack->last)
		return stack;
	}
	c = 0;			/* reset for remaining rows */
    }

    return NULL;
}

static CardStackPtr
FirstEmpty(void)
{
    ForAllCardVars;

    ForAllCards {
	if (!stack->last)
	    return stack;
    }
    return NULL;
}

static void
Deal(void)
{
    ForAllCardVars;
    CardPtr     card;
    CardStackPtr next_stack;

    UnHint();
    stack = cardStacks;

    for (row = 0; row < NUM_ROWS; row++) {
	stack = &cardStacks[row * NUM_COLS];
	for (col = 0; col < NUM_COLS; col++, stack++) {
	    if (stack->last)
		continue;
	    /* XXX otherwise space, so move it up */
	    next_stack = NextStack(row, col);
	    if (next_stack) {
		CardMove(next_stack, next_stack->last, stack, True);
	    }
	    DisplayStacks();	/* i like to see them move */
	}
    }
    next_stack = FirstEmpty();
    while (next_stack && deckStack.last) {
	if ((card = deckStack.last)) {
	    CardMove(&deckStack, card, next_stack, True);
	    CardTurn(card, CardFaceUp, True);
	    DisplayStacks();	/* i like to see them move */
	    cardsLeft--;
	}
	next_stack = FirstEmpty();
    }
    Message(deckCount, "%d", cardsLeft);

    CardNextHistory();
}

static void
NewGame (void)
{
    CardsRemoveAllCards(cards);
    fromStack = 0;
    fromCard = 0;
    hintStack = 0;
    cardsLeft = NUM_CARDS;
    InitStacks();
    GenerateCards();
    CardShuffle(&deckStack, False);
    FirstDeal();
    CardInitHistory();
    DisplayStacks();
    Message(message, "MonteCarlo version 0.9");
}

static void
Undo (void)
{
    if (!MonteCarloUndo()) {
	Message(message, "Nothing to undo.");
	return;
    }
    Message(message, "Undo.");
    CardSetAnimate(False);
    DisplayStacks();
    CardSetAnimate(True);
}

static void
Score (void)
{
    Message(message, "Current position scores %d out of %d.",
	    ComputeScore(), MAX_SCORE);
}

static void
Quit(void)
{
    exit(0);
}

static Bool
NextTo(CardStackPtr from_stack, CardStackPtr to_stack)
{
    if ((abs(from_stack->position - to_stack->position) > 1) ||
	    (abs(from_stack->basePosition - to_stack->basePosition) > 1))
	return False;
    else
	return True;
}

static void
Play(CardStackPtr from_stack, CardStackPtr to_stack)
{
    CardPtr     card;

    UnHint();
    if (from_stack == to_stack)
	return;
    card = from_stack->last;
    if (!card) {
	Message(message, "Can't move spaces around.");
	return;
    }
    if (!to_stack->last) {
	Message(message, "Target is empty.");
	return;
    }
    if (to_stack->last->card.rank != card->card.rank) {
	Message(message, "%P and %P are of different ranks.",
		&card->card, &to_stack->last->card);
	return;
    }
    if (!NextTo(to_stack, from_stack)) {
	Message(message, "%P and %P aren't next to each other.",
		&card->card, &to_stack->last->card);
	return;
    }
    MonteCarloDo(from_stack, to_stack);
    if (ComputeScore() == MAX_SCORE)
	Message(message, "We have a winner!.");
}

#define MAX_MOVES   16

static CardStackPtr
FindMove(CardStackPtr src)
{
    int         r,
                c;

    ForAllCardVars;

    r = src->position;
    c = src->basePosition;
    /* test is '<', so add 2 */
    for (row = max(r - 1, 0); row < min(r + 2, NUM_ROWS); row++) {
	for (col = max(c - 1, 0); col < min(c + 2, NUM_COLS); col++) {
	    if ((row == r) && (col == c))
		continue;
	    stack = &cardStacks[row * NUM_COLS + col];
	    if (!stack->last)
		continue;
	    if (src->last->card.rank == stack->last->card.rank)
		return stack;
	}
    }
    return NULL;
}

static void
BestMove(void)
{
    ForAllCardVars;
    CardStackPtr best,
                first;
    Bool        seen_first = False,
                has_gaps = False;

    ForAllCards {
	if (!stack->last)
	    continue;
	best = FindMove(stack);
	if (best) {
	    Message(message, "Match %P and %P.",
		    &stack->last->card, &best->last->card);
	    return;
	}
    }
    /* see if there're any empties, and where they are in the stacks */
    first = FirstEmpty();
    ForAllCards {
	stack = &cardStacks[row * NUM_COLS + col];
	if (stack != first && !seen_first)
	    continue;
	seen_first = True;
	if (stack->last) {
	    has_gaps = True;
	    break;
	}
    }
    if (deckStack.last && first)
	Message(message, "Deal.");
    else if (ComputeScore() == MAX_SCORE)
	Message(message, "We have a winner!.");
    else if (has_gaps)
	Message(message, "Deal.");
    else
	Message(message, "Its all over.");
}

static void
UnHint(void)
{
    if (hintStack) {
	CardTurn(hintStack->last, CardFaceUp, True);
	hintStack = 0;
    }
}

static void
Hint(CardStackPtr stack)
{
    hintStack = stack;
    CardTurn(stack->last, CardFaceDown, True);
}

static void
FindAMove(CardStackPtr stack)
{
    CardStackPtr hint_stack;

    UnHint();
    hint_stack = FindMove(stack);
    if (hint_stack)
	Hint(hint_stack);
}

static void
Restore(void)
{
    Message(message, "Restore not implemented");
}

static void
Save(void)
{
    Message(message, "Save not implemented");
}

/* Callbacks to user interface functions */

static void
CardsCallback (Widget w, XtPointer closure, XtPointer data)
{
    HandInputPtr    input = (HandInputPtr) data;
    CardStackPtr    stack = NULL;
    String      type = "";
    int         i;

#define MOVE	0
#define HINT	1
#define UNHINT	2
#define	SELECT	3

    (void) closure;
    stack = &cardStacks[input->row * NUM_COLS + input->col];

    if (*input->num_params)
        type = *input->params;

    switch (input->action) {
    case HandActionStart:
        Message (message, "");
        if (!strcmp(type, "source")) {
            if (!fromStack)
                i = SELECT;
            else
                i = MOVE;
        } else if (!strcmp(type, "clear")) {
            fromStack = NULL;
        } else {
            return;
        }
        switch (i) {
        case SELECT:
            fromStack = stack;
            if (!fromStack->last)
                Message(message, "Selected space is empty.");
            break;
        case MOVE:
            if (fromStack) {
                Play(fromStack, stack);
                fromStack = NULL;
            }
            break;
        }
        DisplayStacks();
	break;
    case HandActionDrag:
        break;
    case HandActionExpand:
        FindAMove(stack);
        DisplayStacks();
        break;
    case HandActionStop:
        UnHint();
        DisplayStacks();
        break;
    }
}

static void
DealCallback (Widget w, XtPointer closure, XtPointer data)
{
    HandInputPtr    input = (HandInputPtr) data;

    (void) w;
    (void) closure;
    if (!input || input->action == HandActionStart)
        Deal();
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

static void DealAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Deal ();
}

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

XtActionsRec actions[] = {
    { "montecarloUndo", UndoAction, },
    { "montecarloDeal", DealAction, },
    { "montecarloNewGame", NewGameAction, },
    { "montecarloScore", ScoreAction, },
    { "montecarloQuit", QuitAction, },
    { "montecarloRestore", RestoreAction, },
    { "montecarloSave", SaveAction, },
    { "montecarloBestMove", BestMoveAction, },
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

#define offset(field) XtOffsetOf(MonteCarloResources, field)

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
main(int argc, char **argv)
{
    Atom        wm_delete_window;

    toplevel = XkwInitialize("MonteCarlo", options, XtNumber(options),
			     &argc, argv, True, defaultResources);

    XtGetApplicationResources(toplevel, (XtPointer) & montecarloResources, resources,
			      XtNumber(resources), NULL, 0);

    AnimateSetSpeed(montecarloResources.animationSpeed);

    XtAddActions(actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable("<Message>WM_PROTOCOLS: montecarloQuit()"));
    frame = XtCreateManagedWidget("frame", layoutWidgetClass, toplevel, NULL, 0);
    menuBar = XtCreateManagedWidget("menuBar", layoutWidgetClass, frame, NULL, 0);
    fileMenuButton = XtCreateManagedWidget("fileMenuButton",
					   kmenuButtonWidgetClass,
					   menuBar, NULL, ZERO);
    fileMenu = CreateMenu(fileMenuButton, "fileMenu",
			  fileMenuEntries, XtNumber(fileMenuEntries));
    deal = XtCreateManagedWidget("deal", kcommandWidgetClass,
				 menuBar, NULL, ZERO);
    XtAddCallback(deal, XtNcallback, DealCallback, NULL);
    newGame = XtCreateManagedWidget("newGame", kcommandWidgetClass,
				    menuBar, NULL, ZERO);
    XtAddCallback(newGame, XtNcallback, NewGameCallback, NULL);
    undo = XtCreateManagedWidget("undo", kcommandWidgetClass,
				 menuBar, NULL, ZERO);
    XtAddCallback(undo, XtNcallback, UndoCallback, NULL);
    hint = XtCreateManagedWidget("hint", kcommandWidgetClass,
				 menuBar, NULL, ZERO);
    XtAddCallback(hint, XtNcallback, HintCallback, NULL);
    score = XtCreateManagedWidget("score", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(score, XtNcallback, ScoreCallback, NULL);
    cards = XtCreateManagedWidget("cards", cardsWidgetClass, frame, NULL, 0);
    XtAddCallback(cards, XtNinputCallback, CardsCallback, NULL);

    deck = XtCreateManagedWidget("deck", cardsWidgetClass, frame, NULL, 0);
    deckCount = XtCreateManagedWidget("deckCount", klabelWidgetClass, frame, NULL, 0);
    XtAddCallback(deck, XtNinputCallback, DealCallback, NULL);

    waste = XtCreateManagedWidget("waste", cardsWidgetClass, frame, NULL, 0);
    wasteLabel = XtCreateManagedWidget("wasteLabel", klabelWidgetClass, frame, NULL, 0);
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
