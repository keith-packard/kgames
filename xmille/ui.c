/*
 * ui.c
 *
 * interface routines for mille
 */

# include	"mille.h"
# include	"ui.h"
# include	<X11/Xutil.h>
# include	"gray.bm"

# include	"card.h"

struct color colorMap[NUM_COLOR] = {
	"black",	0,	0,
	"white",	0,	0,
	"red",		0,	0,
	"green",	0,	0,
	"light gray",	0,	0,
	"blue",		0,	0,
};

const char *const C_fmt = "%-18.18s";	/* format for printing cards	*/
char	Initstr[100];		/* initial string for error field	*/
char	*_cn[NUM_CARDS] = {	/* Card name buffer			*/
	"",
	"25",
	"50",
	"75",
	"100",
	"200",
	"Out of Gas",
	"Flat Tire",
	"Accident",
	"Stop",
	"Speed Limit", 
	"Gasoline",
	"Spare Tire",
	"Repairs",
	"Go",
	"End of Limit",
	"Extra Tank",
	"Puncture Proof",
	"Driving Ace",
	"Right of Way",
};
char	**C_name = &_cn[1];	/* Card names				*/

Display		*dpy;
int		screen;
Window		xwindow;
Pixmap		fill;
Button		QUIT, SAVE;
Window		qwindow, swindow;
XFontStruct	*font;
GC		text_gc;
GC		xor_gc;
Region		null, mine;

int	iscolor;

struct safety_offset safety_offsets[4] = {
	0,		0,
	WIDTH+PAD_CARD,	0,
	0,		HEIGHT+PAD_CARD,
	WIDTH+PAD_CARD,	HEIGHT+PAD_CARD,
};

char *
prune (orig, max)
char	*orig;
{
	static char	buf[512];
	int		len;
	char		c;

	len = strlen (orig);
	if (XTextWidth (font, orig, len) < max)
		return orig;
	strcpy (buf, orig);
	do {
		buf[--len] = '\0';
	} while (len > 0 && XTextWidth (font, buf, len) >= max);
	return buf;
}

Message (string)
char	*string;
{
	string = prune (string, MESS_W);
	displayString (MESS_X, MESS_Y, string);
}

void
Error (char *string, ...)
{
	va_list ap;
	char	buf[512];
	char	*o;

	va_start(string, ap);
	vsprintf (buf, string, ap);
	va_end(ap);
	o = prune (buf, ERROR_W);
	displayString (ERROR_X, ERROR_Y,  o);
}

Prompt (string)
char *string;
{
	string = prune (string, PROMPT_W);
	displayString (PROMPT_X, PROMPT_Y, string);
}

debug (pos, string, a0, a1, a2)
{
}

ComputerStatus (string)
{
/*
	char	buffer[512];

	sprintf (buffer, "I %-10.10s", string);
	XDrawImageString (dpy, xwindow, text_gc, COMP_CARD_TX, COMP_CARD_TY,
			  buffer, strlen(buffer));
*/
}

ComputerCard (type)
int	type;
{
	/*	displayCard (type, COMP_CARD_X, COMP_CARD_Y);*/
}

static int	computer_distance = 0;

ComputerDistance (distance)
{
	displayDistance (COMP_DIST_X, COMP_DIST_Y, distance, DIST_WIDTH, DIST_HEIGHT);
	computer_distance = distance;
}

EraseComputerDistance ()
{
	computer_distance = 0;
}

RedisplayComputerDistance ()
{
	displayDistance (COMP_DIST_X, COMP_DIST_Y, computer_distance, DIST_WIDTH, DIST_HEIGHT);
}

ComputerSpeed (type)
{
	displayCard (type, COMP_PLAY_X, COMP_PLAY_Y);
}

ComputerBattle (type)
{
	displayCard (type, COMP_PLAY_X + WIDTH + PAD_CARD, COMP_PLAY_Y);
}

static int computer_miles_count[5];

ComputerMiles (type, index, count)
{
	while (computer_miles_count[index] < count) {
		displayCard (type, COMP_PLAY_X + (WIDTH + PAD_CARD) * (index + 2),
		    COMP_PLAY_Y + (MILE_OFFSET * computer_miles_count[index]));
		++computer_miles_count[index];
	}
}

