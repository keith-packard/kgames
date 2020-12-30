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

#define superclass	(&simpleClassRec)

static void
KSimpleClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

static void
KSimpleRealize (Widget gw,
		XtValueMask *value_mask,
		XSetWindowAttributes *attributes)
{
    KSimpleWidget w = (KSimpleWidget) gw;

    (*superclass->core_class.realize)(gw, value_mask, attributes);
    w->ksimple.surface = cairo_xlib_surface_create(XtDisplay(w),
						   XtWindow(w),
						   XtScreen(w)->root_visual,
						   XtWidth(w),
						   XtHeight(w));
}

static void
KSimpleResize(Widget gw)
{
    KSimpleWidget w = (KSimpleWidget) gw;

    cairo_surface_destroy(w->ksimple.surface);
    w->ksimple.surface = cairo_xlib_surface_create(XtDisplay(w),
						   XtWindow(w),
						   XtScreen(w)->root_visual,
						   XtWidth(w),
						   XtHeight(w));
}

static void
KSimpleDestroy(Widget gw)
{
    KSimpleWidget w = (KSimpleWidget) gw;

    cairo_surface_destroy(w->ksimple.surface);
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
};
#undef offset

KSimpleClassRec ksimpleClassRec = {
  /* core */
  {
    (WidgetClass)&simpleClassRec,	/* superclass */
    "KSimple",				/* class_name */
    sizeof(KSimpleRec),			/* widget_size */
    KSimpleClassInitialize,		/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    NULL,				/* initialize */
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
    KSimpleResize,			/* resize */
    NULL,				/* expose */
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
