/*
 * $NCDId$
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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Converters.h>
#include <Xkw/Xkw.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "DominosP.h"
#include <cairo/cairo-xlib.h>

#define offset(field)	XtOffsetOf(DominosRec, dominos.field)

#define COLOR_UNSET 10

static XtResource resources[] = {
    {XtNfaceColor, XtCFaceColor, XtRRenderColor, sizeof (XRenderColor),
     offset(face_color), XtRString, (XtPointer) "black"},
    {XtNpipsColor, XtCPipsColor, XtRRenderColor, sizeof (XRenderColor),
     offset(pips_color), XtRString, (XtPointer) "white"},
    {XtNbackground, XtCBackground, XtRRenderColor, sizeof (XRenderColor),
     offset(background), XtRString, (XtPointer) "white"},
    {XtNroundDominos, XtCRoundDominos, XtRBoolean, sizeof (Boolean),
     offset(round_dominos), XtRImmediate, (XtPointer) True},
    {XtNsize, XtCSize, XtRDimension, sizeof (Dimension),
     offset(size), XtRImmediate, (XtPointer) 60},
    {XtNinputCallback, XtCInputCallback, XtRCallback, sizeof (XtPointer),
     offset(input_callback), XtRCallback, (XtPointer) NULL},
};

#undef offset
#undef hoffset

#include	<X11/Xmu/Drawing.h>

static char defaultTranslations[] =
"<Btn1Down>: select()\n"
"<Btn2Down>: select()\n"
"<Btn3Down>: select()\n"
"<Btn4Down>: zoomout()\n"
"<Btn5Down>: zoomin()";

#define SuperClass  ((SimpleWidgetClass)&simpleClassRec)

#define INSET(w)	((w)->dominos.size / 20.0)

#ifndef MAX
#define MAX(a,b)	    ((a) < (b) ? (b) : (a))
#endif
#define LINE_WIDTH(w)	    MAX (((w)->dominos.size / 30), 2)
#define PIP_SIZE(w)	    MAX (((w)->dominos.size / 10), 2)
#define PIP_OFF(w)    	    ((w)->dominos.size / 5)

#define DOMINO_MINOR_SIZE(w)   ((w)->dominos.size)
#define DOMINO_MAJOR_SIZE(w)   ((w)->dominos.size * 2)

#define DOMINO_MAJOR_WIDTH(w)  DOMINO_MAJOR_SIZE(w)
#define DOMINO_MAJOR_HEIGHT(w) DOMINO_MAJOR_SIZE(w)
#define DOMINO_MINOR_WIDTH(w)  DOMINO_MINOR_SIZE(w)
#define DOMINO_MINOR_HEIGHT(w) DOMINO_MINOR_SIZE(w)

#define DominoUpright(d)    ((d)->orientation == North || \
			     (d)->orientation == South)

static int
DominoWidth (DominosWidget w, DominoPtr d)
{
    if (DominoUpright(d))
	return DOMINO_MINOR_WIDTH(w);
    else
	return DOMINO_MAJOR_WIDTH(w);
}

static int
DominoHeight (DominosWidget w, DominoPtr d)
{
    if (DominoUpright(d))
	return DOMINO_MAJOR_HEIGHT(w);
    else
	return DOMINO_MINOR_HEIGHT(w);
}

#define DominoX(w,d)	    (DominoWidth (w, d) / 2)
#define DominoY(w,d)	    (DominoHeight (w, d) / 2)

#define ScreenNo(w) XScreenNumberOfScreen (XtScreen (w))

static void
ClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

#define UsualSuspects(w)	Display *dpy = XtDisplay ((Widget) w); \
				Window	window = XtWindow ((Widget) w); \
				Pixmap tmp_map = (w)->dominos.tmp_map

#define GetScreen(w)		int screen = ScreenNo(w)

/*ARGSUSED*/
static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    DominosWidget	new = (DominosWidget) gnew;

    (void) greq;
    (void) args;
    (void) count;
    if (!new->core.width)
	new->core.width = DOMINO_MAJOR_WIDTH(new);
    if (!new->core.height)
	new->core.height = DOMINO_MAJOR_HEIGHT(new);
}

