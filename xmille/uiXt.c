/*
 * ui.c
 *
 * interface routines for mille
 */

# include	"mille.h"
# include	"uiXt.h"
# include	<X11/Intrinsic.h>
# include	<X11/StringDefs.h>
# include	<X11/Shell.h>
# include	<X11/Xos.h>
# include	<X11/Xaw/Paned.h>
# include	<X11/Xaw/Form.h>
# include	<X11/Xaw/Box.h>
# include	<Xkw/KCommand.h>
# include	<X11/Xaw/Dialog.h>
# include	<Xkw/KLabel.h>
# include	<Xkw/KMenuButton.h>
# include	<Xkw/KSimpleMenu.h>
# include	<Xkw/KSmeBSB.h>
# include	<X11/Xaw/AsciiText.h>
# include	<X11/Xaw/Cardinals.h>
# include	<Xkw/Hand.h>
# include	<Xkw/Thermo.h>
# include	<Xkw/Layout.h>
# include	<Xkw/Pad.h>
# include	<Xkw/KTextLine.h>
# include	<Xkw/Xkw.h>
# include	<Xkw/Message.h>
# include	<X11/Xutil.h>
# include	"gray.bm"
# include	"cards-svg.h"
# include	"Mille-res.h"
# include       "MilleCards.h"

# include	"card.h"

struct color colorMap[NUM_COLOR] = {
	{ "black",	0,	0, },
	{ "white",	0,	0, },
	{ "red",	0,	0, },
	{ "green",	0,	0, },
	{ "light gray",	0,	0, },
	{ "blue",	0,	0, },
};

Display		*dpy;
int		screen;
Pixmap		fill;
XFontStruct	*font, *backFont;
GC		text_gc;
GC		xor_gc;
Widget		toplevel;
Widget		layout;
Widget		menu_bar;
Widget		file_menu_button;
Widget		file_menu;
Widget		computer_play;
Widget		computer_miles;
Widget		computer_safeties;
Widget		computer_safety_label;
Widget		deck_count;
Widget		deck_hand;
Widget		score;
Widget		message, errors, prompt;
Widget		quit_button, save_button;
Widget		human_miles;
Widget		human_hand;
Widget		human_play;
Widget		human_safeties;
Widget		human_safety_label;
double		scale = 2.0;

Widget		yes_or_no_shell;
Widget		yes_or_no_dialog;
Widget		yes_or_no_label;

Widget		prompted_shell;
Widget		prompted_dialog;
Widget		prompted_value;
Widget		prompted_label;

#define NUM_COLS_IN_MILES   5
#define SPEED_CARD	    0
#define BATTLE_CARD	    1
#define MILES_OFFSET	    2
#define NUM_COLS_IN_PLAY    NUM_COLS_IN_MILES + MILES_OFFSET
#define NUM_COLS_IN_HAND    HAND_SZ

typedef struct recordHand {
    XtPointer	card;
    int		type;
} RecordHandRec, *RecordHandPtr;

struct menuEntry {
    char    *name;
    void    (*function)(Widget w, XtPointer closure, XtPointer data);
};

RecordHandRec	humanHandCards[NUM_COLS_IN_HAND];
RecordHandRec	humanPlayCards[2];
int		humanMiles[NUM_COLS_IN_MILES];
RecordHandRec	humanSafeties[4];
RecordHandRec	computerPlayCards[2];
int		computerMiles[NUM_COLS_IN_MILES];
RecordHandRec	computerSafeties[4];

#define DECK_DRAW	    0
#define DECK_DISCARD	    1
RecordHandRec	deckCards[2];

static int	getmove_done;

static void DoRestore (Widget w, XtPointer closure, XtPointer data);
static void DoSave (Widget w, XtPointer closure, XtPointer data);
static void DoQuit (Widget w, XtPointer closure, XtPointer data);

static struct menuEntry fileMenuEntries[] = {
    { "restore",    DoRestore, },
    { "save",	    DoSave, },
    { "quit",	    DoQuit, },
};

int	iscolor;

void
MilleMessage (char *string)
{
    Message (message, "%s", string);
}

void
VError(const char *string, va_list ap)
{
    MessageV(errors, string, ap);
}

