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
# include	<X11/Shell.h>
# include	<X11/Xos.h>
# include	<X11/Xaw/Box.h>
# include	<X11/Xaw/Dialog.h>
# include	<X11/Xaw/Cardinals.h>
# include	<X11/Xaw/Panner.h>
# include	"Dominos.h"
# include	<Xkw/Layout.h>
# include 	<Xkw/Message.h>
# include	<Xkw/Animate.h>
# include	<Xkw/KLabel.h>
# include	<Xkw/KCommand.h>
# include	<Xkw/KMenuButton.h>
# include	<Xkw/KSimple.h>
# include	<Xkw/KSimpleMenu.h>
# include	<Xkw/KSmeBSB.h>
# include	<Xkw/KScrollbar.h>
# include	<Xkw/KPorthole.h>
# include	<X11/Xutil.h>
# include	"dominos.h"
# include	"Dominos-res.h"
# include	<stdio.h>

Widget	    toplevel;
Widget	    frame;
Widget	    scroll_h;
Widget	    corner;
Widget	    scroll_v;
Widget	    porthole;
Widget	    board_w;
Widget	    player_porthole;
Widget	    player_scrollbar;
Widget	    player_w;
Widget	    message;
Widget	    menuBar;
Widget	    fileMenuButton;
Widget	    fileMenu;
Widget	    newGame;
Widget	    undo;
Widget	    hint;
Widget	    computerCount;
Widget	    draw;
Widget	    zoom_in;
Widget	    zoom_out;
Widget	    score_w[MAX_PLAYERS];
Widget	    drag;

int	    total_score[MAX_PLAYERS];
int	    last_score[MAX_PLAYERS];

char	    *player_names[MAX_PLAYERS] = { "You", "I" };
int	    game_over;

static void
ComputerMove (void);

static void
MakeLastPlayerVisible (void);

static void
GameOver (void);

static void
UndoGameOver (void);

static int
YesOrNo (Widget original, char *prompt);

static int
MakeFirstMove (void);

typedef struct _dominosResources {
    int		animationSpeed;
    String	saveFile;
} DominosResources, *DominosResourcesPtr;

DominosResources dominosResources;

static int
Count (DominoPtr domino)
{
    int	c = 0;

    for (; domino; domino = domino->peer[LinkPeer])
	c++;
    return c;
}

#define DISPLAY_BOARD	    1
#define DISPLAY_PLAYER	    2
#define DISPLAY_COMPUTER	    4

static void
DisplayDominos (int changes)
{
    int	count;

    if (changes & DISPLAY_PLAYER)
	DominosSetDominos (player_w, &player[0]);
    if (changes & DISPLAY_COMPUTER)
    {
	count = Count (player[1]);
	Message (computerCount, "I have %d domino%s.",
		 count, count != 1 ? "s" : "");
    }
    if (changes & DISPLAY_BOARD)
	DominosSetDominos (board_w, &board);
}

static void
MakeMove (DominoPtr *player,
	  DominoPtr source,
	  DominoPtr target,
	  Direction dir,
	  Direction orientation)
{
    if (game_over)
	return;
    PlayerMove (player, source, target, dir, orientation);
    if (!*player)
	GameOver ();
}

static int
MakeDraw (DominoPtr *player)
{
    if (game_over)
	return FALSE;
    if (!PlayerDraw (player, TRUE))
    {
	GameOver ();
	return FALSE;
    }
    return TRUE;
}

static int
PlayEdgeFunc (DominoPtr source,
	      DominoPtr target,
	      Direction dir,
	      Direction orientation,
	      pointer data)
{
    PlayPtr play = (PlayPtr) data;

    if (target == play->target &&
	source == play->source &&
	(play->dist == 0 || play->dir == dir))
    {
	MakeMove (play->player, source, target, dir, orientation);
	return FALSE;
    }
    return TRUE;
}

