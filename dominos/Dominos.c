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

#define offset(field)	XtOffsetOf(DominosRec, dominos.field)

#define COLOR_UNSET 10

static XtResource resources[] = {
    {XtNfaceColor, XtCFaceColor, XtRRenderColor, sizeof (XRenderColor),
     offset(face_color), XtRString, (XtPointer) "black"},
    {XtNpipsColor, XtCPipsColor, XtRRenderColor, sizeof (XRenderColor),
     offset(pips_color), XtRString, (XtPointer) "white"},
    {XtNsize, XtCSize, XtRDimension, sizeof (Dimension),
     offset(size), XtRImmediate, (XtPointer) 60},
    {XtNinputCallback, XtCInputCallback, XtRCallback, sizeof (XtPointer),
     offset(input_callback), XtRCallback, (XtPointer) NULL},
};

#undef offset
#undef hoffset

#include	<X11/Xmu/Drawing.h>

static char defaultTranslations[] =
    "<Btn4Down>: zoomout()\n"
    "<Btn5Down>: zoomin()\n"
    "<Btn4Up>:\n"
    "<Btn5Up>:\n"
    "<BtnDown>: start()\n"
    "<BtnMotion>: drag(drag)\n"
    "<BtnUp>: stop(dest)";

#define SuperClass  ((KSimpleWidgetClass)&ksimpleClassRec)

#define INSET(w)	((w)->dominos.size / 20.0)
#define RADIUS(w)	((w)->dominos.size / 10.0)

#define LINE_WIDTH(w)	    ((w)->dominos.size / 25.0)
#define PIP_SIZE(w)	    ((w)->dominos.size / 15.0)
#define PIP_OFF(w)    	    ((w)->dominos.size / 5.0)

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

