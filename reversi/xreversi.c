/*
 * Copyright © 2020 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#include <stdint.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <Xkw/Layout.h>
#include "Reversi.h"
#include <stdio.h>
#include "reversi.h"

static Widget	topLevel, layout, reversi, error;

static Widget	quit, hint_button, undoButton, restart;
static Widget	playerLabel, playWhite, playBlack, playBoth, playNeither;

static Widget	levelLabel, levelValue;
static Widget	turn;

static void
DoSetLevel (Widget w, XtPointer closure, XtPointer call_data);

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

typedef struct _Xreversi {
	int		animateTimeout;
	int		animateRepeat;
} Xreversi;

Xreversi    app_resources;

#define offset(field)	XtOffset(Xreversi*, field)

static XtResource resources[] = {
    {"animateTimeout", "AnimateTimeout", XtRInt, sizeof (int),
	offset (animateTimeout), XtRImmediate, (char *) 150},
    {"animateRepeat", "AnimateRepeat", XtRInt, sizeof (int),
	offset (animateRepeat), XtRImmediate, (char *) 4},
};

static XrmOptionDescRec options[] = {
{"-animate",	"*animateTimeout",	XrmoptionSepArg,	NULL}
};

static Widget
MakeCommandButton(Widget box, char *name, XtCallbackProc function)
{
  Widget w = XtCreateManagedWidget(name, commandWidgetClass, box, NULL, ZERO);
  if (function != NULL)
    XtAddCallback(w, XtNcallback, function, (caddr_t) NULL);
  return w;
}

static Widget
MakeRadioButton (
    Widget	    parent,
    char	    *name,
    XtCallbackProc  callback,
    int		    value,
    Widget	    group)
{
    Arg		args[2];
    Widget	button;
    Cardinal	n;

    n = 0;
    XtSetArg (args[n], XtNradioData, (caddr_t) (intptr_t) value); n++;
    if (group) {
	XtSetArg (args[n], XtNradioGroup, (caddr_t) (intptr_t) group); n++;
    }
    button = XtCreateManagedWidget (name, toggleWidgetClass,
				       parent, args, n);
    if (callback != NULL)
	XtAddCallback (button, XtNcallback, callback, (caddr_t) NULL);
    return button;
}

static Widget 
MakeStringBox(Widget parent, String name, String string)
{
    Arg args[5];
    Cardinal numargs = 0;
    Widget StringW;
    Widget Source;
    
    XtSetArg(args[numargs], XtNeditType, XawtextEdit); numargs++;
    XtSetArg(args[numargs], XtNstring, string); numargs++;
    
    StringW = XtCreateManagedWidget(name, asciiTextWidgetClass, 
			      parent, args, numargs);
    
    Source = XawTextGetSource (StringW);
    XtAddCallback (Source, XtNcallback, DoSetLevel, (caddr_t) NULL);
    return(StringW);
}

static void
waitForServer(Boolean yield);

extern int  com, defcom;
extern int  level;

int UIdone, UIret;

static void
DoQuit (Widget w, XtPointer closure, XtPointer call_data)
{
    (void) w;
    (void) closure;
    (void) call_data;
    dispError ("");
    UIdone = 1;
    UIret = 0;
}

static void
DoHint (Widget w, XtPointer closure, XtPointer call_data)
{
    (void) w;
    (void) closure;
    (void) call_data;
    dispError ("");
    doHint ();
}

static void
DoUndo (Widget w, XtPointer closure, XtPointer call_data)
{
    (void) w;
    (void) closure;
    (void) call_data;
    dispError ("");
    undo ();
}

static void
DoRestart (Widget w, XtPointer closure, XtPointer call_data)
{
    (void) w;
    (void) closure;
    (void) call_data;
    dispError ("");
    UIdone = 1;
    UIret = 1;
}

#define PLAY_WHITE	1
#define PLAY_BLACK	2
#define PLAY_BOTH	3
#define PLAY_NEITHER	4

static void
DoPlay (Widget w, XtPointer closure, XtPointer call_data)
{
    int	    current;

    (void) w;
    (void) closure;
    (void) call_data;
    current = (intptr_t) XawToggleGetCurrent (playWhite);
    switch (current)
    {
    case PLAY_WHITE:
	defcom = WHITE;
	com = WHITE;
	break;
    case PLAY_BLACK:
    	defcom = BLACK;
    	com = BLACK;
	break;
    case PLAY_BOTH:
	com = 0;
	break;
    case PLAY_NEITHER:
	com = 2;
	break;
    }
}

static void
SetPlay (void)
{
    int	current, should_be = 0;

    current = (intptr_t) XawToggleGetCurrent (playWhite);
    switch (com)
    {
    case WHITE:
	should_be = PLAY_WHITE;
	break;
    case BLACK:
	should_be = PLAY_BLACK;
	break;
    case 0:
	should_be = PLAY_BOTH;
	break;
    case 2:
	should_be = PLAY_NEITHER;
	break;
    }
    if (current != should_be)
    {
	XawToggleSetCurrent (playWhite, (caddr_t) (intptr_t) should_be);
    }
}

static void
DoMove (Widget w, XtPointer closure, XtPointer call_data)
{
    (void) w;
    (void) closure;
    (void) call_data;
    ReversiMovePtr  move = (ReversiMovePtr) call_data;
    dispError ("");
    domove (move->x + 1, move->y + 1);
}

static char levelString[100];

static Bool levelChanged;

static void
DoSetLevel (Widget w, XtPointer closure, XtPointer call_data)
{
    (void) w;
    (void) closure;
    (void) call_data;
    levelChanged = TRUE;
}

static void
DoSetLevelAction (Widget w, XEvent *event, String *string, Cardinal *num)
{
    (void) event;
    (void) string;
    (void) num;
    DoSetLevel(w, NULL, NULL);
}

static void
SetLevel (void)
{
    Widget  source;
    Arg	    args[1];
    char    *value;
    int	    newlevel;

    levelChanged = FALSE;
    source = XawTextGetSource (levelValue);
    XtSetArg (args[0], XtNstring, &value);
    XtGetValues (source, args, 1);
    if (sscanf (value, "%d", &newlevel) == 1)
	level = newlevel;
}

XtActionsRec xreversi_actions[] = {
    { "SetLevel",	    DoSetLevelAction, },
};

void
dispInit(int argc, char **argv)
{
    topLevel = XtInitialize( NULL, "Xreversi",
			     options, XtNumber(options),
			     &argc, argv );

    XtGetApplicationResources(topLevel, &app_resources, resources,
			      XtNumber(resources), NULL, 0);

    XtAppAddActions (XtWidgetToApplicationContext (topLevel),
		     xreversi_actions, XtNumber (xreversi_actions));

    layout = XtCreateManagedWidget ( "layout", layoutWidgetClass, topLevel, NULL, ZERO);
    reversi = XtCreateManagedWidget( "reversi", reversiWidgetClass, layout, NULL, ZERO );
    XtAddCallback (reversi, XtNstoneCallback, DoMove, (caddr_t) NULL);
    error = XtCreateManagedWidget ( "error", labelWidgetClass, layout, NULL, ZERO );
    quit = MakeCommandButton (layout, "quit", DoQuit);
    hint_button = MakeCommandButton (layout, "hint", DoHint);
    undoButton = MakeCommandButton (layout, "undo", DoUndo);
    restart = MakeCommandButton (layout, "restart", DoRestart);
    playerLabel = XtCreateManagedWidget ("playerLabel", labelWidgetClass, layout, NULL, ZERO);
    playWhite = MakeRadioButton (layout, "playWhite", DoPlay, PLAY_WHITE, (Widget) NULL);
    playBlack = MakeRadioButton (layout, "playBlack", DoPlay, PLAY_BLACK, playWhite);
    playBoth = MakeRadioButton (layout, "playBoth", DoPlay, PLAY_BOTH, playWhite);
    playNeither = MakeRadioButton (layout, "playNeither", DoPlay, PLAY_NEITHER, playWhite);
    SetPlay ();
    levelLabel = XtCreateManagedWidget ( "levelLabel", labelWidgetClass, layout, NULL, ZERO);
    sprintf (levelString, "%d", level);
    levelValue = MakeStringBox (layout, "levelValue", levelString);
    XtSetKeyboardFocus (layout, levelValue);
    turn = XtCreateManagedWidget ("turn", labelWidgetClass, layout, NULL, ZERO);
    XtRealizeWidget(topLevel);
}

int
playGame (void)
{
    UIdone = 0;

    while (!UIdone)
    {
	checkInput ();
	SetPlay ();
	waitForServer(True);
    }
    return UIret;
}

void
display (boardT board)
{
    int	    i, j;
    ReversiStone    stone;

    for (i = 1; i <= SIZE; i++)
	for (j = 1; j <= SIZE; j++)
	{
	    switch (board[i][j])
	    {
	    case BLACK:
		stone = StoneBlack;
		break;
	    case WHITE:
		stone = StoneWhite;
		break;
	    default:
		stone = StoneNone;
		break;
	    }
	    XawReversiSetSpot (reversi, i-1, j-1, stone);
	}
    XFlush (XtDisplay (topLevel));
}

void
dispError (char *s)
{
    Arg	args[1];

    XtSetArg (args[0], XtNlabel, s);
    XtSetValues (error, args, 1);
    waitForServer(False);
}

void
dispGrid (void)
{
}

void
dispEnd (void)
{
}

static void
waitForServer(Boolean yield)
{
    XSync (XtDisplay (topLevel), False);
    while (yield || XtPending ())
    {
	XEvent	event;
	XtNextEvent (&event);
	XtDispatchEvent (&event);
	if (levelChanged)
	    SetLevel ();
	yield = False;
    }
}

void
dispTurn (int player)
{
    static char	turnString[100];
    static int	oldPlayer = 100;
    Arg	args[1];

    SetPlay ();
    if (player != oldPlayer)
    {
	if (player == EMPTY)
	    sprintf (turnString, "Game over");
	else
	    sprintf (turnString, "%s's turn", 
		     player == WHITE ? "white" : "black");
    	XtSetArg (args[0], XtNlabel, turnString);
    	XtSetValues (turn, args, 1);
    	oldPlayer = player;
    }
    waitForServer (False);
}

void
dispMove (int x, int y, int player)
{
    ReversiStone	A, B;

    if (player == WHITE)
	A = StoneWhite;
    else
	A = StoneBlack;
    B = StoneNone;
    XawReversiAnimateSpot (reversi, x - 1, y - 1, A, B, 
			   (unsigned long) app_resources.animateTimeout,
			   app_resources.animateRepeat);
    dispError ("");
}

void
dispHint (int x, int y, int player)
{
    dispMove (x, y, player);
}