static Bool
Play (DominoPtr *player,
      DominoPtr domino,
      DominoPtr target,
      Direction dir,
      int dist)
{
    PlayRec play;

    if (!board)
    {
	PlayerFirstMove (player, domino);
	DisplayDominos (DISPLAY_PLAYER|DISPLAY_BOARD);
	if (!game_over)
	    ComputerMove ();
	return TRUE;
    }
    else
    {
	play.player = player;
	play.source = domino;
	play.target = target;
	play.dist = dist;
	play.dir = dir;
	if (!FindPlays (board, domino, PlayEdgeFunc,
			(pointer) &play))
	{
	    DisplayDominos (DISPLAY_PLAYER|DISPLAY_BOARD);
	    if (!game_over)
		ComputerMove ();
	    return TRUE;
	}
    }
    return FALSE;
}

static void
ComputerMove (void)
{
    PlayRec play;

    if (game_over)
	return;
    MessageStart ();
    for (;;)
    {
	if (FindPlay (&player[1], &play))
	{
	    MessageAppend ("I play %dx%d.", play.source->pips[0],
			   play.source->pips[1]);
	    MessageEnd (message);
	    MakeMove (&player[1], play.source, play.target,
		      play.dir, play.orientation);
	    DisplayDominos (DISPLAY_COMPUTER|DISPLAY_BOARD);
	    break;
	}
	MessageAppend ("I draw. ");
	if (!PlayerDraw (&player[1], TRUE))
	{
	    MessageEnd (message);
	    DisplayDominos (DISPLAY_COMPUTER);
	    GameOver ();
	    break;
	}
    }
}

static void
DisplayScores (void)
{
    int	    i;

    for (i = 0; i < NumPlayers; i++)
    {
	Message (score_w[i], "%s have %d point%s.",
		 player_names[i], total_score[i],
		 total_score[i] == 1 ? "" : "s");
    }
}

static void
Score (void)
{
    int		scores[MAX_PLAYERS];
    int		i;
    int		best, best_i;
    DominoPtr	domino;
    int		score;

    best = 1000;
    best_i = -1;
    for (i = 0; i < NumPlayers; i++)
    {
	scores[i] = 0;
	for (domino = player[i]; domino; domino = domino->peer[LinkPeer])
	    scores[i] += domino->pips[0] + domino->pips[1];
	if (scores[i] < best)
	{
	    best = scores[i];
	    best_i = i;
	}
    }
    score = 0;
    for (i = 0; i < NumPlayers; i++)
    {
	if (i != best_i)
	    score += scores[i];
	last_score[i] = 0;
    }
    score -= scores[best_i];
    Message (message, "%s win, score %d",
	     player_names[best_i], score);
    total_score[best_i] += score;
    last_score[best_i] = score;
    DisplayScores ();
}

static void
Draw (void)
{
    MakeDraw (&player[0]);
    DisplayDominos (DISPLAY_PLAYER);
    MakeLastPlayerVisible ();
}

static void
Hint (void)
{
    PlayRec play;

    if (game_over)
    {
	Message (message, "Game over.");
    }
    else if (FindPlay (&player[0], &play))
    {
	Message (message, "Play %dx%d on %dx%d.",
		 play.source->pips[0], play.source->pips[1],
		 play.target->pips[0], play.target->pips[1]);
    }
    else
    {
	Message (message, "Draw.");
    }
}

static void
GameOver (void)
{
    game_over = TRUE;
    Score ();
}

static void
UndoGameOver (void)
{
    int	    i;

    if (!game_over)
	return;
    game_over = FALSE;
    for (i = 0; i < NumPlayers; i++)
	total_score[i] -= last_score[i];
    DisplayScores ();
}

static void
NewGame (void)
{
    if (!game_over)
    {
	if (!YesOrNo (toplevel, "Abandon game in progress?"))
	    return;
    }
    do {
	ResetGame ();
	DisplayDominos (DISPLAY_COMPUTER|DISPLAY_PLAYER|DISPLAY_BOARD);
    } while (!MakeFirstMove ());
    game_over = FALSE;
}

static void
Save (void)
{
    FILE    *f;
    int	    i;

    f = fopen (dominosResources.saveFile, "w");
    if (!f)
	return;
    WriteDominos (f, pile);
    for (i = 0; i < NumPlayers; i++)
    {
	WriteDominos (f, player[i]);
    }
    WriteDominos (f, board);
    WriteScores (f, total_score, NumPlayers);
    WriteInt (f, game_over);
    fclose (f);
}

