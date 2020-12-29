/*
Copyright 1989, 1998  The Open Group

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
 * Author:  Chris D. Peterson, MIT X Consortium
 */

/*
 * Sme.c - Source code for the generic menu entry
 *
 * Date:    September 26, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium
 *          kit@expo.lcs.mit.edu
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>
#include <Xkw/KSmeLineP.h>
#include <Xkw/Xkw.h>

/*
 * Class Methods
 */
static void XkwKSmeLineDestroy(Widget);
static void XkwKSmeLineInitialize(Widget, Widget, ArgList, Cardinal*);
static void XkwKSmeLineRedisplay(Widget, XEvent*, Region);
static Boolean XkwKSmeLineSetValues(Widget, Widget, Widget,
				   ArgList, Cardinal*);

/*
 * Initialization
 */
#define offset(field)	XtOffsetOf(KSmeLineRec, ksme_line.field)
static XtResource resources[] = {
    { XtNlineWidth, XtCLineWidth, XtRDimension, sizeof(Dimension),
      offset(line_width), XtRImmediate, (XtPointer)1 },
    { XtNbackgroundColor, XtCBackground, XtRRenderColor, sizeof (XRenderColor),
      offset (background), XtRString, XtDefaultBackground },
    { XtNforegroundColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (foreground), XtRString, XtDefaultForeground },
};
#undef offset

#define Superclass	(&ksmeClassRec)
KSmeLineClassRec ksmeLineClassRec = {
  /* rectangle */
  {
    (WidgetClass)Superclass,		/* superclass */
    "KSmeLine",				/* class_name */
    sizeof(KSmeLineRec),			/* widget_size */
    XkwInitializeWidgetSet,		/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class inited */
    XkwKSmeLineInitialize,		/* initialize */
    NULL,				/* initialize_hook */
    NULL,				/* realize */
    NULL,				/* actions */
    0,					/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    False,				/* compress_motion */
    False,				/* compress_exposure */
    False,				/* compress_enterleave */
    False,				/* visible_interest */
    XkwKSmeLineDestroy,			/* destroy */
    NULL,				/* resize */
    XkwKSmeLineRedisplay,		/* expose */
    XkwKSmeLineSetValues,		/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* intrinsics version */
    NULL,				/* callback offsets */
    NULL,				/* tm_table */
    XtInheritQueryGeometry,		/* query_geometry */
    NULL,				/* display_accelerator */
    NULL,				/* extension */
  },
  /* ksme */
  {
    XtInheritHighlight,			/* highlight */
    XtInheritUnhighlight,		/* unhighlight */
    XtInheritNotify,			/* notify */
    NULL,				/* extension */
  },
  /* ksme_line */
  {
    NULL,				/* extension */
  }
};

WidgetClass ksmeLineObjectClass = (WidgetClass)&ksmeLineClassRec;

/*
 * Implementation
 */
/*ARGSUSED*/
static void
XkwKSmeLineInitialize(Widget request, Widget cnew,
		     ArgList args, Cardinal *num_args)
{
    KSmeLineObject entry = (KSmeLineObject)cnew;

    if (XtHeight(entry) == 0)
	XtHeight(entry) = entry->ksme_line.line_width;
}

static void
XkwKSmeLineDestroy(Widget w)
{
}

/*ARGSUSED*/
static void
XkwKSmeLineRedisplay(Widget w, XEvent *event, Region region)
{
    KSmeLineObject entry = (KSmeLineObject)w;
    int y = XtY(w) + (((int)XtHeight(w) - entry->ksme_line.line_width) >> 1);

    cairo_t *cr = XkwGetCairo(w);
    XkwSetSource(cr, &entry->ksme_line.foreground);

    cairo_rectangle(cr, XtX(w), y,
		    XtWidth(w), entry->ksme_line.line_width);
    cairo_fill(cr);
    cairo_destroy(cr);
}

/*
 * Function:
 *	XkwKSmeLineSetValues
 *
 * Parameters:
 *	current - current state of the widget
 *	request - what was requested
 *	cnew	- what the widget will become
 *
 * Description:
 *	Relayout the menu when one of the resources is changed.
 */
/*ARGSUSED*/
static Boolean
XkwKSmeLineSetValues(Widget current, Widget request, Widget cnew,
		    ArgList args, Cardinal *num_args)
{
    KSmeLineObject entry = (KSmeLineObject)cnew;
    KSmeLineObject old_entry = (KSmeLineObject)current;

    if (entry->ksme_line.line_width != old_entry->ksme_line.line_width) {
	return (True);
    }

    return (False);
}