#define MotionMask ( \
	PointerMotionMask | Button1MotionMask | \
	Button2MotionMask | Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask )
#define PointerGrabMask ( \
	ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask | \
	PointerMotionHintMask | KeymapStateMask | \
	MotionMask )

static void
Realize (Widget widget, XtValueMask *value_mask, XSetWindowAttributes *attributes)
{
    unsigned int    event_mask = 0;
#define MAX_BUT	256
    unsigned char   mapping[MAX_BUT];
    int	    i, max;

    (*SuperClass->core_class.realize)(widget, value_mask, attributes);
    if (*value_mask & CWEventMask)
	event_mask = attributes->event_mask;
    event_mask &= PointerGrabMask;
    if (event_mask & ButtonPressMask)
    {
	max = XGetPointerMapping (XtDisplay (widget), mapping, MAX_BUT);
	for (i = 0; i < max; i++)
	{
	    if (mapping[i] != 0)
		XtGrabButton (widget, i, AnyModifier, True, event_mask,
			      GrabModeAsync, GrabModeAsync, None, None);
	}
    }
}

static void
Destroy (Widget gw)
{
    (void) gw;
}

static void
BoardSize (DominosWidget w, DominoPtr b, RectPtr r, int x, int y);

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    DominosWidget cur = (DominosWidget) gcur;
    DominosWidget new = (DominosWidget) gnew;

    (void) greq;
    (void) args;
    (void) count;
    if (cur->dominos.size != new->dominos.size && new->dominos.board && *new->dominos.board)
    {
	RectRec			size;
	Dimension		prefered_width, prefered_height, width, height;
	XtGeometryResult	result;

	BoardSize (new, *(new->dominos.board), &size, 0, 0);
	prefered_width = size.x2 - size.x1;
	prefered_height = size.y2 - size.y1;
	if (prefered_width < DOMINO_MAJOR_WIDTH(new))
	    prefered_width = DOMINO_MAJOR_WIDTH(new);
	if (prefered_height < DOMINO_MAJOR_HEIGHT(new))
	    prefered_height = DOMINO_MAJOR_HEIGHT(new);
	if (prefered_width != new->core.width || prefered_height != new->core.height)
	{
	    result = XtMakeResizeRequest ((Widget) new,
					  prefered_width, prefered_height,
					  &width, &height);
	    if (result == XtGeometryAlmost)
		result = XtMakeResizeRequest ((Widget) new,
					      width, height, NULL, NULL);
	}
    }
    return TRUE;
}

static int
i_sqrt (int a)
{
    double	    d_a;

    d_a = (double) a;
    d_a = sqrt (d_a);
    return (int) d_a;
}

static int
PeerX(DominosWidget w, DominoPtr d, Direction dir)
{
    switch (dir) {
    default:
    case North:
    case South:
	return 0;
    case West:
	return -DominoX(w, d) - DominoWidth(w, d->peer[dir]) + DominoX(w, d->peer[dir]);
    case East:
	return -DominoX(w, d) + DominoWidth(w, d) + DominoX(w, d->peer[dir]);
    }
    /*NOTREACHED*/
}

static int
PeerY(DominosWidget w, DominoPtr d, Direction dir)
{
    switch (dir) {
    default:
    case East:
    case West:
	return 0;
    case North:
	return -DominoY(w, d) - DominoHeight(w, d->peer[dir]) +
	        DominoY(w, d->peer[dir]);
    case South:
	return -DominoY(w, d) + DominoHeight(w, d) +
	        DominoY(w, d->peer[dir]);
    }
    /*NOTREACHED*/
}