void
Error (const char *string, ...)
{
    va_list ap;

    va_start(ap, string);
    VError(string, ap);
    va_end(ap);
}

void
Prompt (char *string)
{
    (void) string;
#ifdef NOTDEF
    displayString (prompt, string);
#endif
}

void
debug (int pos, char *string, int a0, int a1, int a2)
{
}

static int  yn_done, yn_answer;

static void YesFunc (Widget w, XtPointer closure, XtPointer data)
{
    yn_answer = 1;
    yn_done = 1;
}

static void NoFunc (Widget w, XtPointer closure, XtPointer data)
{
    yn_answer = 0;
    yn_done = 1;
}

void
ComputerCard (int type)
{
    (void) type;
    /*	displayCard (type, COMP_CARD_X, COMP_CARD_Y);*/
}

void
ComputerStatus (char *string)
{
    (void) string;
/*
	char	buffer[512];

	sprintf (buffer, "I %-10.10s", string);
*/
}

void
ComputerDistance (int distance)
{
    Arg	arg[1];

    XtSetArg (arg[0], XtNcurrent, distance);
    XtSetValues (computer_miles, arg, 1);
}

static void
UpdateCard (Widget w, RecordHandPtr array, int ind, int col, int row, int type)
{
    if (type == -1)
    {
	if (array[ind].card)
	{
	    HandRemoveCard (w, array[ind].card);
	    array[ind].card = NULL;
	}
    }
    else if (!array[ind].card || type != array[ind].type)
    {
	if (array[ind].card)
	    HandReplaceCard (w, array[ind].card, (XtPointer) (intptr_t) type, XkwHandDefaultOffset);
	else
	    array[ind].card = HandAddCard (w, (XtPointer) (intptr_t) type,
					   row, col, XkwHandDefaultOffset);
    }
    array[ind].type = type;
}

void
ComputerSpeed (int type)
{
    UpdateCard (computer_play, computerPlayCards, SPEED_CARD, SPEED_CARD, 0, type);
}

void
ComputerBattle (int type)
{
    UpdateCard (computer_play, computerPlayCards, BATTLE_CARD, BATTLE_CARD, 0, type);
}

void
ComputerMiles (int type, int ind, int count)
{
    while (computerMiles[ind] < count) {
	HandAddCard (computer_play, (XtPointer) (intptr_t) type,
		     InsertRow, ind + MILES_OFFSET, XkwHandDefaultOffset);
	++computerMiles[ind];
    }
}

void
EraseComputer (void)
{
    int	i;

    for (i = 0; i < NUM_COLS_IN_MILES; i++)
	computerMiles[i] = 0;
    for (i = 0; i < 2; i++)
	computerPlayCards[i].card = 0;
    HandRemoveAllCards (computer_play);
    for (i = 0; i < 4; i++)
	computerSafeties[i].card = 0;
    HandRemoveAllCards (computer_safeties);
    ComputerDistance (0);
}

void
ComputerSafety (int type, int ind)
{
    int	row, col;

    row = ind & 1;
    col = (ind & 2) >> 1;
    UpdateCard (computer_safeties, computerSafeties, ind, col, row, type);
}

void
DisplayDiscard (int type)
{
    UpdateCard (deck_hand, deckCards, DECK_DISCARD, DECK_DISCARD, 0, type);
}

void
DisplayDeck (int numberLeft)
{
    Message(deck_count, "Cards: %d", numberLeft);
}

void
HumanDistance (int distance)
{
    Arg	arg[1];

    XtSetArg (arg[0], XtNcurrent, distance);
    XtSetValues (human_miles, arg, 1);
}

void
HumanSpeed (int type)
{
    UpdateCard (human_play, humanPlayCards, SPEED_CARD, SPEED_CARD, 0, type);
}

void
HumanBattle (int type)
{
    UpdateCard (human_play, humanPlayCards, BATTLE_CARD, BATTLE_CARD, 0, type);
}

void
HumanMiles (int type, int ind, int count)
{
    while (humanMiles[ind] < count) {
	HandAddCard (human_play, (XtPointer) (intptr_t) type,
		     InsertRow, ind + MILES_OFFSET, XkwHandDefaultOffset);
	++humanMiles[ind];
    }
}