EraseComputerMiles ()
{
	int	i;

	for (i = 0; i < 5; i++)
		computer_miles_count[i] = 0;
}

ComputerSafety (type, index)
{
	displayCard (type, COMP_SAFE_X + safety_offsets[index].x,
	    COMP_SAFE_Y + safety_offsets[index].y);
}

DisplayDiscard (type)
{
	displayCard (type, DISCARD_X, DISCARD_Y);
}

DisplayDeck (numberLeft)
{
	char	buffer[512];

	sprintf (buffer, "Cards: %3d  ", numberLeft);
	displayString (DECK_TX, DECK_TY, buffer);
}

static int human_distance = 0;

HumanDistance (distance)
{
	displayDistance (HUM_DIST_X, HUM_DIST_Y, distance, DIST_WIDTH, DIST_HEIGHT);
	human_distance = distance;
}

EraseHumanDistance ()
{
	human_distance = 0;
}

RedisplayHumanDistance ()
{
	displayDistance (HUM_DIST_X, HUM_DIST_Y, human_distance, DIST_WIDTH, DIST_HEIGHT);
}

HumanSpeed (type)
{
	displayCard (type, HUM_PLAY_X, HUM_PLAY_Y);
}

HumanBattle (type)
{
	displayCard (type, HUM_PLAY_X + WIDTH + PAD_CARD, HUM_PLAY_Y);
}

static int human_miles_count[5];

HumanMiles (type, index, count)
{
	while (human_miles_count[index] < count) {
		displayCard (type, HUM_PLAY_X + (WIDTH + PAD_CARD) * (index + 2),
		    HUM_PLAY_Y + (MILE_OFFSET * human_miles_count[index]));
		++human_miles_count[index];
	}
}

EraseHumanMiles ()
{
	int	i;

	for (i = 0; i < 5; i++)
		human_miles_count[i] = 0;
}

HumanSafety (type, index)
{
	displayCard (type, HUM_SAFE_X + safety_offsets[index].x,
	    HUM_SAFE_Y + safety_offsets[index].y);
}

HumanHand (type, index)
int	type, index;
{
	displayCard (type, HUM_HAND_X + (WIDTH + PAD_CARD) * index, HUM_HAND_Y);
}

displayDistance (x, y, value, width, height)
{
	XFillRectangle (dpy, xwindow, colorMap[BLUE_COLOR].gc, x, y, (value * width) / 1000,
			height);
}

eraseDistance (x, y, value, width, height)
{
	XClearArea (dpy, xwindow, x, y, (value * width) / 1000, height, TRUE);
}

char *
GetpromptedInput (string)
char	*string;
{
	extern char	*co_prompted ();

	return co_prompted (string, xwindow);
}

newboard()
{
	int	i;
	char	buffer[20];
	int	len;
	int	x, y1, y2, ym1, ym2;
	int	width;

	XClearWindow (dpy, xwindow);
	cardEraseAll ();
	stringsEraseAll ();
	EraseHumanMiles ();
	EraseComputerMiles ();
	EraseHumanDistance ();
	EraseComputerDistance ();
	cardDisplay (&deck, DECK_X, DECK_Y);
	draw_board ();
	flushEvents (xwindow, ~0);
}

newscore()
{
	register int	i;

	InScore (-1, 0, "You");
	InScore (-1, 1, "Computer");
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
}

draw_board ()
{
	int	i, x, y1, ym1, y2, ym2, width;
	char	buffer[50];

	redraw_board ();
	newscore ();
	for (i = 0; i <= 1000; i += 100) {
		sprintf (buffer, "%d", i);
		x = COMP_DIST_TX + (i * DIST_WIDTH) / 1000;
		y1 = COMP_DIST_TY + MESS_H;
		ym1 = COMP_DIST_MY + MESS_H;
		y2 = HUM_DIST_TY + MESS_H;
		ym2 = HUM_DIST_MY + MESS_H;
		width = XTextWidth (font, buffer, strlen(buffer));
		displayString (x - width / 2, y1, buffer, strlen(buffer));
		displayString (x - width / 2, y2, buffer, strlen(buffer));
	}
}