static DominoPtr
XYInDomino (DominosWidget w,
	    DominoPtr b,
	    int x,
	    int y,
	    int test_x,
	    int test_y,
	    int *distp,
	    Direction *dirp)
{
    RectRec	r;
    Direction	dir;
    DominoPtr	peer, closest;
    Direction	to_dir, x_dir, y_dir;
    Direction	sub_dir;
    int		to_dist;
    int		x_dist, y_dist;
    int		sub_dist;

    r.x1 = x - DominoX(w, b);
    r.y1 = y - DominoY(w, b);
    r.x2 = r.x1 + DominoWidth(w, b);
    r.y2 = r.y1 + DominoHeight(w, b);
    to_dir = North;
    to_dist = 0;
    if (test_x < r.x1)
    {
	x_dist = r.x1 - test_x;
	x_dir = West;
    }
    else if (r.x2 < test_x)
    {
	x_dir = East;
	x_dist = test_x - r.x2;
    }
    else
    {
	x_dir = West;
	x_dist = 0;
    }
    if (test_y < r.y1)
    {
	y_dir = North;
	y_dist = r.y1 - test_y;
    }
    else if (r.y2 < test_y)
    {
	y_dir = South;
	y_dist = test_y - r.y2;
    }
    else
    {
	y_dir = South;
	y_dist = 0;
    }
    if (x_dist >= y_dist)
	to_dir = x_dir;
    else
	to_dir = y_dir;
    to_dist = i_sqrt (x_dist * x_dist + y_dist * y_dist);
    closest = b;
    if (to_dist > 0)
    {
	for (dir = North; dir <= West; dir++)
	{
	    if (b->peer[dir])
	    {
		peer = XYInDomino (w, b->peer[dir],
				   x + PeerX(w, b, dir), y + PeerY(w, b, dir),
				   test_x, test_y, &sub_dist, &sub_dir);
		if (sub_dist < to_dist)
		{
		    to_dist = sub_dist;
		    to_dir = sub_dir;
		    closest = peer;
		    if (sub_dist == 0)
			break;
		}
	    }
	}
    }
    *distp = to_dist;
    *dirp = to_dir;
    return closest;
}

static DominoPtr
XYToDomino (DominosWidget w,
	    int x,
	    int y,
	    int *distp,
	    Direction *dirp)
{
    if (w->dominos.board && *w->dominos.board)
	return XYInDomino (w, *w->dominos.board,
			   w->dominos.x_off, w->dominos.y_off, x, y,
			   distp, dirp);
    return NULL;
}

static void
ActionSelect (Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    DominosWidget   w = (DominosWidget) gw;
    DominoPtr	    d;
    DominosInputRec input;
    Direction	    dir;
    int		    dist;

    d = XYToDomino (w, event->xbutton.x, event->xbutton.y, &dist, &dir);
    input.w = gw;
    input.domino = d;
    input.direction = dir;
    input.distance = dist;
    input.params = params;
    input.event = *event;
    input.num_params = num_params;
    XtCallCallbackList (gw, w->dominos.input_callback, (XtPointer) &input);
}

static void
Zoom(Widget w, double ratio)
{
    Arg		args[1];
    Dimension	size;

    XtSetArg(args[0], XtNsize, &size);
    XtGetValues(w, args, 1);
    size = (Dimension) (size * ratio + 0.5);
    XtSetArg(args[0], XtNsize, size);
    XtSetValues(w, args, 1);
}

static void
ZoomInAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) e;
    (void) p;
    (void) n;
    Zoom (w, 1.25);
}

static void
ZoomOutAction (Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void) e;
    (void) p;
    (void) n;
    Zoom (w, 0.8);
}

static void
set_source_render(cairo_t *cr, XRenderColor *color)
{
    cairo_set_source_rgb(cr,
			 color->red / 65535.0,
			 color->green / 65535.0,
			 color->blue / 65535.0);
}

static void
OutlineDomino (DominosWidget w, cairo_t *cr, DominoPtr d, double width, double height)
{
    set_source_render(cr, &w->dominos.pips_color);
    if (DominoUpright (d)) {
	cairo_move_to(cr, 0, height/2.0);
	cairo_line_to(cr, width, height/2.0);
    } else {
	cairo_move_to(cr, width / 2.0, 0);
	cairo_line_to(cr, width / 2.0, height);
    }
    XkwDrawRoundedRect(cr, width, height, INSET(w));
    cairo_stroke(cr);
}

