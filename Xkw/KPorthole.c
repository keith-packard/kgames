/*
 *
Copyright 1990, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Jim Fulton, MIT X Consortium
 *
 * This widget is a trivial clipping widget.  It is typically used with a
 * panner or scrollbar to navigate.
 */

#include <Xkw/KPortholeP.h>
#include <X11/Xmu/Misc.h>

/*
 * Implementation
 */
static Widget
find_child(KPortholeWidget pw)
{
    Widget *children;
    unsigned int i;

    /*
     * Find the managed child on which we should operate.  Ignore multiple
     * managed children
     */
    for (i = 0, children = pw->composite.children;
	 i < pw->composite.num_children; i++, children++)
	if (XtIsManaged(*children))
	    return (*children);

    return (NULL);
}

static void
SendReport(KPortholeWidget pw)
{
    Widget child = find_child(pw);

    if (pw->kporthole.callbacks && child)
	XtCallCallbackList((Widget)pw, pw->kporthole.callbacks, child);
}

static void
LayoutChild(KPortholeWidget pw, Widget child, XtWidgetGeometry *geomp,
	     Position *xp, Position *yp, Dimension *widthp, Dimension *heightp)
{
    Position minx, miny;
    XtWidgetGeometry intended = {
	.request_mode = CWWidth | CWHeight,
	.width = XtWidth(pw),
	.height = XtHeight(pw)
    };
    XtWidgetGeometry preferred = intended;

    (void) XtQueryGeometry(child, &intended, &preferred);

    *xp = XtX(child);			/* default to current pos, preferred size */
    *yp = XtY(child);
    *widthp = preferred.width;
    *heightp = preferred.height;

    if (geomp) {			/* mix in any requested changes */
	if (geomp->request_mode & CWX)
	    *xp = geomp->x;
	if (geomp->request_mode & CWY)
	    *yp = geomp->y;
	if (geomp->request_mode & CWWidth)
	    *widthp = geomp->width;
	if (geomp->request_mode & CWHeight)
	    *heightp = geomp->height;
    }

    /*
     * Make sure that the child is at least as large as the porthole; there
     * is no maximum size
     */
    if (*widthp < XtWidth(pw)) *widthp = XtWidth(pw);
    if (*heightp < XtHeight(pw)) *heightp = XtHeight(pw);

    /*
     * Make sure that the child is still on the screen.  Note that this must
     * be done *after* the size computation so that we know where to put it
     */
    minx = (Position)XtWidth(pw) - (Position)*widthp;
    miny = (Position)XtHeight(pw) - (Position)*heightp;

    if (*xp < minx)
	*xp = minx;
    if (*yp < miny)
	*yp = miny;

    if (*xp > 0)
	*xp = 0;
    if (*yp > 0)
	*yp = 0;
}

static void KPortholeStart(Widget, XEvent*, String*, Cardinal*);

static void
KPortholeClassInitialize(void)
{
    XkwInitializeWidgetSet();
    XtRegisterGrabAction(KPortholeStart,
			 False,
			 PointerMotionMask|ButtonReleaseMask|ButtonPressMask,
			 GrabModeAsync, GrabModeAsync);
}

static void
KPortholeRealize(Widget gw, Mask *valueMask, XSetWindowAttributes *attr)
{
    attr->bit_gravity = NorthWestGravity;
    *valueMask |= CWBitGravity;

    if (XtWidth(gw) < 1)
	XtWidth(gw) = 1;
    if (XtHeight(gw) < 1)
	XtHeight(gw) = 1;
    (*kportholeWidgetClass->core_class.superclass->core_class.realize)
	(gw, valueMask, attr);
}

static void
KPortholeResize(Widget gw)
{
    KPortholeWidget pw = (KPortholeWidget)gw;
    Widget child = find_child(pw);

    /*
     * If we have a child, we need to make sure that it is at least as big
     * as we are and in the right place
     */
    if (child) {
	Position x, y;
	Dimension width, height;

	LayoutChild(pw, child, NULL, &x, &y, &width, &height);
	XtConfigureWidget(child, x, y, width, height, 0);
    }

    SendReport(pw);
}