static int
Restore (void)
{
    FILE    *f;
    int	    i;
    DominoPtr	new_pile;
    DominoPtr	new_player[MAX_PLAYERS];
    DominoPtr	new_board;
    int		new_score[MAX_PLAYERS];
    int		new_game_over;

    f = fopen (dominosResources.saveFile, "r");
    if (!f)
	return FALSE;
    new_pile = ReadDominos (f);
    if (DominoErrno)
    {
	fclose (f);
	return FALSE;
    }
    for (i = 0; i < NumPlayers; i++)
    {
	new_player[i] = ReadDominos (f);
	if (DominoErrno)
	{
	    fclose (f);
	    return FALSE;
	}
    }
    new_board = ReadDominos (f);
    if (DominoErrno)
    {
	fclose (f);
	return FALSE;
    }
    ReadScores (f, new_score, NumPlayers);
    if (DominoErrno)
    {
	fclose (f);
	return FALSE;
    }
    ReadInt (f, &new_game_over);
    if (DominoErrno)
	new_game_over = FALSE;
    fclose (f);
    DisposeGame ();
    pile = new_pile;
    for (i = 0; i < NumPlayers; i++)
    {
	player[i] = new_player[i];
	total_score[i] = new_score[i];
    }
    board = new_board;
    game_over = new_game_over;
    DisplayScores ();
    if (game_over)
	NewGame ();
    else
	DisplayDominos (DISPLAY_COMPUTER|DISPLAY_PLAYER|DISPLAY_BOARD);
    return TRUE;
}

static void
GetSize (Widget widget, Dimension *w, Dimension *h)
{
    Arg		args[2];

    XtSetArg (args[0], XtNwidth, w);
    XtSetArg (args[1], XtNheight, h);
    XtGetValues (widget, args, 2);
}

static void
Center (Widget original, Widget new)
{
    Arg		args[2];
    Dimension	center_width, center_height;
    Dimension	prompt_width, prompt_height;
    Position	source_x, source_y, dest_x, dest_y;
    /*
     * place the widget in the center of the "parent"
     */
    GetSize (new, &prompt_width, &prompt_height);
    GetSize (original, &center_width, &center_height);
    source_x = (int)(center_width - prompt_width) / 2;
    source_y = (int)(center_height - prompt_height) / 3;
    XtTranslateCoords (original, source_x, source_y, &dest_x, &dest_y);
    XtSetArg (args[0], XtNx, dest_x);
    XtSetArg (args[1], XtNy, dest_y);
    XtSetValues (new, args, 2);
}

static int  yn_done, yn_answer;

static void
YesFunc (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    yn_answer = 1;
    yn_done = 1;
}

static void
NoFunc (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    yn_answer = 0;
    yn_done = 1;
}

static int
YesOrNo (Widget original, char *prompt)
{
    Arg	    args[3];
    XEvent  event;
    Widget  shell, dialog, label;

    XtSetArg (args[0], XtNmappedWhenManaged, FALSE);
    shell = XtCreateApplicationShell ("yesOrNo", transientShellWidgetClass,
				      args, ONE);
    dialog = XtCreateManagedWidget ("yesOrNoDialog", layoutWidgetClass,
				shell, NULL, ZERO);
    label = XtCreateManagedWidget ("yesOrNoLabel", klabelWidgetClass,
				dialog, NULL, ZERO);
    XkwDialogAddButton (dialog, "yesOrNoOk", YesFunc, NULL);
    XkwDialogAddButton (dialog, "yesOrNoNo", NoFunc, NULL);

    XtSetArg (args[0], XtNlabel, prompt);
    XtSetValues (label, args, 1);
    XtRealizeWidget (shell);
    if (XtIsRealized (original))
    {
	Center (original, shell);
	XtSetKeyboardFocus (original, dialog);
    }
    else
    {
	Dimension   prompt_width, prompt_height;
	Position    x, y;

	GetSize (shell, &prompt_width, &prompt_height);
	x = (XtScreen (shell)->width - (int) prompt_width) / 2;
	y = (XtScreen (shell)->height - (int) prompt_height) / 3;
	XtSetArg (args[0], XtNx, x);
	XtSetArg (args[1], XtNy, y);
	XtSetValues (shell, args, 2);
    }
    XtMapWidget (shell);
    yn_done = 0;
    while (!yn_done) {
	XtNextEvent (&event);
	XtDispatchEvent (&event);
    }
    XtSetKeyboardFocus (original, (Widget) None);
    XtDestroyWidget (shell);
    return yn_answer;
}