void
EraseHuman (void)
{
    int	i;

    for (i = 0; i < NUM_COLS_IN_MILES; i++)
	humanMiles[i] = 0;
    for (i = 0; i < 2; i++)
	humanPlayCards[i].card = 0;
    HandRemoveAllCards (human_play);
    for (i = 0; i < NUM_COLS_IN_HAND; i++)
	humanHandCards[i].card = 0;
    HandRemoveAllCards (human_hand);
    for (i = 0; i < 4; i++)
	humanSafeties[i].card = 0;
    HandRemoveAllCards (human_safeties);
    HumanDistance (0);
}

void
HumanSafety (int type, int ind)
{
    int	row, col;

    row = ind & 1;
    col = (ind & 2) >> 1;
    UpdateCard (human_safeties, humanSafeties, ind, col, row, type);
}

void
HumanHand (int type, int ind)
{
    UpdateCard (human_hand, humanHandCards, ind, ind, 0, type);
}

void
newboard(void)
{
    EraseHuman ();
    EraseComputer ();
}

void
newscore(void)
{
    int	    i;
    char    c;
    InScore (-1, 0, "  You");
    InScore (-1, 1, "Computer");
    c = XkwPadUnderline;
    for (i = 20; i < 48; i++)
	XkwPadAttributes (score, 0, i, &c, 1);
    InScore (0, -1, "Milestones");
    InScore (1, -1, "Safeties");
    InScore (2, -1, "All 4 Safeties");
    InScore (3, -1, "Coup Fourre");
    InScore (4, -1, "Trip Complete");
    InScore (5, -1, "Safe Trip");
    InScore (6, -1, "Delayed Action");
    InScore (7, -1, "Extension");
    InScore (8, -1, "Shut Out");
    InScore (9, -1, "Hand Total");
    InScore (10, -1, "Overall Total");
    InScore (11, -1, "Games");
    XkwPadUpdate (score);
}

void
draw_board (void)
{
}

void
redraw_board (void)
{
}

typedef struct _milleResources {
    XFontStruct	*font;
    XFontStruct	*backFont;
    int		animationSpeed;
    Boolean    	color;
    Boolean	clipCards;
} MilleResources, *MilleResourcesPtr;

MilleResources milleResources;

#define offset(field) XtOffsetOf(MilleResources, field)

#define COLOR_UNSET 10

XtResource resources[] = {
    { XtNfont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
     offset (font), XtRString, XtDefaultFont},
    { "backFont", XtCFont, XtRFontStruct, sizeof (XFontStruct *),
     offset (backFont), XtRString, XtDefaultFont},
    { "animationSpeed", "AnimationSpeed", XtRInt, sizeof (int),
     offset(animationSpeed), XtRImmediate, (XtPointer) 20},
    { "color", "Color", XtRBoolean, sizeof (Boolean),
     offset(color), XtRImmediate, (XtPointer) COLOR_UNSET },
    { "clipCards", "ClipCards", XtRBoolean, sizeof (Boolean),
     offset(clipCards), XtRImmediate, (XtPointer) TRUE},
};

static void yes_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) e;
    (void) p;
    (void) n;
    YesFunc (w, (XtPointer) NULL, (XtPointer) NULL);
}

static void no_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) e;
    (void) p;
    (void) n;
    NoFunc (w, (XtPointer) NULL, (XtPointer) NULL);
}

static void noop_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
}

static void card_action(String *p, Cardinal *n, int m)
{
    if (*n == 1) {
	Movetype = m;
	Card_no = atoi (*p) - 1;
	getmove_done = 1;
    }
    else
    {
	Beep ();
    }
}

static void discard_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    card_action (p, n, M_DISCARD);
}

static void draw_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Movetype = M_DRAW;
    getmove_done = 1;
}

static void play_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) p;
    (void) n;
    card_action (p, n, M_PLAY);
}

static void reasonable_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    card_action (p, n, M_REASONABLE);
}

static void order_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    (void) e;
    (void) p;
    (void) n;
    Order = !Order;
    Movetype = M_ORDER;
    getmove_done = 1;
}

static void quit_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    DoQuit (w, (XtPointer) 0, (XtPointer) 0);
}

static void save_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    DoSave (w, (XtPointer) 0, (XtPointer) 0);
}

