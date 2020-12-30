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

#include <Xkw/KLabelP.h>

static XtResource resources[] = {
#define offset(field) XtOffsetOf(KLabelRec, klabel.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNlabel, XtCLabel, XtRString, sizeof(String),
      offset (label), XtRString, NULL },
    { XtNresize, XtCResize, XtRBoolean, sizeof(Boolean),
      offset(resize), XtRImmediate, (XtPointer) True },
    { XtNfont, XtCFont, XtRXkwFont, sizeof (XkwFont),
      offset (font), XtRString, XtDefaultFont },
    { XtNjustify, XtCJustify, XtRJustify, sizeof(XtJustify),
      offset (justify), XtRImmediate, (XtPointer) XtJustifyCenter },
    { XtNshapeStyle, XtCShapeStyle, XtRShapeStyle, sizeof(int),
      offset(shape_style), XtRImmediate, (XtPointer)XmuShapeRectangle },
    { XtNcornerRoundPercent, XtCCornerRoundPercent, XtRDimension, sizeof(Dimension),
      offset(corner_round), XtRImmediate, (XtPointer)25 },
#undef offset
};

#define superclass (&ksimpleClassRec)

static void
XkwKLabelClassInitialize(void)
{
    XtSetTypeConverter(XtRString, XtRShapeStyle, XmuCvtStringToShapeStyle,
		       NULL, 0, XtCacheNone, NULL);
}

static void
init_cairo(KLabelWidget w, cairo_t *cr)
{
    cairo_set_font_face(cr, w->klabel.font.font_face);
    cairo_set_font_size(cr, w->klabel.font.size * w->ksimple.dpi / 72.0);
}

static cairo_t *
get_cairo(KLabelWidget w)
{
    cairo_t *cr = XkwGetCairo((Widget) w);
    init_cairo(w, cr);
    return cr;
}

static cairo_t *
draw_begin(KLabelWidget w)
{
    cairo_t *cr = XkwDrawBegin((Widget) w);
    init_cairo(w, cr);
    return cr;
}

static void
draw_end(KLabelWidget w, cairo_t *cr)
{
    XkwDrawEnd((Widget) w, cr);
}

static double
pad(cairo_font_extents_t *font_extents)
{
    return font_extents->height / 4.0;
}

static void
preferred_size(KLabelWidget w, Dimension *width, Dimension *height)
{
    cairo_t *cr = get_cairo(w);
    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    cairo_text_extents(cr, w->klabel.label, &text_extents);
    cairo_destroy(cr);

    *width = text_extents.width + pad(&font_extents) * 2;
    *height = font_extents.height + pad(&font_extents) * 2;
}

static void
XkwKLabelInitialize(Widget request, Widget cnew,
		    ArgList args, Cardinal *num_args)
{
    KLabelWidget w = (KLabelWidget)cnew;

    if (w->klabel.label == NULL)
	w->klabel.label = XtNewString(w->core.name);
    else
	w->klabel.label = XtNewString(w->klabel.label);

    if (XtWidth(w) == 0 || XtHeight(w) == 0) {
	Dimension width, height;
	preferred_size(w, &width, &height);
	if (XtWidth(w) == 0)
	    XtWidth(w) = width;
	if (XtHeight(w) == 0)
	    XtHeight(w) = height;
    }
}

static void
XkwKLabelDestroy(Widget gw)
{
    KLabelWidget w = (KLabelWidget)gw;

    if (w->klabel.label != w->core.name)
	XtFree(w->klabel.label);
}

static void
XkwKLabelRedisplay(Widget gw, XEvent *event, Region region)
{
    KLabelWidget w = (KLabelWidget)gw;
    if (*superclass->core_class.expose != NULL)
	(*superclass->core_class.expose)(gw, event, region);

    cairo_t *cr = draw_begin(w);

    if (XtIsSensitive(gw))
	XkwSetSource(cr, &w->ksimple.foreground);
    else
	XkwSetSourceInterp(cr, &w->ksimple.foreground, &w->ksimple.background);
    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    cairo_text_extents(cr, w->klabel.label, &text_extents);

    double x;
    switch (w->klabel.justify) {
    case XtJustifyLeft:
	x = pad(&font_extents);
	break;
    case XtJustifyRight:
	x = w->core.width - text_extents.width - pad(&font_extents);
	break;
    default:
	x = (w->core.width - text_extents.width) / 2;
	break;
    }
    double y = (w->core.height - font_extents.height) / 2 + font_extents.ascent;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, w->klabel.label);
    draw_end(w, cr);
}

static Boolean
XkwKLabelSetValues(Widget gcur, Widget greq, Widget gnew,
		   ArgList args, Cardinal *num_args)
{
    KLabelWidget cur = (KLabelWidget)gcur;
    KLabelWidget req = (KLabelWidget)greq;
    KLabelWidget new = (KLabelWidget)gnew;
    Boolean was_resized = False;

    (void) req;
    if (new->klabel.label != cur->klabel.label) {
	XtFree(cur->klabel.label);
	if (!new->klabel.label)
	    new->klabel.label = XtNewString(new->core.name);
	else
	    new->klabel.label = XtNewString(new->klabel.label);
	was_resized = True;
    }
    if (new->klabel.font.font_face != cur->klabel.font.font_face ||
	new->klabel.font.size != cur->klabel.font.size)
	was_resized = True;
    if (new->klabel.resize && was_resized) {
	Dimension width, height;

	preferred_size(new, &width, &height);
	if (XtHeight(cur) == XtHeight(req))
	    XtHeight(new) = height;
	if (XtWidth(cur) == XtWidth(req))
	    XtWidth(new) = width;
    }
    return TRUE;
}

static XtGeometryResult
XkwKLabelQueryGeometry(Widget gw, XtWidgetGeometry *intended,
		       XtWidgetGeometry *preferred)
{
    KLabelWidget w = (KLabelWidget)gw;

    preferred->request_mode = CWWidth | CWHeight;
    preferred_size(w, &preferred->width, &preferred->height);

    if (((intended->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	&& intended->width == preferred->width
	&& intended->height == preferred->height)
	return (XtGeometryYes);
      else if (preferred->width == XtWidth(w) && preferred->height == XtHeight(w))
	return (XtGeometryNo);

    return (XtGeometryAlmost);
}

KLabelClassRec klabelClassRec = {
  /* core */
  {
    (WidgetClass)superclass,		/* superclass */
    "KLabel",				/* class_name */
    sizeof(KLabelRec),			/* widget_size */
    XkwKLabelClassInitialize,		/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    XkwKLabelInitialize,		/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    NULL,				/* actions */
    0,					/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    XkwKLabelDestroy,			/* destroy */
    NULL,				/* resize */
    XkwKLabelRedisplay,			/* expose */
    XkwKLabelSetValues,			/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    NULL,				/* tm_table */
    XkwKLabelQueryGeometry,		/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
  },
  /* klabel */
  {
    0,					/* extension */
  }
};

WidgetClass klabelWidgetClass = (WidgetClass)&klabelClassRec;
