/* $XConsortium: Thermo.c,v 1.4 91/02/17 16:18:42 converse Exp $ */

/* Copyright	Massachusetts Institute of Technology	1987, 1988
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "ThermoP.h"

static XtResource resources[] = {
#define offset(field) XtOffsetOf(ThermoRec, thermo.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNfont, XtCFont, XtRXkwFont, sizeof (XkwFont),
      offset (font), XtRString, XtDefaultFont },
    { XtNmercuryColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (mercuryColor), XtRString, XtDefaultForeground },
    { XtNtickColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (tickColor), XtRString, XtDefaultForeground },
    { XtNthickness, XtCThickness, XtRInt, sizeof (int),
      offset (reqThickness), XtRImmediate, (XtPointer) ThermoUnspecified },
    { XtNminimum, XtCMinimum, XtRInt, sizeof (int),
      offset (minimum), XtRImmediate, (XtPointer) 0},
    { XtNmaximum, XtCMaximum, XtRInt, sizeof (int),
      offset (maximum), XtRImmediate, (XtPointer) 0},
    { XtNcurrent, XtCCurrent, XtRInt, sizeof (int),
      offset (current), XtRImmediate, (XtPointer) 0},
    { XtNminorStart, XtCMinorStart, XtRInt, sizeof (int),
      offset (reqMinorStart), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNmajorStart, XtCMajorStart, XtRInt, sizeof (int),
      offset (reqMajorStart), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNminorStep, XtCMinorStep, XtRInt, sizeof (int),
      offset (reqMinorStep), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNmajorStep, XtCMajorStep, XtRInt, sizeof (int),
      offset (reqMajorStep), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNstartPad, XtCStartPad, XtRDimension, sizeof (Dimension),
      offset (reqStartPad), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNendPad, XtCEndPad, XtRDimension, sizeof (Dimension),
      offset (reqEndPad), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNmajorTickLen, XtCStartPad, XtRDimension, sizeof (Dimension),
      offset (reqMajorTickLen), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNminorTickLen, XtCMinorTickLen, XtRDimension, sizeof (Dimension),
      offset (reqMinorTickLen), XtRImmediate, (XtPointer) ThermoUnspecified},
    { XtNvertical, XtCVertical, XtRBoolean, sizeof (Boolean),
      offset (vertical), XtRImmediate, (XtPointer) FALSE},
#undef offset
};

double
scale_points(ThermoWidget w, double points)
{
    return points * w->ksimple.dpi / 72.0;
}

static void
init_cairo(ThermoWidget w, cairo_t *cr)
{
    cairo_set_font_face(cr, w->thermo.font.font_face);
    cairo_set_font_size(cr, scale_points(w, w->thermo.font.size));
}

static cairo_t *
get_cairo(ThermoWidget w)
{
    cairo_t *cr = XkwGetCairo((Widget) w);
    init_cairo(w, cr);
    return cr;
}

static cairo_t *
draw_begin(ThermoWidget w, Region region)
{
    cairo_t *cr = XkwDrawBegin((Widget) w, region);
    init_cairo(w, cr);
    return cr;
}

static void
draw_end(ThermoWidget w, Region region, cairo_t *cr)
{
    XkwDrawEnd((Widget) w, region, cr);
}

static void
getSize (ThermoWidget w, Dimension *widthp, Dimension *heightp)
{
    int	size;

    if (w->thermo.vertical)
    {
	size = w->thermo.textWidth;
	if (w->thermo.majorTickLen > size)
	    size = w->thermo.majorTickLen;
	if (w->thermo.minorTickLen > size)
	    size = w->thermo.minorTickLen;
	*widthp = size + w->thermo.thickness;
    }
    else
    {
	cairo_t *cr = get_cairo(w);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cr, &font_extents);
	cairo_destroy(cr);

	size = font_extents.height;
	size += w->thermo.majorTickLen;
	if (w->thermo.minorTickLen > size)
	    size = w->thermo.minorTickLen;
	*heightp = size + w->thermo.thickness;
    }
}

static int
NiceValue (int num, int den)
{
    int	v;
    int	l;

    v = num / den;
    if (v == 0)
	return 1;
    if (v < 5)
	return v;
    if (v < 8)
	return 5;
    l = 1;
    while (v > 100)
    {
	v /= 10;
	l++;
    }
    if (v < 15)
	v = 10;
    else if (v < 37)
	v = 25;
    else if (v < 75)
	v = 50;
    else
	v = 100;
    while (l--)
	v *= 10;
    return v;
}

static void
setDefaults (ThermoWidget req, ThermoWidget new)
{
    double max_text_width;
    char label[30];

    cairo_t *cr = get_cairo(new);
    sprintf(label, "%d", req->thermo.minimum);
    cairo_text_extents_t text_extents;
    cairo_text_extents(cr, label, &text_extents);
    max_text_width = text_extents.width;
    sprintf(label, "%d", req->thermo.maximum);
    cairo_text_extents(cr, label, &text_extents);
    if (text_extents.width > max_text_width)
	max_text_width = text_extents.width;

    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);

    new->thermo.textWidth = max_text_width;
    if (req->thermo.reqThickness == ThermoUnspecified)
	new->thermo.thickness = font_extents.height;
    else
	new->thermo.thickness = req->thermo.reqThickness;
    if (req->thermo.reqMinorStart == ThermoUnspecified)
	new->thermo.minorStart = req->thermo.minimum;
    else
	new->thermo.minorStart = req->thermo.reqMinorStart;
    if (req->thermo.reqMajorStart == ThermoUnspecified)
	new->thermo.majorStart = req->thermo.minimum;
    else
	new->thermo.majorStart = req->thermo.reqMajorStart;
    if (req->thermo.reqMajorStep == ThermoUnspecified)
	new->thermo.majorStep = NiceValue (req->thermo.maximum - req->thermo.majorStart, 10);
    else
	new->thermo.majorStep = req->thermo.reqMajorStep;
    if (req->thermo.reqMinorStep == ThermoUnspecified)
	new->thermo.minorStep = NiceValue (new->thermo.majorStep, 4);
    else
	new->thermo.minorStep = req->thermo.reqMinorStep;
    if (req->thermo.reqStartPad == ThermoUnspecified) {
	if (new->thermo.vertical)
	    new->thermo.startPad = 0;
	else
	    new->thermo.startPad = new->thermo.textWidth / 2.0;
    }
    if (req->thermo.reqEndPad == ThermoUnspecified)
    {
	if (new->thermo.vertical)
	    new->thermo.endPad = font_extents.height + 2.0;
	else
	    new->thermo.endPad = new->thermo.textWidth / 2.0;
    }
    if (req->thermo.reqMajorTickLen == ThermoUnspecified)
	new->thermo.majorTickLen = font_extents.height / 2.0;
    else
	new->thermo.majorTickLen = req->thermo.reqMajorTickLen;
    if (req->thermo.reqMinorTickLen == ThermoUnspecified)
	new->thermo.minorTickLen = new->thermo.majorTickLen / 2.0;
    else
	new->thermo.minorTickLen = req->thermo.reqMinorTickLen;
}

static void
ClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    ThermoWidget	req = (ThermoWidget) greq,
			new = (ThermoWidget) gnew;

    (void) args;
    (void) count;
    setDefaults (req, new);
    getSize (new, &new->core.width, &new->core.height);
}

#define AreaPad(w)	((w)->thermo.startPad + (w)->thermo.endPad)
#define VArea(w)	((w)->core.height - AreaPad(w))
#define HArea(w)	((w)->core.width  - AreaPad(w))
#define VerticalPos(w,v)    ((w)->core.height - (w)->thermo.startPad - VArea(w) * (v) / \
			    ((w)->thermo.maximum - (w)->thermo.minimum))
#define HorizontalPos(w,v)    ((w)->thermo.startPad + HArea(w) * (v) / \
			    ((w)->thermo.maximum - (w)->thermo.minimum))

static void
drawMercury (ThermoWidget	w,
	     cairo_t		*cr,
	     int		old,
	     int		new)
{
    double	    x, y, other, width, height;

    if (w->thermo.vertical)
    {
	width = w->thermo.thickness;
	x = w->core.width - width;
	other = VerticalPos (w, old);
	y = VerticalPos (w, new);
	height = other - y;
	if (height < 0) {
	    XkwSetSource(cr, &w->ksimple.background);
	    cairo_rectangle(cr, x, other, width, -height);
	} else {
	    XkwSetSource(cr, &w->thermo.mercuryColor);
	    cairo_rectangle (cr, x, y, width, height);
	}
    }
    else
    {
	height = w->thermo.thickness;
	y = w->core.height - height;
	x = HorizontalPos (w, old);
	other = HorizontalPos (w, new);
	width = other - x;
	if (width < 0) {
	    XkwSetSource(cr, &w->ksimple.background);
	    cairo_rectangle(cr, other, y, -width, height);
	} else {
	    XkwSetSource(cr, &w->thermo.mercuryColor);
	    cairo_rectangle(cr, x, y, width, height);
	}
    }
    cairo_fill(cr);
}

static void
drawTick (ThermoWidget	w,
	  cairo_t	*cr,
	  int		v,
	  double	len)
{
    double	x, y, width, height;

    if (w->thermo.vertical) {
	x = w->core.width - w->thermo.thickness - len;
	width = len;
	height = len / 10.0;
	y = VerticalPos (w, v) - height / 2.0;
    }
    else
    {
	y = w->core.height - w->thermo.thickness - len;
	height = len;
	width = len / 10.0;
	x = HorizontalPos (w, v) - width / 2.0;
    }
    XkwSetSource(cr, &w->thermo.tickColor);
    cairo_rectangle(cr, x, y, width, height);
    cairo_fill(cr);
}

static void
drawValue (ThermoWidget	w,
	   cairo_t	*cr,
	   int		v)
{
    char    label[30];
    double  width;
    double  x, y;
    cairo_text_extents_t text_extents;

    sprintf (label, "%d", v);
    cairo_text_extents(cr, label, &text_extents);
    width = text_extents.width;
    if (w->thermo.vertical) {
	x = w->core.width - w->thermo.thickness - width;
	y = VerticalPos (w, v) - 2;
    }
    else
    {
	y = w->core.height - w->thermo.thickness - w->thermo.majorTickLen - 2;
	x = HorizontalPos (w, v) - width / 2.0 - text_extents.x_bearing;
    }
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, label);
}

static void
Redisplay (Widget  gw,
	   XEvent  *event,
	   Region  region)
{
    ThermoWidget    w = (ThermoWidget) gw;
    int		    v;

    (void) event;
    cairo_t *cr = draw_begin(w, region);
    drawMercury (w, cr, w->thermo.minimum, w->thermo.current);
    for (v = w->thermo.minorStart; v <= w->thermo.maximum; v += w->thermo.minorStep)
	drawTick (w, cr, v, w->thermo.minorTickLen);
    for (v = w->thermo.majorStart; v <= w->thermo.maximum; v += w->thermo.majorStep)
    {
	drawTick (w, cr, v, w->thermo.majorTickLen);
	drawValue (w, cr, v);
    }
    draw_end(w, region, cr);
}

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    ThermoWidget    cur = (ThermoWidget) gcur,
		    req = (ThermoWidget) greq,
		    new = (ThermoWidget) gnew;
    Boolean	    redraw = FALSE;
    Dimension	    width, height;

    (void) args;
    (void) count;
    if (memcmp(&req->thermo.mercuryColor, &cur->thermo.mercuryColor, sizeof (XRenderColor)) != 0)
    {
	redraw = TRUE;
    }
    if (memcmp(&req->thermo.tickColor, &cur->thermo.tickColor, sizeof (XRenderColor)) != 0)
    {
	redraw = TRUE;
    }
    if (req->thermo.minimum != cur->thermo.minimum ||
	req->thermo.maximum != cur->thermo.maximum ||
	req->thermo.reqThickness != cur->thermo.reqThickness ||
	req->thermo.reqMinorStart != cur->thermo.reqMinorStart ||
	req->thermo.reqMajorStart != cur->thermo.reqMajorStart ||
	req->thermo.reqMinorStep != cur->thermo.reqMinorStep ||
	req->thermo.reqMajorStep != cur->thermo.reqMajorStep ||
	req->thermo.reqStartPad != cur->thermo.reqStartPad ||
	req->thermo.reqEndPad != cur->thermo.reqEndPad ||
	req->thermo.reqMinorTickLen != cur->thermo.reqMinorTickLen ||
	req->thermo.reqMajorTickLen != cur->thermo.reqMajorTickLen ||
	req->thermo.vertical != cur->thermo.vertical)
    {
	setDefaults (req, new);
	getSize (new, &width, &height);
	XtMakeResizeRequest (gnew, width, height, &width, &height);
	redraw = TRUE;
    }
    if (!redraw && req->thermo.current != cur->thermo.current)
    {
	cairo_t *cr = get_cairo(new);
	drawMercury (new, cr, cur->thermo.current, req->thermo.current);
	cairo_destroy(cr);
    }
    return redraw;
}

ThermoClassRec thermoClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &ksimpleClassRec,
    /* class_name		*/	"Thermo",
    /* widget_size		*/	sizeof(ThermoRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	XtInheritResize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
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
    /* empty			*/	0
  },
  { /* thermo fields */
    /* empty			*/	0
  }
};

WidgetClass thermoWidgetClass = (WidgetClass)&thermoClassRec;