static void restore_action (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) w;
    DoRestore (w, (XtPointer) 0, (XtPointer) 0);
}

XtActionsRec actions[] = {
    { "milleYes", yes_action, },
    { "milleNo", no_action, },
    { "milleCancel", no_action, },
    { "Noop", noop_action, },
    { "milleDiscard", discard_action, },
    { "milleDraw", draw_action, },
    { "millePlay", play_action, },
    { "milleReasonable", reasonable_action, },
    { "milleOrder", order_action, },
    { "milleQuit", quit_action, },
    { "milleSave", save_action, },
    { "milleRestore", restore_action, },
};

static void
InputCallback (Widget w, XtPointer closure, XtPointer data)
{
    HandInputPtr    input = (HandInputPtr) data;

    switch (input->action) {
    default:
        break;
    case HandActionClick:
        animate_enable(1);
        if (input->start.w == human_hand)
        {
            Movetype = M_REASONABLE;
            getmove_done = 1;
            Card_no = input->start.col;
        }
        else if (input->start.w == deck_hand)
        {
            Movetype = M_DRAW;
            getmove_done = 1;
            Card_no = input->start.col;
        }
        break;
    case HandActionDrag:
        if (input->start.w == input->current.w)
            break;
        animate_enable(0);
        if (input->start.w == human_hand) {
            if (input->current.w == human_miles ||
                input->current.w == human_play ||
                input->current.w == human_safeties)
            {
                Movetype = M_PLAY;
                getmove_done = 1;
                Card_no = input->start.col;
            }
            else if (input->current.w == deck_hand)
            {
                Movetype = M_DISCARD;
                getmove_done = 1;
                Card_no = input->start.col;
            }
            break;
        }
        else if (input->start.w == deck_hand)
        {
            if (input->current.w == human_hand) {
                Movetype = M_DRAW;
                getmove_done = 1;
                Card_no = input->start.col;
            }
        }
    }
}

static Widget
make_hand (char *name, Widget parent, int rows, int cols, Bool overlap_rows)
{
    Widget	hand;
    Arg		args[20];
    Cardinal    i = 0;
    int		display_x, display_y;

    XtSetArg (args[i], XtNcardWidth, WIDTH * scale); i++;
    XtSetArg (args[i], XtNcardHeight, HEIGHT * scale); i++;
    XtSetArg (args[i], XtNnumRows, rows); i++;
    XtSetArg (args[i], XtNnumCols, cols); i++;
    if (!overlap_rows) {
	XtSetArg (args[i], XtNrowOffset, HEIGHT * scale + WIDTH * scale/10); i++;
    }
    XtSetArg (args[i], XtNcolOffset, WIDTH * scale + WIDTH * scale/10); i++;
    display_x = 0;
    display_y = 0;
    if (rows == 1)
	display_x = 8;
    else
	display_y = 8;
    XtSetArg (args[i], XtNdisplayX, display_x); i++;
    XtSetArg (args[i], XtNdisplayY, display_y); i++;
    XtSetArg (args[i], XtNdisplayWidth, WIDTH * scale - display_x * 2); i++;
    XtSetArg (args[i], XtNdisplayHeight, HEIGHT * scale - display_y * 2); i++;
    hand = XtCreateManagedWidget (name, milleCardsWidgetClass, parent, args, i);
    XtAddCallback (hand, XtNinputCallback, InputCallback, NULL);
    return hand;
}

static void
DoRestore (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    rest();
    prboard();
}

static void
DoSave (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    save ();
}

static void
DoQuit (Widget w, XtPointer closure, XtPointer data)
{
    (void) w;
    (void) closure;
    (void) data;
    rub (0);
}

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

int		clip_cards;