void
FileError (char *s)
{
    char    label[1024];

    sprintf (label, "%s: %s", dominosResources.saveFile, s);
    YesOrNo (toplevel, label);
}

static void
Quit (void)
{
    if (YesOrNo (toplevel, "Save game?"))
	Save ();
    exit (0);
}

static void
Undo (void)
{
    DominoPtr	*p;

    if (!undoList)
    {
	Message (message, "Nothing to undo.");
	return;
    }
    if (game_over)
	UndoGameOver ();
    Message (message, "Undo.");
    do {
	p = undoList->player;
	PlayerUndo ();
    } while (undoList && p != &player[0]);
    if (!undoList)
	MakeFirstMove ();
    DisplayDominos (DISPLAY_COMPUTER|DISPLAY_PLAYER|DISPLAY_BOARD);
}

static int
MakeFirstMove (void)
{
    DominoPtr	max_domino;
    DominoPtr	domino;
    DominoPtr	*max_player;
    int		i;

    max_domino = 0;
    max_player = 0;
    for (i = 0; i < NumPlayers; i++)
    {
	for (domino = player[i]; domino; domino = domino->peer[LinkPeer])
	{
	    if (IsDouble (domino))
	    {
		if (!max_domino || domino->pips[0] > max_domino->pips[0])
		{
		    max_domino = domino;
		    max_player = &player[i];
		}
	    }
	}
    }
    if (max_domino)
    {
	PlayerFirstMove (max_player, max_domino);
	i = DISPLAY_BOARD;
	if (max_player != &player[1])
	    i |= DISPLAY_PLAYER;
	else
	    i |= DISPLAY_COMPUTER;
	DisplayDominos (i);
	if (max_player != &player[1])
	    ComputerMove ();
	DisposeUndoList ();
	return TRUE;
    }
    return FALSE;
}

static void
SetScrollbar(Widget w,
	     Position p,
	     Dimension child,
	     Dimension parent)
{
    double pos = -(double) p / ((double) child - (double) parent);
    XkwScrollbarSetThumb(w, pos, (double) parent / (double) child);
}

static void
UpdateScrollbars(Widget child,
		 Position x, Position y,
		 Dimension child_width, Dimension child_height,
		 Dimension parent_width, Dimension parent_height)
{
    if (child == player_w) {
	SetScrollbar(player_scrollbar, x, child_width, parent_width);
    }
    if (child == board_w) {
	SetScrollbar(scroll_h, x, child_width, parent_width);
	SetScrollbar(scroll_v, y, child_height, parent_height);
    }
}

/* Called when scrollbar is moved */
static void
ScrollbarCallback (Widget w, XtPointer closure, XtPointer data)
{
    Widget child = (Widget) closure;
    double pos = *((double *) data);
    Arg args[4];
    Position	child_x, child_y;
    Dimension	child_width, child_height;
    Dimension  	parent_width, parent_height;
    XtOrientation orientation;

    /* child geometry */
    XtSetArg(args[0], XtNx, &child_x);
    XtSetArg(args[1], XtNy, &child_y);
    XtSetArg(args[2], XtNwidth, &child_width);
    XtSetArg(args[3], XtNheight, &child_height);
    XtGetValues(child, args, 4);

    /* parent geometry */
    XtSetArg(args[0], XtNwidth, &parent_width);
    XtSetArg(args[1], XtNheight, &parent_height);
    XtGetValues(XtParent(child), args, 2);

    /* scrollbar direction */
    XtSetArg(args[0], XtNorientation, &orientation);
    XtGetValues(w, args, 1);

    Dimension	parent_length, child_length;
    Position	child_position, new_child_position;
    if (orientation == XtorientVertical) {
	parent_length = parent_height;
	child_position = child_y;
	child_length = child_height;
    } else {
	parent_length = parent_width;
	child_position = child_x;
	child_length = child_width;
    }

    if (pos == XkwScrollbarPageDown)
	new_child_position = child_position - parent_length * 0.75;
    else if (pos == XkwScrollbarPageUp)
	new_child_position = child_position + parent_length * 0.75;
    else
	new_child_position = ((double) parent_length - (double) child_length) * pos;

    if (new_child_position < (Position) (parent_length - child_length))
	new_child_position = (Position) (parent_length - child_length);

    if (new_child_position > 0)
	new_child_position = 0;

    if (new_child_position != child_position) {
	if (orientation == XtorientVertical)
	    child_y = new_child_position;
	else
	    child_x = new_child_position;
	Arg args[2];
	XtSetArg(args[0], XtNx, child_x);
	XtSetArg(args[1], XtNy, child_y);
	XtSetValues(child, args, 2);
	UpdateScrollbars(child, child_x, child_y,
			 child_width, child_height,
			 parent_width, parent_height);
    }
}

