/* $XConsortium: CribBoard.c,v 1.4 91/02/17 16:18:42 converse Exp $ */

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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <math.h>
#include "CribBoardP.h"

static XtResource resources[] = {
#define offset(field) XtOffsetOf(CribBoardRec, cribBoard.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNpegColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (pegColor), XtRString, XtDefaultForeground },
    { XtNholeColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (holeColor), XtRString, XtDefaultForeground },
    { XtNnumPegs, XtCNumPegs, XtRInt, sizeof (int),
      offset (numPegs), XtRImmediate, (XtPointer) 2 },
    { XtNnumCols, XtCNumCols, XtRInt, sizeof (int),
      offset (numCols), XtRImmediate, (XtPointer) 30 },
    { XtNnumRows, XtCNumRows, XtRInt, sizeof (int),
      offset (numRows), XtRImmediate, (XtPointer) 4 },
#undef offset
};

#define PEG_SIZE	4.0
#define PEG_LENGTH	8.0
#define HOLE_SIZE	3.0
#define GROUP_SPACE	4.0
#define PEG_SPACE	10.0
#define TRACK_WIDTH     1.0
#define BORDER_WIDTH    12.0

static inline double
scaleSize(CribBoardWidget w, double size)
{
    return w->ksimple.dpi * size / 72.0;
}

static inline double
pegSize(CribBoardWidget w)
{
    return scaleSize(w, PEG_SIZE);
}

static inline double
pegLength(CribBoardWidget w)
{
    return scaleSize(w, PEG_LENGTH);
}

static inline double
holeSize(CribBoardWidget w)
{
    return scaleSize(w, HOLE_SIZE);
}

static inline double
trackWidth(CribBoardWidget w)
{
    return scaleSize(w, TRACK_WIDTH);
}

static inline double
groupSpace(CribBoardWidget w)
{
    return scaleSize(w, GROUP_SPACE);
}

static inline double
pegSpace(CribBoardWidget w)
{
    return scaleSize(w, PEG_SPACE);
}

static inline double
RowPos(CribBoardWidget w, int row)
{
    return (row + 0.5) * pegSpace(w);
}

static inline double
ColPos(CribBoardWidget w, int col)
{
    int	group = col / 5;

    return (col + 0.5) * (pegSpace(w)) + group * groupSpace(w);
}

static inline double
borderWidth(CribBoardWidget w)
{
    return scaleSize(w, BORDER_WIDTH);
}

static void
getSize (CribBoardWidget w, Dimension *widthp, Dimension *heightp)
{
    *widthp = ColPos(w, w->cribBoard.numCols - 1) + pegSize(w) + borderWidth(w) * 2;
    *heightp = RowPos(w, w->cribBoard.numRows - 1) + pegSize(w) + borderWidth(w) * 2;
}

static void
ClassInitialize (void)
{
    XkwInitializeWidgetSet();
}

static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    CribBoardWidget	req = (CribBoardWidget) greq,
			new = (CribBoardWidget) gnew;
    int			i;

    (void) args;
    (void) count;
    getSize (new, &new->core.width, &new->core.height);
    new->cribBoard.pegs = Some (int, req->cribBoard.numPegs);
    for (i = 0; i < req->cribBoard.numPegs; i++)
	new->cribBoard.pegs[i] = CribBoardUnset;
}

static void
Destroy (Widget gw)
{
    CribBoardWidget    w = (CribBoardWidget) gw;

    Dispose (w->cribBoard.pegs);
}

/*
 * The peg is drawn as a trapezoid topped by a filled arc
 */

#define ANGLE	    60	    /* angle from vertical */
#define COS	    0.5
#define SIN	    0.866025403784439

#define PEG_RAD	    (pegSize(w))
#define HOLE_RAD    (holeSize(w))

#define CIRCLE_X    -COS * pegLength(w)
#define CIRCLE_Y    -SIN * pegLength(w)

#define POLY_X0	    SIN * HOLE_RAD
#define POLY_Y0	    -COS * HOLE_RAD

#define POLY_X1	    -SIN * HOLE_RAD
#define POLY_Y1	    COS * HOLE_RAD

#define POLY_X2	    CIRCLE_X - PEG_RAD * SIN
#define POLY_Y2	    CIRCLE_Y + PEG_RAD * COS

#define POLY_X3	    CIRCLE_X + PEG_RAD * SIN
#define POLY_Y3	    CIRCLE_Y - PEG_RAD * COS