void
init_ui (int *argc, char **argv)
{
    XColor	hardware_color, exact_color;
    int	i;
    XGCValues	gcv;
    Colormap	def_cm;
    extern double	animation_speed;
    unsigned long	gcmask;
    Pixmap		grayStipple;
    Arg			arg[2];
    Visual		*visual;

    toplevel = XkwInitialize ("Mille", 0, 0,
			      argc, argv, True, defaultResources);

    Arg	args[1];
    XtSetArg(args[0], XtNinput, True);
    XtSetValues(toplevel, args, ONE);

    dpy = XtDisplay (toplevel);
    screen = DefaultScreen(dpy);
    def_cm = DefaultColormap(dpy, screen);
    visual = DefaultVisual(dpy, screen);

    XtGetApplicationResources (toplevel, (XtPointer)&milleResources, resources,
			       XtNumber (resources), NULL, 0);

    XtAddActions (actions, XtNumber(actions));
    animation_speed = milleResources.animationSpeed;
    clip_cards = milleResources.clipCards;

    font = milleResources.font;
    backFont = milleResources.backFont;

    if (milleResources.color == COLOR_UNSET)
    {
	iscolor = TRUE;
	if (visual->map_entries < 3)
	    iscolor = FALSE;
    }
    else
        iscolor = milleResources.color;

    if (!iscolor)
    {
	grayStipple = XCreateBitmapFromData (dpy, RootWindow (dpy, screen),
				    gray_bits, gray_width, gray_height);
    }

    for (i = 0; i < NUM_COLOR; i++) {
	    if (!iscolor && i > WHITE_COLOR) {
		    gcv.foreground = WhitePixel (dpy, screen);
		    gcv.background = BlackPixel (dpy, screen);
		    gcv.fill_style = FillOpaqueStippled;
		    gcv.stipple = grayStipple;
		    gcmask = GCForeground|GCBackground|GCFillStyle|GCStipple;
	    } else {
		    XAllocNamedColor (dpy, def_cm, colorMap[i].name,
				      &hardware_color, &exact_color);
		    colorMap[i].pixel = hardware_color.pixel;
		    gcv.foreground = hardware_color.pixel;
		    gcmask = GCForeground;
	    }
	    if (font) {
		    gcv.font = font->fid;
		    gcmask |= GCFont;
	    }
	    colorMap[i].gc = XCreateGC (dpy, RootWindow(dpy, screen),
					gcmask, &gcv);
    }

    text_gc = colorMap[BLACK_COLOR].gc;
    if (iscolor)
	gcv.background = colorMap[GREY_COLOR].pixel;
    else
	gcv.background = colorMap[WHITE_COLOR].pixel;
    XChangeGC (dpy, text_gc, GCBackground, &gcv);

    if (!font)
	    font = XQueryFont (dpy, XGContextFromGC (text_gc));
    if (!backFont)
	    backFont = font;
    gcv.foreground = colorMap[WHITE_COLOR].pixel ^ colorMap[BLACK_COLOR].pixel;
    gcv.function = GXxor;
    gcv.subwindow_mode = IncludeInferiors;
    xor_gc = XCreateGC (dpy, RootWindow (dpy, screen),
			GCForeground|GCFunction|GCSubwindowMode, &gcv);

    if (iscolor)
	init_color_cards ();
    else
	init_mono_cards();
    layout = XtCreateManagedWidget ("layout", layoutWidgetClass, toplevel, NULL, (Cardinal) 0);
    menu_bar = XtCreateManagedWidget ("menuBar", boxWidgetClass, layout, NULL, ZERO);
    file_menu_button = XtCreateManagedWidget ("fileMenuButton", kmenuButtonWidgetClass, menu_bar, NULL, ZERO);
    file_menu = CreateMenu (file_menu_button, "fileMenu", fileMenuEntries, XtNumber (fileMenuEntries));
    computer_play = make_hand ("computerPlay", layout, 3, 7, True);
    computer_miles = XtCreateManagedWidget ("computerMiles", thermoWidgetClass, layout, NULL, ZERO);
    computer_safeties = make_hand ("computerSafeties", layout, 2, 2, False);
    computer_safety_label = XtCreateManagedWidget ("computerSafetyLabel", klabelWidgetClass, layout, NULL, ZERO);
    deck_count = XtCreateManagedWidget ("deckCount", klabelWidgetClass, layout, NULL, ZERO);
    deck_hand = make_hand ("deck", layout, 1, 2, False);
    XtSetArg (arg[0], XtNnumRows, 13);
    XtSetArg (arg[1], XtNnumCols, 48);
    score = XtCreateManagedWidget ("score", padWidgetClass, layout,
				   arg, TWO);
    HandAddCard (deck_hand, (XtPointer) -2, 0, 0, XkwHandDefaultOffset);
    message = XtCreateManagedWidget ("message", klabelWidgetClass, layout, NULL, ZERO);
    errors = XtCreateManagedWidget ("errors", klabelWidgetClass, layout, NULL, ZERO);
    human_miles = XtCreateManagedWidget ("humanMiles", thermoWidgetClass, layout, NULL, ZERO);
    human_play = make_hand ("humanPlay", layout, 3, 7, True);
    human_hand = make_hand ("humanHand", layout, 1, 7, False);
    human_safeties = make_hand ("humanSafeties", layout, 2, 2, False);
    human_safety_label = XtCreateManagedWidget ("humanSafetyLabel", klabelWidgetClass, layout, NULL, ZERO);
    newscore ();
    prscore (FALSE);
    XtRealizeWidget (toplevel);
    yes_or_no_shell = XtCreatePopupShell ("yesOrNo", transientShellWidgetClass,
			        toplevel, NULL, ZERO);
    yes_or_no_dialog = XtCreateManagedWidget ("yesOrNoDialog", layoutWidgetClass,
				yes_or_no_shell, NULL, ZERO);
    yes_or_no_label = XtCreateManagedWidget ("yesOrNoLabel", klabelWidgetClass,
				yes_or_no_dialog, NULL, ZERO);
    XkwDialogAddButton (yes_or_no_dialog, "yesOrNoOk", YesFunc, NULL);
    XkwDialogAddButton (yes_or_no_dialog, "yesOrNoNo", NoFunc, NULL);
    XtRealizeWidget (yes_or_no_shell);
    prompted_shell = XtCreatePopupShell ("prompted", transientShellWidgetClass,
				toplevel, NULL, ZERO);
    XtSetArg (arg[0], XtNvalue, "");
    prompted_dialog = XtCreateManagedWidget ("promptedDialog", layoutWidgetClass,
				prompted_shell, arg, ONE);
    prompted_label = XtCreateManagedWidget ("promptedLabel", klabelWidgetClass,
				prompted_dialog, NULL, ZERO);
    prompted_value = XtCreateManagedWidget ("promptedValue", ktextLineWidgetClass,
				prompted_dialog, NULL, ZERO);
    XtAddCallback(prompted_value, XtNcallback, YesFunc, NULL);
    XkwDialogAddButton (prompted_dialog, "promptedOk", YesFunc, NULL);
    XkwDialogAddButton (prompted_dialog, "promptedCancel", NoFunc, NULL);
    XtRealizeWidget (prompted_shell);
}

