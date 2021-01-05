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

#include <Xkw/Xkw.h>
#include <X11/IntrinsicI.h>

typedef struct {
    Position x, y;
} XkwPosition;

static void
XkwTranslateOffset(Widget to, Widget from, XkwPosition *p)
{
    Position	to_x, to_y;
    Position	from_x, from_y;

    XtTranslateCoords(to, 0, 0, &to_x, &to_y);
    XtTranslateCoords(from, 0, 0, &from_x, &from_y);
    p->x = from_x - to_x;
    p->y = from_y - to_y;
}

void
XkwTranslateCoordsPosition(Widget to, Widget from, Position *x, Position *y)
{
    XkwPosition p;
    XkwTranslateOffset(to, from, &p);
    *x += p.x;
    *y += p.y;
}

void
XkwTranslateCoordsInt(Widget to, Widget from, int *x, int *y)
{
    XkwPosition p;
    XkwTranslateOffset(to, from, &p);
    *x += p.x;
    *y += p.y;
}

void
XkwGetEventCoordPointers(XEvent *e, XkwEventCoordPointers *cp)
{
    *cp = (XkwEventCoordPointers) { };
    switch(e->type) {
    case MotionNotify:
	cp->x = &e->xmotion.x;
	cp->y = &e->xmotion.y;
	cp->x_root = &e->xmotion.x_root;
	cp->y_root = &e->xmotion.y_root;
	cp->window = &e->xmotion.window;
	break;
    case ButtonPress:
    case ButtonRelease:
	cp->x = &e->xbutton.x;
	cp->y = &e->xbutton.y;
	cp->x_root = &e->xbutton.x_root;
	cp->y_root = &e->xbutton.y_root;
	cp->window = &e->xbutton.window;
	break;
    case KeyPress:
    case KeyRelease:
	cp->x = &e->xkey.x;
	cp->y = &e->xkey.y;
	cp->x_root = &e->xkey.x_root;
	cp->y_root = &e->xkey.y_root;
	cp->window = &e->xkey.window;
	break;
    case EnterNotify:
    case LeaveNotify:
	cp->x = &e->xcrossing.x;
	cp->y = &e->xcrossing.y;
	cp->x_root = &e->xcrossing.x_root;
	cp->y_root = &e->xcrossing.y_root;
	cp->window = &e->xcrossing.window;
	break;
    case FocusIn:
    case FocusOut:
	cp->window = &e->xfocus.window;
	break;
    case Expose:
	cp->x = &e->xexpose.x;
	cp->y = &e->xexpose.y;
	cp->window = &e->xexpose.window;
	break;
    }
}

void
XkwTranslateEvent(Widget to, Widget from, XEvent *e)
{
    XkwEventCoordPointers c;
    XkwGetEventCoordPointers(e, &c);
    if (c.x) {
	XkwPosition p;
	XkwTranslateOffset(to, from, &p);
	*c.x += p.x;
	*c.y += p.y;
	if (c.x_root) {
	    *c.x_root += p.x;
	    *c.y_root += p.y;
	}
    }
    if (c.window)
	*c.window = XtWindow(to);
}

Bool
XkwGetEventCoords(XEvent *e, Position *x, Position *y)
{
    XkwEventCoordPointers c;
    XkwGetEventCoordPointers(e, &c);
    if (c.x) {
	*x = *c.x;
	*y = *c.y;
	return TRUE;
    }
    return FALSE;
}

#define ForAllChildren(pw, childP) \
  for ( (childP) = (pw)->composite.children ; \
        (childP) < (pw)->composite.children + (pw)->composite.num_children ; \
        (childP)++ ) if (!XtIsManaged(*childP)) ; else

Widget
XkwXYToWidget(Widget w, Position x, Position y)
{
    Dimension width = XtWidth(w), height = XtHeight(w);

    if (x < 0 || width <= x || y < 0 || height <= y)
	return NULL;

    if (XtIsComposite(w)) {
	CompositeWidget cw = (CompositeWidget) w;
	Widget *children;
	Widget child;

	ForAllChildren(cw, children) {
	    child = *children;
	    w = XkwXYToWidget(child, x - XtX(child), y - XtY(child));
	    if (w)
		break;
	}
    }
    return w;
}

Bool
XkwForwardEvent(Widget to, Widget from, XEvent *e)
{
    XEvent copy = *e;

    if (!to) {
	Widget shell = from;
	while (!XtIsShell(shell) && XtParent(shell))
	    shell = XtParent(shell);
	XkwTranslateEvent(shell, from, &copy);

	Position x, y;
	if (XkwGetEventCoords(&copy, &x, &y))
	    to = XkwXYToWidget(shell, x, y);

	if (to == from)
	    return FALSE;

	from = shell;
    }
    if (to == from || !to)
	return FALSE;

    XkwTranslateEvent(to, from, &copy);
    XtDispatchEvent(&copy);
    return TRUE;
}