/* Called when porthole has moved */
static void
PortholeCallback(Widget w, XtPointer closure, XtPointer data)
{
    Widget child = (Widget) data;
    Arg args[4];
    Position	child_x, child_y;
    Dimension	child_width, child_height;
    Dimension  	parent_width, parent_height;

    /* child geometry */
    XtSetArg(args[0], XtNx, &child_x);
    XtSetArg(args[1], XtNy, &child_y);
    XtSetArg(args[2], XtNwidth, &child_width);
    XtSetArg(args[3], XtNheight, &child_height);
    XtGetValues(child, args, 4);

    /* parent geometry */
    XtSetArg(args[0], XtNwidth, &parent_width);
    XtSetArg(args[1], XtNheight, &parent_height);
    XtGetValues(XtParent(child), args, 2);

    UpdateScrollbars(child, child_x, child_y,
		     child_width, child_height,
		     parent_width, parent_height);
}

static void
MakeLastPlayerVisible (void)
{
    Arg		args[10];
    Cardinal	n;
    Dimension	player_width, porthole_width;
    Position	x;

    n = 0;
    XtSetArg (args[n], XtNwidth, &player_width); n++;
    XtGetValues (player_w, args, n);
    n = 0;
    XtSetArg (args[n], XtNwidth, &porthole_width); n++;
    XtGetValues (player_porthole, args, n);
    x = porthole_width - player_width;
    if (x < 0)
    {
	n = 0;
	XtSetArg (args[n], XtNx, x); n++;
	XtSetValues (player_w, args, n);
    }
}

static void
Zoom(double ratio)
{
    Arg		args[1];
    Dimension	size;

    XtSetArg(args[0], XtNsize, &size);
    XtGetValues(board_w, args, 1);
    size = (Dimension) (size * ratio + 0.5);
    XtSetArg(args[0], XtNsize, size);
    XtSetValues(board_w, args, 1);
    MakeLastPlayerVisible();
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
UndoCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Undo ();
}

static void
DrawCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Draw ();
}

static void
ZoomInCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Zoom (1.25);
}

static void
ZoomOutCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Zoom (0.8);
}