static XtGeometryResult
KPortholeQueryGeometry(Widget gw, XtWidgetGeometry *intended,
			 XtWidgetGeometry *preferred)
{
    KPortholeWidget pw = (KPortholeWidget)gw;
    Widget child = find_child(pw);

    if (child) {
#define SIZEONLY (CWWidth | CWHeight)
	preferred->request_mode = intended->request_mode & SIZEONLY;
	preferred->width = XtWidth(child);
	preferred->height = XtHeight(child);

	if (((intended->request_mode & CWWidth) == 0 ||
	     intended->width == preferred->width) &&
	    ((intended->request_mode & CWHeight) == 0 ||
	     intended->height == preferred->height))
	    return (XtGeometryYes);
	else if (preferred->width == XtWidth(pw) &&
		 preferred->height == XtHeight(pw))
	    return (XtGeometryNo);

	return (XtGeometryAlmost);
#undef SIZEONLY
    }

    return (XtGeometryNo);
}

static XtGeometryResult
KPortholeGeometryManager(Widget w, XtWidgetGeometry *req,
			   XtWidgetGeometry *reply)
{
    KPortholeWidget pw = (KPortholeWidget) w->core.parent;
    Widget child = find_child(pw);
    Bool okay = True;

    if (child != w)
	return (XtGeometryNo);

    *reply = *req;			/* assume we'll grant everything */

    if ((req->request_mode & CWBorderWidth) && req->border_width != 0) {
	reply->border_width = 0;
	okay = False;
    }

    LayoutChild(pw, child, req, &reply->x, &reply->y,
		&reply->width, &reply->height);

    if ((req->request_mode & CWX) && req->x != reply->x)
	okay = False;
    if ((req->request_mode & CWY) && req->x != reply->x)
	okay = False;
    if ((req->request_mode & CWWidth) && req->width != reply->width)
	okay = False;
    if ((req->request_mode & CWHeight) && req->height != reply->height)
	okay = False;

    /*
     * If we failed on anything, simply return without touching widget
     */
    if (!okay)
	return (XtGeometryAlmost);

    /*
     * If not just doing a query, update widget and send report.  Note that
     * we will often set fields that weren't requested because we want to keep
     * the child visible
     */
    if (!(req->request_mode & XtCWQueryOnly)) {

	XtX(child) = reply->x;
	XtY(child) = reply->y;
	XtWidth(child) = reply->width;
	XtHeight(child) = reply->height;
	SendReport(pw);
    }

    return (XtGeometryYes);		/* success! */
}

static void
KPortholeChangeManaged(Widget gw)
{
    KPortholeWidget pw = (KPortholeWidget)gw;
    Widget child = find_child (pw);	/* ignore extra children */

    if (child) {
	if (!XtIsRealized (gw)) {
	    XtWidgetGeometry geom, retgeom;

	    geom.request_mode = 0;
	    if (XtWidth(pw) == 0) {
		geom.width = XtWidth(child);
		geom.request_mode |= CWWidth;
	    }
	    if (XtHeight(pw) == 0) {
		geom.height = XtHeight(child);
		geom.request_mode |= CWHeight;
	    }
	    if (geom.request_mode &&
		XtMakeGeometryRequest (gw, &geom, &retgeom)
		== XtGeometryAlmost)
		(void)XtMakeGeometryRequest(gw, &retgeom, NULL);
	}

	XtResizeWidget(child, Max(XtWidth(child), XtWidth(pw)),
		       Max(XtHeight(child), XtHeight(pw)), 0);

	SendReport(pw);
    }
}