static void
FillDomino (DominosWidget w, cairo_t *cr, DominoPtr d)
{
    double	width, height;

    width = DominoWidth(w, d) - INSET(w) * 2;
    height = DominoHeight(w, d) - INSET(w) * 2;
    set_source_render(cr, &w->dominos.face_color);
    cairo_save(cr);
    cairo_translate(cr, INSET(w), INSET(w));
    XkwDrawRoundedRect(cr, width, height, INSET(w));
    cairo_fill(cr);
    OutlineDomino (w, cr, d, width, height);
    cairo_restore(cr);
}

static void
BoardSize (DominosWidget w, DominoPtr b, RectPtr r, int x, int y)
{
    RectRec	sub;
    Direction	dir;

    r->x1 = x - DominoX(w, b);
    r->y1 = y - DominoY(w, b);
    r->x2 = r->x1 + DominoWidth(w, b);
    r->y2 = r->y1 + DominoHeight(w, b);
    for (dir = North; dir <= West; dir++)
    {
	if (b->peer[dir])
	{
	    BoardSize (w, b->peer[dir], &sub,
		       x + PeerX(w, b, dir), y + PeerY(w, b, dir));
	    if (sub.x1 < r->x1)
		r->x1 = sub.x1;
	    if (sub.x2 > r->x2)
		r->x2 = sub.x2;
	    if (sub.y1 < r->y1)
		r->y1 = sub.y1;
	    if (sub.y2 > r->y2)
		r->y2 = sub.y2;
	}
    }
}

static void
DrawPip (cairo_t *cr, double x, double y, double radius)
{
    cairo_arc(cr, x, y, radius, 0, M_PI * 2);
    cairo_fill(cr);
}

static void
DrawPips (DominosWidget w, cairo_t *cr, Pips p, int flip)
{
    int	off_x, off_y;
    int	half_x, half_y;
    double radius = PIP_SIZE(w) / 2.0;

    set_source_render(cr, &w->dominos.pips_color);
    off_x = PIP_OFF(w);
    off_y = PIP_OFF(w);
    half_x = 0;
    half_y = off_y;
    if (flip)
    {
	half_x = off_x;
	half_y = 0;
	off_x = -off_x;
    }

    if (p & 1)
    {
	DrawPip (cr, 0, 0, radius);
	p = p - 1;
    }
    switch (p) {
    case 8:
	DrawPip (cr, - half_x, - half_y, radius);
	DrawPip (cr, + half_x, + half_y, radius);
	/* fall through */
    case 6:
	DrawPip (cr, - half_y, - half_x, radius);
	DrawPip (cr, + half_y, + half_x, radius);
	/* fall through */
    case 4:
	DrawPip (cr, + off_x, - off_y, radius);
	DrawPip (cr, - off_x, + off_y, radius);
	/* fall through */
    case 2:
	DrawPip (cr, - off_x, - off_y, radius);
	DrawPip (cr, + off_x, + off_y, radius);
    }
}

static void
DrawDomino(DominosWidget w, cairo_t *cr_w, cairo_surface_t *s, DominoPtr d, int x, int y)
{
    cairo_surface_t *t = cairo_surface_create_similar(s,
						      CAIRO_CONTENT_COLOR,
						      DominoWidth(w, d),
						      DominoHeight(w, d));
    cairo_t *cr = cairo_create(t);
    int	off_x, off_y;
    Pips    p;
    int	    flip;

    cairo_set_line_width(cr, LINE_WIDTH(w));
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    set_source_render(cr, &w->dominos.background);
    cairo_rectangle(cr, 0, 0, DominoWidth(w, d), DominoHeight(w, d));
    cairo_fill(cr);
    cairo_save(cr);
    {
	FillDomino (w, cr, d);
	flip = !DominoUpright(d);
	if (d->orientation == North || d->orientation == West)
	    p = d->pips[0];
	else
	    p = d->pips[1];
	off_x = DOMINO_MINOR_WIDTH(w) / 2;
	off_y = DOMINO_MINOR_HEIGHT(w) / 2;
	cairo_save(cr);
	{
	    cairo_translate(cr, off_x, off_y);
	    DrawPips (w, cr, p, flip);
	}
	cairo_restore(cr);
	if (d->orientation == North || d->orientation == West)
	    p = d->pips[1];
	else
	    p = d->pips[0];
	if (!flip)
	    off_y = DominoY(w, d) + DOMINO_MINOR_HEIGHT(w) / 2;
	else
	    off_x = DominoX(w, d) + DOMINO_MINOR_WIDTH(w) / 2;
	cairo_save(cr);
	{
	    cairo_translate(cr, off_x, off_y);
	    DrawPips (w, cr, p, flip);
	}
	cairo_restore(cr);
    }
    cairo_restore(cr);
    cairo_set_source_surface(cr_w, t, x - DominoX(w, d),  y - DominoY(w, d));
    cairo_paint(cr_w);
}