static void
drawPeg (CribBoardWidget w, cairo_t *cr, int value)
{
    int	    row, col;
    double	x, y;

    if (value == CribBoardUnset)
	return;
    if (value > w->cribBoard.numCols * w->cribBoard.numRows)
        value = w->cribBoard.numCols * w->cribBoard.numRows;

    row = value / w->cribBoard.numCols;
    col = value % w->cribBoard.numCols;
    if (row & 1)
	col = (w->cribBoard.numCols - 1) - col;
    x = ColPos (w, col);
    y = RowPos (w, row);
    cairo_save(cr);
    XkwSetSource(cr, &w->cribBoard.pegColor);
    cairo_translate(cr, x, y);
    cairo_move_to(cr, POLY_X0, POLY_Y0);
    cairo_line_to(cr, POLY_X1, POLY_Y1);
    cairo_line_to(cr, POLY_X2, POLY_Y2);
    cairo_line_to(cr, POLY_X3, POLY_Y3);
    cairo_arc(cr, CIRCLE_X, CIRCLE_Y, pegSize(w), 0, 2 * M_PI);
    cairo_arc(cr, 0, 0, holeSize(w), 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_restore(cr);
}

static void
drawHole (CribBoardWidget w, cairo_t *cr, int value)
{
    int	    row, col;
    double  x;
    double  y;

    row = value / w->cribBoard.numCols;
    col = value % w->cribBoard.numCols;
    x = ColPos (w, col);
    y = RowPos (w, row);
    cairo_save(cr);
    XkwSetSource(cr, &w->cribBoard.holeColor);
    cairo_translate(cr, x, y);
    cairo_arc(cr, 0, 0, holeSize(w), 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_restore(cr);
}

static void
drawTrack (CribBoardWidget w, cairo_t *cr, XRenderColor *color)
{
    int row;

    cairo_save(cr);
    XkwSetSource(cr, color);
    cairo_set_line_width(cr, trackWidth(w));

    for (row = 0; row < w->cribBoard.numRows; row += 2) {
        double left_x = ColPos (w, 0);
        double right_x = ColPos (w, w->cribBoard.numCols - 1);
        double top_y = RowPos (w, row);
        double bottom_y = RowPos (w, row + 1);

        if (row == 0)
            cairo_move_to(cr, left_x, top_y);

        cairo_line_to(cr, right_x + pegSize(w), top_y);
        cairo_arc(cr, right_x + pegSize(w), (top_y + bottom_y) / 2, (bottom_y - top_y) / 2, -M_PI/2.0, M_PI/2.0);

        if (row < w->cribBoard.numRows - 2) {
            cairo_line_to(cr, left_x - pegSize(w), bottom_y);

            cairo_arc_negative(cr, left_x - pegSize(w), (bottom_y + RowPos(w, row+2)) / 2, (bottom_y - top_y) / 2, -M_PI/2.0, M_PI/2.0);
        } else
            cairo_line_to(cr, left_x, bottom_y);
    }
    cairo_stroke(cr);
    cairo_restore(cr);
}

static void
Redisplay (Widget gw, XEvent *event, Region region)
{
    CribBoardWidget w = (CribBoardWidget) gw;
    int		    v;
    cairo_t	    *cr = XkwDrawBegin(gw, region);
    Dimension	    natural_width, natural_height;

    getSize(w, &natural_width, &natural_height);

    /* scale to fit area */
    double width_ratio = (double) XtWidth(w) / (double) natural_width;
    double height_ratio = (double) XtHeight(w) / (double) natural_height;

    if (width_ratio > height_ratio)
	width_ratio = height_ratio;

    double bw = borderWidth(w);

    cairo_save(cr);
    cairo_scale(cr, width_ratio, width_ratio);
    cairo_translate(cr, bw, bw);

    (void) event;
    drawTrack (w, cr, &w->cribBoard.pegColor);
    for (v = 0; v < w->cribBoard.numRows * w->cribBoard.numCols; v++)
	drawHole (w, cr, v);
    for (v = 0; v < w->cribBoard.numPegs; v++)
	drawPeg (w, cr, w->cribBoard.pegs[v]);
    cairo_restore(cr);
    XkwDrawEnd(gw, region, cr);
}

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    CribBoardWidget    cur = (CribBoardWidget) gcur,
		    req = (CribBoardWidget) greq,
		    new = (CribBoardWidget) gnew;
    Boolean	    redraw = FALSE;

    (void) args;
    (void) count;
    (void) new;
    if (!XkwColorEqual(&req->cribBoard.pegColor, &cur->cribBoard.pegColor))
	redraw = TRUE;
    if (!XkwColorEqual(&req->cribBoard.holeColor, &cur->cribBoard.holeColor))
	redraw = TRUE;
    return redraw;
}

CribBoardClassRec cribBoardClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &ksimpleClassRec,
    /* class_name		*/	"CribBoard",
    /* widget_size		*/	sizeof(CribBoardRec),
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
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
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
  },
  /* ksimple */
  {
      0,				/* not used */
  },
  { /* cribBoard fields */
    /* empty			*/	0
  }
};

WidgetClass cribBoardWidgetClass = (WidgetClass)&cribBoardClassRec;

void
XkwCribBoardSetPeg (Widget gw, int i, int v)
{
    CribBoardWidget w = (CribBoardWidget) gw;

    if (0 <= i && i < w->cribBoard.numPegs && w->cribBoard.pegs[i] != v)
    {
	w->cribBoard.pegs[i] = v;
	Redisplay(gw, NULL, NULL);
    }
}