static Boolean
GetPosition(XEvent *event, Position *x, Position *y)
{
    switch(event->type) {
    case MotionNotify:
	*x = event->xmotion.x;
	*y = event->xmotion.y;
	break;
    case ButtonPress:
    case ButtonRelease:
	*x = event->xbutton.x;
	*y = event->xbutton.y;
	break;
    case KeyPress:
    case KeyRelease:
	*x = event->xkey.x;
	*y = event->xkey.y;
	break;
    case EnterNotify:
    case LeaveNotify:
	*x = event->xcrossing.x;
	*y = event->xcrossing.y;
	break;
    default:
	*x = 0;
	*y = 0;
	return False;
    }
    return True;
}

static void KPortholeStart(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KPortholeWidget	w = (KPortholeWidget) gw;
    Widget child = find_child(w);

    w->kporthole.dragging = GetPosition(event, &w->kporthole.start_x, &w->kporthole.start_y);
    if (child) {
	w->kporthole.child_start_x = XtX(child);
	w->kporthole.child_start_y = XtY(child);
    }
}

static void KPortholeDrag(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KPortholeWidget	w = (KPortholeWidget) gw;
    Position		x, y;

    if (w->kporthole.dragging && GetPosition(event, &x, &y)) {
	Widget child = find_child(w);

	if (child) {
	    Position dx = x - w->kporthole.start_x;
	    Position dy = y - w->kporthole.start_y;
	    Position child_x = w->kporthole.child_start_x + dx;
	    Position child_y = w->kporthole.child_start_y + dy;
	    Position min_x = XtWidth(gw) - XtWidth(child);
	    Position min_y = XtHeight(gw) - XtHeight(child);

	    if (child_x > 0)
		child_x = 0;
	    if (child_y > 0)
		child_y = 0;

	    if (child_x < min_x)
		child_x = min_x;
	    if (child_y < min_y)
		child_y = min_y;

	    if (child_x != XtX(child) || child_y != XtY(child)) {
		Arg	args[2];
		XtSetArg(args[0], XtNx, child_x);
		XtSetArg(args[1], XtNy, child_y);
		XtSetValues(child, args, 2);
	    }
	}
    }
}

static void KPortholeStop(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KPortholeWidget	w = (KPortholeWidget) gw;

    w->kporthole.dragging = False;
}

static const char defaultTranslations[] =
    "<Btn2Down>:"	"start()\n"
    "<Btn2Motion>:"	"drag()\n"
    "<Btn2Up>:"		"stop()\n"
    "Shift <Btn1Down>:"	"start()\n"
    "Shift <Btn1Motion>: drag()\n"
    "Shift <Btn1Up>:"	"stop()\n";

static XtActionsRec actions[] = {
    { "start",	KPortholeStart, },
    { "drag",	KPortholeDrag, },
    { "stop",	KPortholeStop, },
};

#define offset(field)	XtOffsetOf(KPortholeRec, kporthole.field)
static XtResource resources[] = {
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      offset(callbacks), XtRCallback, NULL },
};
#undef offset

#define Superclass	(&compositeClassRec)
KPortholeClassRec kportholeClassRec = {
  /* core */
  {
    (WidgetClass)Superclass,		/* superclass */
    "Porthole",				/* class_name */
    sizeof(KPortholeRec),		/* widget_size */
    KPortholeClassInitialize,		/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    NULL,				/* initialize */
    NULL,				/* initialize_hook */
    KPortholeRealize,			/* realize */
    actions,				/* actions */
    XtNumber(actions),			/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    NULL,				/* destroy */
    KPortholeResize,			/* resize */
    NULL,				/* expose */
    NULL,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    (char *)defaultTranslations,	/* tm_table */
    KPortholeQueryGeometry,		/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* composite */
  {
    KPortholeGeometryManager,		/* geometry_manager */
    KPortholeChangeManaged,		/* change_managed */
    XtInheritInsertChild,		/* insert_child */
    XtInheritDeleteChild,		/* delete_child */
    NULL,				/* extension */
  },
  { /* porthole */
    NULL,				/* extension */
  },
};

WidgetClass kportholeWidgetClass = (WidgetClass)&kportholeClassRec;