static void
DisplayText (Widget w, int row, int col, char *text)
{
    XkwPadText (w, row, col, text, strlen (text));
}

void
finish_ui (void)
{
}

void
update_ui (void)
{
	XFlush (dpy);
}

void
Beep (void)
{
	XBell (dpy, 0);
}

/*
 *	Get a yes or no answer to the given question.  Saves are
 * also allowed.  Return TRUE if the answer was yes, FALSE if no.
 */

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
    XtSetArg (args[0], XtNwidth, &center_width);
    XtSetArg (args[1], XtNheight, &center_height);
    XtGetValues (original, args, 2);
    XtSetArg (args[0], XtNwidth, &prompt_width);
    XtSetArg (args[1], XtNheight, &prompt_height);
    XtGetValues (new, args, 2);
    source_x = (int)(center_width - prompt_width) / 2;
    source_y = (int)(center_height - prompt_height) / 3;
    XtTranslateCoords (original, source_x, source_y, &dest_x, &dest_y);
    XtSetArg (args[0], XtNx, dest_x);
    XtSetArg (args[1], XtNy, dest_y);
    XtSetValues (new, args, 2);
}

int
getyn(char *prompt)
{
    Arg	    args[1];
    XEvent  event;

    XtSetArg (args[0], XtNlabel, prompt);
    XtSetValues (yes_or_no_label, args, 1);
    Center (toplevel, yes_or_no_shell);
    XtMapWidget (yes_or_no_shell);
    XtSetKeyboardFocus (toplevel, yes_or_no_dialog);
    yn_done = 0;
    while (!yn_done) {
	XtNextEvent (&event);
	XtDispatchEvent (&event);
    }
    XtSetKeyboardFocus (toplevel, (Widget) None);
    XtUnmapWidget (yes_or_no_shell);
    return yn_answer;
}

