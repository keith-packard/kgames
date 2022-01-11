/*
 * Copyright © 2021 Keith Packard
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

#include <Xkw/KScrollbarP.h>

#define superclass (&ksimpleClassRec)

static double
Thickness(KScrollbarWidget w)
{
    return w->kscrollbar.thickness * w->ksimple.dpi / 72.0;
}

static double
Pad(KScrollbarWidget w)
{
    return Thickness(w) / 4;
}

static double
Room(KScrollbarWidget w)
{
    Dimension l;
    if (w->kscrollbar.orientation == XtorientHorizontal)
	l = XtWidth(w);
    else
	l = XtHeight(w);
    return l - Pad(w) * 2;
}

static double
Length(KScrollbarWidget w)
{
    double thick = Thickness(w);
    double length = w->kscrollbar.size * Room(w);
    if (length < thick)
	length = thick;
    return length;
}

static double
Avail (KScrollbarWidget w)
{
    return Room(w) - Length(w);
}

static Dimension
PreferredSize(KScrollbarWidget w)
{
    return (Dimension) (Thickness(w) + Pad(w) * 2 + 0.5);
}

static double
ThumbStart(KScrollbarWidget w)
{
    return w->kscrollbar.position * Avail(w) + Pad(w);
}

static void
ClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

static void
Initialize(Widget request, Widget cnew,
	   ArgList args, Cardinal *num_args)
{
    KScrollbarWidget w = (KScrollbarWidget) cnew;

    (void) request;
    (void) args;
    (void) num_args;
    if (XtWidth(w) == 0)
	XtWidth(w) = PreferredSize(w);
    if (XtHeight(w) == 0)
	XtWidth(w) = PreferredSize(w);
    w->kscrollbar.dragging = False;
}

static void
Redisplay(Widget gw, XEvent *event, Region region)
{
    KScrollbarWidget w = (KScrollbarWidget)gw;

    double thick = Thickness(w);
    double pos = w->kscrollbar.position * Avail(w) + Pad(w);
    cairo_t *cr = XkwDrawBegin(gw, region);

    (void) event;
    if (w->kscrollbar.orientation == XtorientHorizontal) {
	double width = Length(w);
	double x = pos;
	double y = (XtHeight(w) - thick) / 2.0;
	cairo_translate(cr, x, y);
	XkwDrawOval(cr, width, thick);
    } else {
	double height = Length(w);
	double x = (XtWidth(w) - thick) / 2.0;
	double y = pos;
	cairo_translate(cr, x, y);
	XkwDrawOval(cr, thick, height);
    }
    cairo_fill(cr);
    XkwDrawEnd(gw, region, cr);
}

static Boolean
SetValues(Widget gcur, Widget greq, Widget gnew,
	  ArgList args, Cardinal *num_args)
{
    (void) gcur;
    (void) greq;
    (void) gnew;
    (void) args;
    (void) num_args;
    if (XtIsRealized(gnew))
	Redisplay(gnew, NULL, NULL);
    return False;
}

static XtGeometryResult
QueryGeometry(Widget gw, XtWidgetGeometry *intended,
	      XtWidgetGeometry *preferred)
{
    KScrollbarWidget w = (KScrollbarWidget) gw;

    if (w->kscrollbar.orientation == XtorientHorizontal) {
	preferred->request_mode = CWHeight;
	preferred->height = PreferredSize(w);
	if (intended->request_mode & CWHeight &&
	    intended->height == preferred->height)
	    return XtGeometryYes;
	else if (preferred->height == XtHeight(w))
	    return XtGeometryNo;
    } else {
	preferred->request_mode = CWWidth;
	preferred->width = PreferredSize(w);
	if (intended->request_mode & CWWidth &&
	    intended->width == preferred->width)
	    return XtGeometryYes;
	else if (preferred->width == XtWidth(w))
	    return XtGeometryNo;
    }
    return XtGeometryAlmost;
}

static double
CoordToPosition(KScrollbarWidget w, int coord)
{
    double	length = Length(w);
    double	avail = Avail(w);
    double	c = coord - length / 2.0;

    return c/avail;
}

static int
ClassifyCoord(KScrollbarWidget w, int coord)
{
    double	thumb_start = ThumbStart(w);
    double	thumb_end = ThumbStart(w) + Length(w);

    if (coord < thumb_start)
	return -1;
    if (coord > thumb_end)
	return 1;
    return 0;
}

static Position
Coord(KScrollbarWidget w, XEvent *event)
{
    Position x = 0, y = 0;
    switch(event->type) {
    case MotionNotify:
	x = event->xmotion.x;
	y = event->xmotion.y;
	break;
    case ButtonPress:
    case ButtonRelease:
	x = event->xbutton.x;
	y = event->xbutton.y;
	break;
    case KeyPress:
    case KeyRelease:
	x = event->xkey.x;
	y = event->xkey.y;
	break;
    case EnterNotify:
    case LeaveNotify:
	x = event->xcrossing.x;
	y = event->xcrossing.y;
	break;
    }
    if (w->kscrollbar.orientation == XtorientHorizontal)
	return x;
    return y;
}

static void
Notify(KScrollbarWidget w, double position)
{
    XtCallCallbackList((Widget) w, w->kscrollbar.callbacks, (XtPointer) &position);
}

static void
Start(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KScrollbarWidget w = (KScrollbarWidget) gw;
    int coord = Coord(w, event);
    int class = ClassifyCoord(w, coord);

    (void) params;
    (void) num_params;
    w->kscrollbar.dragging = False;
    if (class < 0)
	Notify(w, XkwScrollbarPageUp);
    else if (class > 0)
	Notify(w, XkwScrollbarPageDown);
    else {
	w->kscrollbar.dragging = True;
	w->kscrollbar.start_pos = CoordToPosition(w, coord) - w->kscrollbar.position;
    }
}

static void
Drag(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KScrollbarWidget w = (KScrollbarWidget) gw;

    (void) params;
    (void) num_params;
    if (w->kscrollbar.dragging) {
	double coord = CoordToPosition(w, Coord(w, event)) - w->kscrollbar.start_pos;
	if (coord < 0)
	    coord = 0;
	if (coord > 1)
	    coord = 1;
	Notify(w, coord);
    }
}

static void
Stop(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KScrollbarWidget w = (KScrollbarWidget) gw;

    (void) event;
    (void) params;
    (void) num_params;
    w->kscrollbar.dragging = False;
}

void
XkwScrollbarSetThumb(Widget gw, double position, double size)
{
    Arg	args[2];
    Cardinal n;

    n = 0;
    /* Don't update from invalid data */
    if (0 <= position && position <= 1) {
	XkwSetArg(args[n], XtNposition, &position); n++;
    }
    if (0 <= size && size <= 1) {
	XkwSetArg(args[n], XtNsize, &size); n++;
    }
    XtSetValues(gw, args, n);
}

