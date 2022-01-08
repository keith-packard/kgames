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

#include <Xkw/KSimpleP.h>

#define SuperClass	(&simpleClassRec)

static void
KSimpleClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

static void
KSimpleInitialize(Widget request, Widget cnew,
		  ArgList args, Cardinal *num_args)
{
    KSimpleWidget w = (KSimpleWidget) cnew;

    w->ksimple.surface = NULL;
    w->ksimple.surface_width = 0;
    w->ksimple.surface_height = 0;
}

static void
KSimpleRealize(Widget w, Mask *valueMask, XSetWindowAttributes *attributes)
{
    *valueMask &= ~(CWBackPixel);
    *valueMask |= (CWBackPixmap);
    attributes->background_pixmap = None;
    (*ksimpleWidgetClass->core_class.superclass->core_class.realize)
	(w, valueMask, attributes);
}

static void
KSimpleDestroy(Widget gw)
{
    KSimpleWidget w = (KSimpleWidget) gw;

    if (w->ksimple.surface) {
	cairo_surface_destroy(w->ksimple.surface);
	w->ksimple.surface = NULL;
    }
}

static void
KSimpleRedisplay(Widget gw, XEvent *event, Region region)
{
    KSimpleWidget w = (KSimpleWidget) gw;
    cairo_t *cr = XkwGetCairo(gw);

    XkwSetSource(cr, &w->ksimple.background);
    cairo_paint(cr);
    cairo_destroy(cr);
}

static Boolean
KSimpleSetValues(Widget gcur, Widget greq, Widget gnew,
		 ArgList args, Cardinal *num_args)
{
    KSimpleWidget cur = (KSimpleWidget)gcur;
    KSimpleWidget req = (KSimpleWidget)greq;
    KSimpleWidget new = (KSimpleWidget)gnew;

    (void) req;
    if (!XkwColorEqual(&cur->ksimple.foreground, &new->ksimple.foreground) ||
	!XkwColorEqual(&cur->ksimple.background, &new->ksimple.background))
	return True;
    return False;
}

#define offset(field) XtOffsetOf(KSimpleRec, ksimple.field)
static XtResource resources[] = {
    { XtNbackgroundColor, XtCBackground, XtRRenderColor, sizeof (XRenderColor),
      offset (background), XtRString, XtDefaultBackground },
    { XtNforegroundColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (foreground), XtRString, XtDefaultForeground },
    { XtNdpi, XtCDpi, XtRDpi, sizeof(double),
      offset (dpi), XtRString, "" },
    {XtNwantForward, XtCWantForward, XtRBoolean, sizeof(Boolean),
     offset(want_forward), XtRImmediate, (XtPointer) True},
};
#undef offset

KSimpleClassRec ksimpleClassRec = {
  /* core */
  {
    (WidgetClass)SuperClass,		/* superclass */
    "KSimple",				/* class_name */
    sizeof(KSimpleRec),			/* widget_size */
    KSimpleClassInitialize,		/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    KSimpleInitialize,			/* initialize */
    NULL,				/* initialize_hook */
    KSimpleRealize,			/* realize */
    NULL,				/* actions */
    0,					/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    KSimpleDestroy,			/* destroy */
    NULL,				/* resize */
    KSimpleRedisplay,			/* expose */
    KSimpleSetValues,			/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    NULL,				/* tm_table */
    XtInheritQueryGeometry,		/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
  },
  /* ksimple */
  {
    0,
  },
};

WidgetClass ksimpleWidgetClass = (WidgetClass)&ksimpleClassRec;