static void
DrawDominos (DominosWidget w, cairo_t *cr, cairo_surface_t *s, DominoPtr b, int x, int y)
{
    Direction	dir;

    DrawDomino (w, cr, s,b, x, y);
    for (dir = North; dir <= West; dir++)
	if (b->peer[dir])
	    DrawDominos (w, cr, s, b->peer[dir],
			 x + PeerX(w, b, dir), y + PeerY(w, b, dir));
}

static void
DrawBoard (DominosWidget w, DominoPtr b, Boolean ok_resize)
{
    RectRec	size;
    int		xoff, yoff;
    Dimension	prefered_width, prefered_height, width, height;
    XtGeometryResult	result;

    BoardSize (w, b, &size, 0, 0);
    prefered_width = size.x2 - size.x1;
    prefered_height = size.y2 - size.y1;
    if (prefered_width < DOMINO_MAJOR_WIDTH(w))
	prefered_width = DOMINO_MAJOR_WIDTH(w);
    if (prefered_height < DOMINO_MAJOR_HEIGHT(w))
	prefered_height = DOMINO_MAJOR_HEIGHT(w);
    if (ok_resize && (XtIsRealized ((Widget) w) ||
		      w->core.width < prefered_width ||
		      w->core.height < prefered_height) &&
	(prefered_width != w->core.width ||
	prefered_height != w->core.height))
    {
	result = XtMakeResizeRequest ((Widget) w,
				      prefered_width, prefered_height,
				      &width, &height);
	if (result == XtGeometryAlmost)
	    result = XtMakeResizeRequest ((Widget) w,
					  width, height, NULL, NULL);
    }
    if (XtIsRealized ((Widget) w))
    {
	cairo_surface_t	*s = cairo_xlib_surface_create(XtDisplay(w),
						       XtWindow(w),
						       XtScreen(w)->root_visual,
						       w->core.width,
						       w->core.height);
	cairo_t *cr = cairo_create(s);
	xoff = (w->core.width - prefered_width) / 2 - size.x1;
	yoff = (w->core.height- prefered_height) / 2 - size.y1;
	w->dominos.x_off = xoff;
	w->dominos.y_off = yoff;
	DrawDominos (w, cr, s, b, xoff, yoff);
	cairo_destroy (cr);
	cairo_surface_destroy (s);
    }
}

void
DominosSetDominos (Widget gw, DominoPtr *boardp)
{
    DominosWidget   w = (DominosWidget) gw;

    w->dominos.board = boardp;
    if (XtIsRealized ((Widget) w))
	XClearWindow (XtDisplay(w), XtWindow(w));
    if (w->dominos.board && *w->dominos.board)
	DrawBoard (w, *(w->dominos.board), TRUE);
}

static void
Redisplay (Widget gw, XEvent *event, Region region)
{
    DominosWidget   w = (DominosWidget) gw;

    (void) event;
    (void) region;
    if (w->dominos.board && *w->dominos.board)
	DrawBoard (w, *(w->dominos.board), FALSE);
}

static XtActionsRec actions[] = {
    { "select", ActionSelect },		    /* select card */
    { "zoomin", ZoomInAction },
    { "zoomout", ZoomOutAction },
};

DominosClassRec	dominosClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) SuperClass,
    /* class_name		*/	"Dominos",
    /* widget_size		*/	sizeof(DominosRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	XtInheritResize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	XtInheritChangeSensitive,
    /* extension                */      NULL
  },
  { /* dominos fields */
    /* ignore			*/	0
  },
};

WidgetClass dominosWidgetClass = (WidgetClass) &dominosClassRec;