static DominoPtr
Dominos(DominosWidget w)
{
    if (w->dominos.board)
	return *w->dominos.board;
    return NULL;
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
	new->core.width = DOMINO_MINOR_WIDTH(new);
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
Destroy (Widget gw)
{
    (void) gw;
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

static void
BoardSize (DominosWidget w, DominoPtr b, RectPtr r, int x, int y)
{
    RectRec	sub;
    Direction	dir;

    if (!b) {
	r->x1 = x;
	r->y1 = y;
	r->x2 = x;
	r->y2 = y;
	return;
    }
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
PreferredSize (DominosWidget w, Dimension *width, Dimension *height, Position *x, Position *y)
{
    RectRec		size;
    Dimension		preferred_width, preferred_height;

    BoardSize (w, Dominos(w), &size, 0, 0);
    preferred_width = size.x2 - size.x1;
    preferred_height = size.y2 - size.y1;
    if (preferred_width < DOMINO_MINOR_WIDTH(w))
	preferred_width = DOMINO_MINOR_WIDTH(w);
    if (preferred_height < DOMINO_MAJOR_HEIGHT(w))
	preferred_height = DOMINO_MAJOR_HEIGHT(w);
    *width = preferred_width;
    *height = preferred_height;
    if (x)
	*x = size.x1;
    if (y)
	*y = size.y1;
}

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    DominosWidget cur = (DominosWidget) gcur;
    DominosWidget new = (DominosWidget) gnew;
    Boolean redisplay = False;

    (void) greq;
    (void) args;
    (void) count;
    if (!XkwColorEqual(&cur->dominos.face_color, &new->dominos.face_color))
	redisplay = True;
    if (!XkwColorEqual(&cur->dominos.pips_color, &new->dominos.pips_color))
	redisplay = True;
    if (XtWidth(gcur) != XtWidth(greq) || XtHeight(gcur) != XtHeight(greq))
	redisplay = True;
    if (cur->dominos.size != new->dominos.size)
    {
	Dimension		preferred_width, preferred_height, width, height;
	XtGeometryResult	result;

	PreferredSize(new, &preferred_width, &preferred_height, NULL, NULL);
	if (preferred_width != new->core.width || preferred_height != new->core.height)
	{
	    result = XtMakeResizeRequest ((Widget) new,
					  preferred_width, preferred_height,
					  &width, &height);
	    if (result == XtGeometryAlmost)
		result = XtMakeResizeRequest ((Widget) new,
					      width, height, NULL, NULL);
	}
	redisplay = True;
    }
    return redisplay;
}

static int
i_sqrt (int a)
{
    double	    d_a;

    d_a = (double) a;
    d_a = sqrt (d_a);
    return (int) d_a;
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

    if (!b)
	return NULL;
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
		if (peer && sub_dist < to_dist)
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
    return XYInDomino (w, Dominos(w),
		       w->dominos.x_off, w->dominos.y_off, x, y,
		       distp, dirp);
}

DominoPtr
DominosXYToDomino (Widget gw,
		   int x,
		   int y,
		   int *distp,
		   Direction *dirp)
{
    DominosWidget w = (DominosWidget) gw;
    DominoPtr domino = XYToDomino(w, x, y, distp, dirp);
    if (domino && *distp > DOMINO_MAJOR_SIZE(w))
	domino = NULL;
    return domino;
}

static void
DoInputCallback(Widget gw, DominosAction action,
		XEvent *event, String *params, Cardinal *num_params)
{
    DominosWidget   w = (DominosWidget) gw;
    DominoPtr	    d;
    DominosInputRec input;
    Direction	    dir;
    int		    dist;

    d = XYToDomino (w, event->xbutton.x, event->xbutton.y, &dist, &dir);
    input.w = gw;
    input.action = action;
    input.domino = d;
    input.direction = dir;
    input.distance = dist;
    input.params = params;
    input.event = *event;
    input.num_params = num_params;
    XtCallCallbackList (gw, w->dominos.input_callback, (XtPointer) &input);
}

static void
ActionStart (Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    DoInputCallback(gw, DominosActionStart, event, params, num_params);
}

static void
ActionDrag (Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    DoInputCallback(gw, DominosActionDrag, event, params, num_params);
}

static void
ActionStop (Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    if (XkwForwardEvent(NULL, gw, event))
	return;
    DoInputCallback(gw, DominosActionStop, event, params, num_params);
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
OutlineDomino (DominosWidget w, cairo_t *cr, DominoPtr d, double width, double height)
{
    XkwSetSource (cr, &w->dominos.pips_color);
    cairo_set_line_width(cr, LINE_WIDTH(w));
    if (DominoUpright (d)) {
	cairo_move_to(cr, 0, height/2.0);
	cairo_line_to(cr, width, height/2.0);
    } else {
	cairo_move_to(cr, width / 2.0, 0);
	cairo_line_to(cr, width / 2.0, height);
    }
    XkwDrawRoundedRect(cr, width, height, RADIUS(w));
    cairo_stroke(cr);
}

static void
FillDomino (DominosWidget w, cairo_t *cr, DominoPtr d)
{
    double	width, height;

    width = DominoWidth(w, d) - INSET(w) * 2;
    height = DominoHeight(w, d) - INSET(w) * 2;
    XkwSetSource (cr, &w->dominos.face_color);
    cairo_save(cr);
    cairo_translate(cr, INSET(w), INSET(w));
    XkwDrawRoundedRect(cr, width, height, RADIUS(w));
    cairo_fill(cr);
    OutlineDomino (w, cr, d, width, height);
    cairo_restore(cr);
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
    double radius = PIP_SIZE(w);

    XkwSetSource(cr, &w->dominos.pips_color);
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
DrawDomino(DominosWidget w, cairo_t *cr, DominoPtr d)
{
    int	off_x, off_y;
    Pips    p;
    int	    flip;

    if (d->hide)
	return;

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

static void
DrawDominos (DominosWidget w, cairo_t *cr, DominoPtr d, int x, int y)
{
    Direction	dir;

    cairo_save(cr);
    cairo_translate(cr, x - DominoX(w, d), y - DominoY(w, d));
    DrawDomino (w, cr, d);
    cairo_restore(cr);
    for (dir = North; dir <= West; dir++)
	if (d->peer[dir])
	    DrawDominos (w, cr, d->peer[dir],
			 x + PeerX(w, d, dir), y + PeerY(w, d, dir));
}

static void
DrawBoard (DominosWidget w, Boolean ok_resize, Region region)
{
    int		xoff, yoff;
    Position	x, y;
    Dimension	preferred_width, preferred_height, width, height;
    XtGeometryResult	result;

    PreferredSize(w, &preferred_width, &preferred_height, &x, &y);
    if (ok_resize && (XtIsRealized ((Widget) w) ||
		      w->core.width < preferred_width ||
		      w->core.height < preferred_height) &&
	(preferred_width != w->core.width ||
	preferred_height != w->core.height))
    {
	result = XtMakeResizeRequest ((Widget) w,
				      preferred_width, preferred_height,
				      &width, &height);
	if (result == XtGeometryAlmost && (width != w->core.width || height != w->core.height))
	    result = XtMakeResizeRequest ((Widget) w,
					  width, height, NULL, NULL);
    }
    if (XtIsRealized ((Widget) w))
    {
	DominoPtr b = Dominos(w);
	cairo_t *cr = XkwDrawBegin((Widget) w, region);
	xoff = (w->core.width - preferred_width) / 2 - x;
	yoff = (w->core.height- preferred_height) / 2 - y;
	w->dominos.x_off = xoff;
	w->dominos.y_off = yoff;
	if (b)
	    DrawDominos (w, cr, b, xoff, yoff);
	XkwDrawEnd((Widget) w, region, cr);
    }
}

void
DominosSetDominos (Widget gw, DominoPtr *boardp)
{
    DominosWidget   w = (DominosWidget) gw;

    w->dominos.board = boardp;
    if (XtIsRealized ((Widget) w))
	DrawBoard (w, TRUE, NULL);
}

static void
Redisplay (Widget gw, XEvent *event, Region region)
{
    DominosWidget   w = (DominosWidget) gw;

    (void) event;
    DrawBoard (w, FALSE, region);
}

static void
Resize (Widget gw)
{
    if (XtIsRealized(gw))
	XClearArea(XtDisplay(gw), XtWindow(gw), 0, 0, 0, 0, True);
}

static XtGeometryResult
QueryGeometry(Widget gw, XtWidgetGeometry *intended,
	      XtWidgetGeometry *preferred)
{
    DominosWidget   w = (DominosWidget) gw;

    preferred->request_mode = intended->request_mode & (CWWidth|CWHeight);
    PreferredSize(w, &preferred->width, &preferred->height, NULL, NULL);
    if (((intended->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	&& intended->width == preferred->width
	&& intended->height == preferred->height)
	return (XtGeometryYes);
      else if (preferred->width == XtWidth(w) && preferred->height == XtHeight(w))
	return (XtGeometryNo);

    return (XtGeometryAlmost);
}

static XtActionsRec actions[] = {
    { "start", ActionStart },		    /* select card */
    { "drag", ActionDrag },
    { "stop", ActionStop },
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
    /* realize			*/	XtInheritRealize,
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
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultTranslations,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	XtInheritChangeSensitive,
    /* extension                */      NULL
  },
  { /* ksimple fields */
       /* unused */			0,
  },
  { /* dominos fields */
    /* ignore			*/	0
  },
};

WidgetClass dominosWidgetClass = (WidgetClass) &dominosClassRec;