redraw_board ()
{
	redraw_region (0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

exposeBoard (rep)
XExposeEvent	*rep;
{
	XRectangle	rect;

	rect.x = rep->x;
	rect.y = rep->y;
	rect.width = rep->width;
	rect.height = rep->height;

	XUnionRectWithRegion(&rect, mine, mine);
	if( rep->count == 0 )
	{
		XClipBox(mine, &rect);
		XIntersectRegion(null, mine, mine);
		redraw_region (rect.x, rect.y, rect.width, rect.height);
		/*redraw_board();*/
	}
}

redraw_region (xpos, ypos, w, h)
{
	int	x1, y1, x2, y2;
	int	i;
	int	ym1, ym2, x;
	char	buffer[20];
	int	width;

	XDrawImageString (dpy, xwindow, text_gc, DISCARD_TX, DISCARD_TY,
			     "Discard Pile", 12);
	RedisplayHumanDistance ();
	RedisplayComputerDistance ();
	x1 = HUM_HAND_X - PAD_CARD/2;
	y1 = HUM_HAND_Y - PAD_CARD/2;
	x2 = HUM_HAND_X + (WIDTH + PAD_CARD) * 7 - PAD_CARD/2;
	y2 = HUM_HAND_Y + (HEIGHT + PAD_CARD) - PAD_CARD/2;
	XDrawLine (dpy, xwindow, colorMap[BLACK_COLOR].gc, x1, y1, x2, y1);
	XDrawLine (dpy, xwindow, colorMap[BLACK_COLOR].gc, x2, y1, x2, y2);
	XDrawLine (dpy, xwindow, colorMap[BLACK_COLOR].gc, x2, y2, x1, y2);
	XDrawLine (dpy, xwindow, colorMap[BLACK_COLOR].gc, x1, y2, x1, y1);
	for (i = 0; i <= 1000; i += 100) {
		x = COMP_DIST_TX + (i * DIST_WIDTH) / 1000;
		y1 = COMP_DIST_TY + MESS_H;
		ym1 = COMP_DIST_MY + MESS_H;
		y2 = HUM_DIST_TY + MESS_H;
		ym2 = HUM_DIST_MY + MESS_H;
		XDrawLine (dpy, xwindow, colorMap[BLACK_COLOR].gc, x, ym1, x, ym1 + DIST_MARK);
		XDrawLine (dpy, xwindow, colorMap[BLACK_COLOR].gc, x, ym2, x, ym2 + DIST_MARK);
	}
	cardRedisplay (xpos, ypos, w, h);
	redisplayStrings (xpos, ypos, w, h);
}

char	*font_name;
char	*bold_font_name;

init_ui (argc, argv)
    int argc;
    char **argv;
{
	XColor	hardware_color, exact_color;
	XClassHint	xch;
	XWMHints	xwm;
	XSizeHints	xsh;
	int	i;
	int	do_quit (), do_save ();
	int	CmanageButton ();
	XGCValues	gcv;
	Colormap	def_cm;
	XEvent		ev;
	char		*def;
	extern double	animation_speed;
	unsigned long	gcmask;
	Pixmap		grayStipple;
	XSetWindowAttributes	winvalues;
	unsigned long		winmask;

	dpy = XOpenDisplay ((char *) 0);
	screen = DefaultScreen(dpy);
	def_cm = DefaultColormap(dpy, screen);

	if (def = XGetDefault (dpy, argv[0], "animationSpeed"))
		animation_speed = ((double) atoi (def)) / 100.0;
	
	font_name = "fixed";

	if (def = XGetDefault (dpy, argv[0], "font"))
		font_name = def;
	bold_font_name = "fixed";

	if (def = XGetDefault (dpy, argv[0], "boldFont"))
		bold_font_name = def;

	font = XLoadQueryFont (dpy, font_name);

	if (DisplayCells(dpy, screen) > 2)
		iscolor = 1;
	else {
		iscolor = 0;
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
	
	if (iscolor)
	{
		winvalues.background_pixel = colorMap[GREY_COLOR].pixel;
		winvalues.border_pixel = colorMap[BLACK_COLOR].pixel;
		winmask = CWBackPixel|CWBorderPixel;
	}
	else
	{
		winvalues.background_pixel = colorMap[WHITE_COLOR].pixel;
		winvalues.border_pixel = colorMap[BLACK_COLOR].pixel;
		winmask = CWBackPixel|CWBorderPixel;
	}
	
	text_gc = colorMap[BLACK_COLOR].gc;
	if (iscolor)
	    gcv.background = colorMap[GREY_COLOR].pixel;
	else
	    gcv.background = colorMap[WHITE_COLOR].pixel;
	XChangeGC (dpy, text_gc, GCBackground, &gcv);

	if (!font)
		font = XQueryFont (dpy, text_gc->gid);

	gcv.foreground = colorMap[WHITE_COLOR].pixel ^ colorMap[BLACK_COLOR].pixel;
	gcv.function = GXxor;
	xor_gc = XCreateGC (dpy, RootWindow (dpy, screen),
			    GCForeground|GCFunction, &gcv);

	xsh.flags = PSize | PMinSize | PMaxSize;
	xsh.width = xsh.min_width = xsh.max_width = WINDOW_WIDTH;
	xsh.height = xsh.min_height = xsh.max_height =  WINDOW_HEIGHT;

	xwindow = XCreateWindow (dpy, RootWindow(dpy, screen),
				 0, 0, xsh.width, xsh.height, 1,
				 DefaultDepth (dpy, screen),
 				 InputOutput, CopyFromParent,
 				 winmask, &winvalues);

	XStoreName(dpy, xwindow, "Xmille -- Version 1.1");
	XSetIconName(dpy, xwindow, "Xmille");
	
	xch.res_name = "xmille";
	xch.res_class = "XMille";
	XSetClassHint(dpy, xwindow, &xch);

	xwm.flags = InputHint | StateHint;
	xwm.input = True;
	xwm.initial_state = NormalState;
	XSetWMHints(dpy, xwindow, &xwm);

	XSetNormalHints(dpy, xwindow, &xsh);
	
	co_init (font_name, bold_font_name);
	QUIT = CcreateButton ("Quit", 50, colorMap[BLACK_COLOR].gc, font,
			      colorMap[WHITE_COLOR].pixel, 1);
	SAVE = CcreateButton ("Save", 50, colorMap[BLACK_COLOR].gc, font,
			      colorMap[WHITE_COLOR].pixel, 1);
	qwindow = CmapButton (xwindow, QUIT_X, QUIT_Y, QUIT, do_quit);
	swindow = CmapButton (xwindow, SAVE_X, SAVE_Y, SAVE, do_save);
	bindEvent (qwindow, ExposureMask|ButtonPressMask|LeaveWindowMask|ButtonReleaseMask,
	    CmanageButton);
	bindEvent (swindow, ExposureMask|ButtonPressMask|LeaveWindowMask|ButtonReleaseMask,
	    CmanageButton);
	XSelectInput (dpy, xwindow, ExposureMask);
	XMapWindow (dpy, xwindow);
	if (iscolor)
		init_color_cards ();
	else
		init_mono_cards();
	for (;;) {
		XNextEvent (dpy, &ev);
		if (ev.type == Expose);
			break;
	}
	bindEvent (xwindow, ExposureMask, exposeBoard);
	mine = XCreateRegion();
	null = XCreateRegion();
}


finish_ui ()
{
}

update_ui ()
{
	XFlush (dpy);
}

Beep ()
{
	XBell (dpy, 0);
}

/*
 *	Get a yes or no answer to the given question.  Saves are
 * also allowed.  Return TRUE if the answer was yes, FALSE if no.
 */

getyn(prompt)
register char	*prompt;
{

	register char	c;

	return co_affirm (prompt, xwindow);
}

static char	incharacter;
static int	gotcharacter;

static int	getmove_done;

mouse_event (rep)
XButtonEvent	*rep;
{
	int	x, y;

	x = rep->x;
	y = rep->y;
	if (HUM_HAND_Y <= y && y <= HUM_HAND_Y + HEIGHT &&
	    HUM_HAND_X <= x && x <= HUM_HAND_X + (WIDTH + PAD_CARD) * 7) {
		switch (rep->button & 0377) {
		case Button1:
			Movetype = M_PLAY;
			break;
		case Button2:
			Movetype = M_REASONABLE;
			break;
		case Button3:
			Movetype = M_DISCARD;
			break;
		}
		Card_no = (x - HUM_HAND_X) / (WIDTH + PAD_CARD);
		getmove_done = 1;
		return;
	}
	if (DECK_Y <= y && y <= DECK_Y + HEIGHT &&
	    DECK_X <= x && x <= DECK_X + WIDTH) {
		Movetype = M_DRAW;
		getmove_done = 1;
		return;
	}
	Beep ();
}

getmove()
{

	getmove_done = 0;
	bindEvent (xwindow, ButtonPressMask, mouse_event);
	while (!getmove_done) {
		dispatch ();
	}
	unbindEvent (xwindow, ButtonPressMask);
}


do_save ()
{
	save ();
}

do_quit ()
{
	rub();
}

# define	COMP_STRT	20
# define	CARD_STRT	2

prboard() {

	register PLAY	*pp;
	register int		i, j, k, temp;

	for (k = 0; k < 2; k++) {
		pp = &Player[k];
		temp = k * COMP_STRT + CARD_STRT;
		for (i = 0; i < NUM_SAFE; i++)
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
		for (i = C_25; i <= C_200; i++) {
			register char	*name;
			register int		end;

			name = C_name[i];
			temp = k * 40;
			end = pp->nummiles[i];
			if (k == 0)
				HumanMiles (i, C_200-i, end);
			else
				ComputerMiles (i, C_200-i, end);
		}
	}
	prscore(TRUE);
	temp = CARD_STRT;
	pp = &Player[PLAYER];
	for (i = 0; i < HAND_SZ; i++) {
		HumanHand (pp->hand[i], i);
	}
	DisplayDeck (Topcard - Deck);
	DisplayDiscard (Discard);
	if (End == 1000) {
		static char	ext[] = "Extension";

		/*		stand(EXT_Y, EXT_X, ext); */
	}
}

/*
 *	Put str at (y,x) in standout mode
 */

stand(y, x, str)
register int		y, x;
register char	*str;
{
}

InScore (line, player, text)
int	line, player;
char	*text;
{
	displayString (SCORE_X + player * SCORE_W,
			  SCORE_Y + SCORE_H * (line + 1), text);
}

struct displayedString {
	struct displayedString	*next;
	int	x, y;
	int	w;
	char	*data;
};

struct displayedString	*strings;

displayString (x, y, string)
char	*string;
{
	struct displayedString	*s;

	for (s = strings; s; s = s->next) {
		if (s->x == x && s->y == y)
			break;
	}
	if (s) {
		if (!strcmp (s->data, string))
			return;
	} else {
		s = (struct displayedString *) 
			malloc (sizeof (struct displayedString));
		s->x = x;
		s->y = y;
		s->data = 0;
		s->next = strings;
		strings = s;
	}
	if (s->data) {
		s->data = (char *) realloc (s->data, strlen (string) + 1);
	} else {
		s->data = (char *) malloc (strlen (string) + 1);
	}
	strcpy (s->data, string);
	s->w = XTextWidth (font, string, strlen (string));
	XDrawImageString (dpy, xwindow, text_gc, s->x, s->y, s->data, strlen (s->data));
}

stringsEraseAll ()
{
	struct displayedString	*s, *n;

	for (s = strings; s; s = n) {
		n = s->next;
		free (s->data);
		free (s);
	}
	strings = 0;
}

redisplayStrings (x, y, w, h)
{
	struct displayedString	*s;
	int			sx, sy, sw, sh;
	
	for (s = strings; s; s = s->next) {
		sx = s->x;
		sy = s->y - font->ascent;
		sw = s->w;
		sh = font->ascent + font->descent;
		if (x < sx +  sw && y < sy + sh &&
		    sx < x + w && sy < y + h)
			XDrawImageString (dpy, xwindow, text_gc,
 				s->x, s->y, s->data, strlen (s->data));
	}
}

prscore(for_real)
register bool	for_real;
{

	register PLAY	*pp;
	register int		x;
	register char	*Score_fmt = "%4d  ";
	char		buffer[512];

	ComputerDistance (Player[1].mileage);
	HumanDistance (Player[0].mileage);

	for (pp = Player; pp < &Player[2]; pp++) {
		x = (pp - Player) * 6 + 21;
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
}

FlushInput ()
{
}