static void
HintCallback (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    Hint ();
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

static int	    selected_player;
static DominoPtr    selected_domino;

static DominoPtr    drag_domino_ptr;
static DominoRec    drag_domino;

static void
TranslateCoords(Widget from, Widget to, Position *x, Position *y)
{
    Position	to_x, to_y;
    Position	from_x, from_y;

    XtTranslateCoords(to, 0, 0, &to_x, &to_y);
    XtTranslateCoords(from, 0, 0, &from_x, &from_y);

    *x += from_x - to_x;
    *y += from_y - to_y;
}

static void
Drag (Widget child, XEvent *event)
{
    Dimension	width, height;
    Arg		arg[2];

    XtSetArg(arg[0], XtNwidth, &width);
    XtSetArg(arg[1], XtNheight, &height);
    XtGetValues(drag, arg, 2);

    Position	x = event->xmotion.x - width / 2;
    Position	y = event->xmotion.y - height / 2;
    TranslateCoords(child, frame, &x, &y);

    XtWidgetGeometry request = {
	.x = x,
	.y = y,
	.stack_mode = Above,
	.request_mode = CWX | CWY | CWStackMode
    };

    XtMakeGeometryRequest(drag, &request, NULL);
}

static void
StartDrag (Widget child, XEvent *event)
{
    drag_domino.pips[0] = selected_domino->pips[0];
    drag_domino.pips[1] = selected_domino->pips[1];
    drag_domino_ptr = &drag_domino;
    DominosSetDominos(drag, &drag_domino_ptr);
    Drag(child, event);
    XtMapWidget(drag);
}

static void
StopDrag (void)
{
    XtUnmapWidget(drag);
}

static void
PlayerCallback (Widget w, XtPointer closure, XtPointer data)
{
    DominosInputPtr input = (DominosInputPtr) data;

    (void) w;
    (void) closure;
    switch (input->action) {
    case DominosActionStart:
	if (input->domino && input->distance == 0)
	{
	    selected_player = (intptr_t) closure;
	    selected_domino = input->domino;
	    selected_domino->hide = True;
	    DisplayDominos(DISPLAY_PLAYER);
	    StartDrag(w, &input->event);
	}
	break;
    case DominosActionDrag:
	Drag(w, &input->event);
	break;
    default:
	StopDrag();
	if (selected_domino) {
	    selected_domino->hide = False;
	    DisplayDominos(DISPLAY_PLAYER);
	    selected_domino = NULL;
	}
	break;
    }
}

static void
BoardCallback (Widget w, XtPointer closure, XtPointer data)
{
    DominosInputPtr input = (DominosInputPtr) data;

    (void) w;
    (void) closure;
    switch (input->action) {
    case DominosActionStop:
	StopDrag();
	if (selected_domino) {
	    selected_domino->hide = False;
	    if (!Play (&player[selected_player], selected_domino,
		       input->domino, input->direction, input->distance))
		DisplayDominos(DISPLAY_PLAYER);
	    selected_domino = NULL;
	}
	break;
    default:
	StopDrag();
	if (selected_domino) {
	    selected_domino->hide = False;
	    DisplayDominos(DISPLAY_PLAYER);
	    selected_domino = NULL;
	}
	break;
    }
}

static void
UndoAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Undo ();
}

static void
NewGameAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    NewGame ();
}

static void
QuitAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Quit ();
}

static void
HintAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Hint ();
}

static void
RestoreAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Restore ();
}

static void
SaveAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Save ();
}

static void
DrawAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Draw ();
}

static void
ZoomInAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Zoom (1.25);
}

static void
ZoomOutAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Zoom (0.8);
}

static void
YesAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    yn_answer = 1;
    yn_done = 1;
}

static void
NoAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    yn_answer = 0;
    yn_done = 0;
}

XtActionsRec	actions[] = {
    { "dominosUndo",	UndoAction, },
    { "dominosNewGame",	NewGameAction, },
    { "dominosQuit",	QuitAction, },
    { "dominosHint",	HintAction, },
    { "dominosRestore",	RestoreAction, },
    { "dominosSave",	SaveAction, },
    { "dominosDraw",	DrawAction, },
    { "dominosYes",	YesAction, },
    { "dominosNo",	NoAction, },
    { "dominosZoomIn",	ZoomInAction },
    { "dominosZoomOut",	ZoomOutAction },
};