static XtResource resources[] = {
#define offset(field) XtOffsetOf(KScrollbarRec, kscrollbar.field)
    { XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
      offset(orientation), XtRImmediate, (XtPointer)XtorientVertical },
    { XtNthickness, XtCThickness, XtRDouble, sizeof(double),
      offset(thickness), XtRString, "9" },
    { XtNsize, XtCSize, XtRDouble, sizeof(double),
      offset(size), XtRString, (XtPointer) "1" },
    { XtNposition, XtCPosition, XtRDouble, sizeof(double),
      offset(position), XtRString, "0" },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      offset(callbacks), XtRCallback, NULL },
#undef offset
};

static XtActionsRec actions[] = {
    {"start",	Start},
    {"drag",	Drag},
    {"stop",	Stop},
};

static char defaultTranslations[] =
    "<BtnDown>:"	"start()\n"
    "<BtnMotion>:"	"drag()\n"
    "<BtnUp>:"		"stop()\n";

KScrollbarClassRec kscrollbarClassRec = {
  /* core */
  {
    (WidgetClass)superclass,		/* superclass */
    "Scrollbar",			/* class_name */
    sizeof(KScrollbarRec),		/* widget_size */
    ClassInitialize,			/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    Initialize,				/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    actions,			       	/* actions */
    XtNumber(actions),			/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    NULL,				/* destroy */
    XtInheritResize,			/* resize */
    Redisplay,				/* expose */
    SetValues,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    defaultTranslations,		/* tm_table */
    QueryGeometry,			/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
#ifndef OLDXAW
    NULL,                               /* extension */
#endif
  },
  {
    /* ksimple fields */
      0					/* empty */
  },
  /* kscrollbar */
  {
    0,					/* empty */
  }
};

WidgetClass kscrollbarWidgetClass = (WidgetClass)&kscrollbarClassRec;