char *
GetpromptedInput (char *string)
{
    Arg	    args[1];
    XEvent  event;
    char    *value;

    XtSetArg (args[0], XtNlabel, string);
    XtSetValues (prompted_label, args, 1);
    Center (toplevel, prompted_shell);
    XtMapWidget (prompted_shell);
    XtSetKeyboardFocus (prompted_dialog, prompted_value);
    XtSetKeyboardFocus (toplevel, prompted_value);
    yn_done = 0;
    while (!yn_done) {
	XtNextEvent (&event);
	XtDispatchEvent (&event);
    }
    XtSetKeyboardFocus (toplevel, (Widget) None);
    XtUnmapWidget (prompted_shell);
    if (yn_answer) {
	XtSetArg(args[0], XtNstring, &value);
	XtGetValues (prompted_value, args, ONE);
	return value;
    } else
	return NULL;
}

void
getmove(void)
{
    XEvent  event;

    getmove_done = 0;
    while (!getmove_done)
    {
	XtNextEvent (&event);
	XtDispatchEvent (&event);
    }
    MilleMessage ("");
    Error ("", "");
}

void
do_save (void)
{
	save ();
}

void
do_quit (void)
{
	rub(0);
}

# define	COMP_STRT	20
# define	CARD_STRT	2

void
prboard(void)
{

	PLAY	*pp;

	for (int k = 0; k < 2; k++) {
		pp = &Player[k];
		for (int i = 0; i < NUM_SAFE; i++)
			if (pp->safety[i] == S_PLAYED) {
				if (k == 0) {
					HumanSafety (i + S_CONV, i);
				} else {
					ComputerSafety (i + S_CONV, i);
				}
			}
		if (k == 0) {
			HumanBattle (pp->battle);
			HumanSpeed (pp->speed);
		} else {
			ComputerBattle (pp->battle);
			ComputerSpeed (pp->speed);
		}
		for (int i = C_25; i <= C_200; i++) {
			int		end;

			end = pp->nummiles[i];
			if (k == 0)
				HumanMiles (i, C_200-i, end);
			else
				ComputerMiles (i, C_200-i, end);
		}
	}
	prscore(TRUE);
	pp = &Player[PLAYER];
	for (int i = 0; i < HAND_SZ; i++) {
		HumanHand (pp->hand[i], i);
	}
	DisplayDeck (Topcard - Deck);
	DisplayDiscard (Discard);
	if (End == 1000) {
		/*		stand(EXT_Y, EXT_X, ext); */
	}
}

/*
 *	Put str at (y,x) in standout mode
 */

void
stand(int y, int x, char *str)
{
}

void
InScore (int line, int player, char *text)
{
    DisplayText (score, line + 1, 20 + player * 20, text);
#ifdef NOTDEF
	displayString (SCORE_X + player * SCORE_W,
			  SCORE_Y + SCORE_H * (line + 1), text);
#endif
}

void
prscore(bool for_real)
{
	PLAY		*pp;
	const char Score_fmt[] = "%4d  ";
	char		buffer[512];

	ComputerDistance (Player[1].mileage);
	HumanDistance (Player[0].mileage);

	for (pp = Player; pp < &Player[2]; pp++) {
		sprintf (buffer, Score_fmt, pp->mileage);
		InScore (0, pp - Player, buffer);
		sprintf (buffer, Score_fmt, pp->safescore);
		InScore (1, pp - Player, buffer);
		if (pp->safescore == 400)
			InScore (2, pp - Player, " 300 ");
		else
			InScore (2, pp - Player, "   0 ");
		sprintf (buffer, Score_fmt, pp->coupscore);
		InScore (3, pp - Player, buffer);
#ifdef EXTRAP
		if (for_real)
			finalscore(pp);
		else
			extrapolate(pp);
#else
		finalscore(pp);
#endif
		sprintf (buffer, Score_fmt, pp->hand_tot);
		InScore (9, pp - Player, buffer);
		sprintf (buffer, Score_fmt, pp->total);
		InScore (10, pp - Player, buffer);
		sprintf (buffer, Score_fmt, pp->games);
		InScore (11, pp - Player, buffer);
	}
	XkwPadUpdate (score);
}

void
FlushInput (void)
{
}