struct menuEntry {
    char    *name;
    void    (*function)(Widget, XtPointer, XtPointer);
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

#define offset(field) XtOffsetOf(DominosResources, field)

XtResource resources[] = {
    { "animationSpeed", "AnimationSpeed", XtRInt, sizeof (int),
     offset(animationSpeed), XtRImmediate, (XtPointer) -1},
    { "saveFile", "SaveFile", XtRString, sizeof (String),
     offset(saveFile), XtRString, NULL},
};

XrmOptionDescRec options[] = {
    { "-size",		"*Dominos.size",	XrmoptionSepArg, NULL, },
};

static void
makeDefaultSaveFile (void)
{
    if (!dominosResources.saveFile || !*dominosResources.saveFile)
    {
	char	path[1024];

	sprintf (path, "%s/%s", getenv ("HOME"), ".dominos");
	dominosResources.saveFile = malloc (strlen (path) + 1);
	strcpy (dominosResources.saveFile, path);
    }
}

int
main (int argc, char **argv)
{
    Atom	wm_delete_window;
    int		i;
    int		restored;

    toplevel = XkwInitialize("Dominos", options, XtNumber(options),
			     &argc, argv, True, defaultResources);

    XtGetApplicationResources (toplevel, (XtPointer)&dominosResources, resources,
			       XtNumber (resources), NULL, 0);

    makeDefaultSaveFile ();

    AnimateSetSpeed (dominosResources.animationSpeed);

    XtAddActions (actions, XtNumber(actions));

    XtOverrideTranslations
	(toplevel,
	 XtParseTranslationTable ("<Message>WM_PROTOCOLS: dominosQuit()"));
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
    XtAddCallback(hint, XtNcallback, HintCallback, NULL);

    draw = XtCreateManagedWidget ("draw", kcommandWidgetClass,
				  menuBar, NULL, ZERO);
    XtAddCallback(draw, XtNcallback, DrawCallback, NULL);

    zoom_in = XtCreateManagedWidget ("zoom_in", kcommandWidgetClass,
				     menuBar, NULL, ZERO);
    XtAddCallback(zoom_in, XtNcallback, ZoomInCallback, NULL);

    zoom_out = XtCreateManagedWidget ("zoom_out", kcommandWidgetClass,
				      menuBar, NULL, ZERO);
    XtAddCallback(zoom_out, XtNcallback, ZoomOutCallback, NULL);

    for (i = 0; i < NumPlayers; i++)
    {
	char	foo[32];

	sprintf (foo, "score%d", i);
	score_w[i] = XtCreateManagedWidget(foo, klabelWidgetClass,
					   menuBar, NULL, ZERO);
    }
    porthole = XtCreateManagedWidget("porthole", kportholeWidgetClass,
				     frame, NULL, ZERO);

    scroll_h = XtCreateManagedWidget("scroll_h", kscrollbarWidgetClass,
				     frame, NULL, ZERO);

    corner = XtCreateManagedWidget("corner", ksimpleWidgetClass,
				   frame, NULL, ZERO);

    scroll_v = XtCreateManagedWidget("scroll_v", kscrollbarWidgetClass,
				     frame, NULL, ZERO);

    board_w = XtCreateManagedWidget ("board", dominosWidgetClass, porthole, NULL, 0);

    XtAddCallback(board_w, XtNinputCallback, BoardCallback, NULL);

    XtAddCallback (porthole, XtNcallback, PortholeCallback, NULL);

    XtAddCallback(scroll_h, XtNcallback, ScrollbarCallback, (XtPointer) board_w);

    XtAddCallback(scroll_v, XtNcallback, ScrollbarCallback, (XtPointer) board_w);

    player_porthole = XtCreateManagedWidget("player_porthole", kportholeWidgetClass,
					  frame, NULL, ZERO);

    player_scrollbar = XtCreateManagedWidget("player_scrollbar", kscrollbarWidgetClass,
					     frame, NULL, ZERO);

    player_w = XtCreateManagedWidget ("player", dominosWidgetClass,
				      player_porthole, NULL, 0);

    XtAddCallback (player_porthole, XtNcallback, PortholeCallback, NULL);

    XtAddCallback (player_scrollbar, XtNcallback, ScrollbarCallback,
		   (XtPointer) player_w);

    XtAddCallback(player_w, XtNinputCallback, PlayerCallback, NULL);

    drag = XtCreateManagedWidget("drag", dominosWidgetClass, frame, NULL, 0);

    XtSetMappedWhenManaged(drag, False);

    message = XtCreateManagedWidget ("message", klabelWidgetClass, frame, NULL, 0);

    computerCount = XtCreateManagedWidget ("computerCount", klabelWidgetClass, frame, NULL, ZERO);

    srandom (getpid () ^ time ((long *) 0));

    restored = Restore();

    MessageStart();
    MessageAppend("Keith's Dominos, Version 1.0.");
    if (restored)
	MessageAppend(" (Resuming existing game)");
    MessageEnd(message);

    if (!restored)
    {
	game_over = True;
	NewGame ();
	DisplayScores ();
    }

    XtRealizeWidget (toplevel);
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);

    XtMainLoop ();
    return 0;
}
