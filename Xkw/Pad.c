/*
 * $NCD$
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
#include <X11/Xatom.h>
#include "PadP.h"

static void
Clear (PadLinePtr l, int n)
{
    char	*t, *a;

    t = l->text;
    a = l->attr;
    while (n--) {
	*t++ = ' ';
	*a++ = XkwPadNormal;
    }
    *t = '\0';
}

static void
ResizeBuffer (PadLinePtr *bp, Dimension old_rows, Dimension old_cols, Dimension new_rows, Dimension new_cols)
{
    PadLinePtr	oldb;
    PadLinePtr	b;
    int		row;
    int		max_row, max_col;

    b = Some(PadLineRec, new_rows);
    for (row = 0; row < new_rows; row++)
	AllocText (&b[row], new_cols);
    oldb = *bp;
    if (oldb)
    {
	max_col = new_cols;
	if (max_col > old_cols)
	    max_col = old_cols;
	max_row = new_rows;
	if (max_row > old_rows)
	    max_row = old_rows;
	for (row = 0; row < max_row; row++)
	    CopyText (&oldb[row], &b[row], 0, max_col);
	for (row = 0; row < old_rows; row++)
	    DisposeText (&oldb[row]);
	Dispose (oldb);
    }
    *bp = b;
}

static void
ResizeText (PadWidget w, Dimension rows, Dimension cols)
{
    ResizeBuffer (&w->pad.lines, w->pad.rows, w->pad.cols, rows, cols);
    w->pad.rows = rows;
    w->pad.cols = cols;
    if (w->pad.resize_callbacks != NULL)
	XtCallCallbackList ((Widget) w, w->pad.resize_callbacks, (XtPointer) NULL);
}

static void
init_cairo(PadWidget w, cairo_t *cr)
{
    cairo_set_font_face(cr, w->pad.font.font_face);
    cairo_set_font_size(cr, w->pad.font.size * w->ksimple.dpi / 72.0);
}

static cairo_t *
get_cairo(PadWidget w)
{
    cairo_t *cr = XkwGetCairo((Widget) w);
    init_cairo(w, cr);
    return cr;
}

static cairo_t *
draw_begin(PadWidget w, Region region)
{
    cairo_t *cr = XkwDrawBegin((Widget) w, region);
    init_cairo(w, cr);
    return cr;
}

static void
draw_end(PadWidget w, Region region, cairo_t *cr)
{
    XkwDrawEnd((Widget) w, region, cr);
}

static int
show_text(cairo_t *cr, XRenderColor *fg, XRenderColor *bg, double x, double y, char *text, int len)
{
    char save = text[len];
    if (save != '\0')
	text[len] = '\0';
    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_text_extents(cr, text, &text_extents);
    cairo_font_extents(cr, &font_extents);
    if (bg) {
	XkwSetSource(cr, bg);
	cairo_rectangle(cr, x, y - font_extents.ascent, text_extents.x_advance, font_extents.ascent + font_extents.descent);
	cairo_fill(cr);
    }
    XkwSetSource(cr, fg);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
    if (save != '\0')
	text[len] = save;
    return text_extents.x_advance;
}

static double
text_width(cairo_t *cr, char *text, int len)
{
    char save = text[len];
    if (save != '\0')
	text[len] = '\0';
    cairo_text_extents_t text_extents;
    cairo_text_extents(cr, text, &text_extents);
    if (save != '\0')
	text[len] = save;
   return text_extents.x_advance;
}

static void
getSize (PadWidget w, Dimension rows, Dimension cols, Dimension *widthp, Dimension *heightp)
{
    cairo_t *cr = get_cairo(w);
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);

    w->pad.underline_pos = (font_extents.descent) / 2;
    w->pad.underline_thickness = font_extents.height / 10;

    if (w->pad.underline_thickness < 1)
	w->pad.underline_thickness = 1;
    if (w->pad.underline_pos + w->pad.underline_thickness > font_extents.descent)
    {
	if (w->pad.underline_pos > 1)
	    w->pad.underline_pos = font_extents.descent - w->pad.underline_thickness;
	if (w->pad.underline_pos <= 1)
	{
	    if (w->pad.underline_pos < 0)
		w->pad.underline_pos = 0;
	    w->pad.underline_thickness = font_extents.descent - w->pad.underline_pos;
	}
    }
    w->pad.char_width = text_width(cr, "0", 1);
    w->pad.char_height = font_extents.height * 1.1;
    w->pad.char_vAdjust = font_extents.ascent;
    w->pad.char_hAdjust = 0;
    *widthp = w->pad.char_width * cols + 2 * w->pad.internal_border;
    *heightp = w->pad.char_height * rows + 2 * w->pad.internal_border;
    cairo_destroy(cr);
}

static void
setSize (PadWidget w)
{
    int	rows, cols;

    rows = ((int) w->core.height - 2 * (int) w->pad.internal_border) / (int) w->pad.char_height;
    if (rows <= 0)
	rows = 1;
    cols = ((int) w->core.width - 2 * (int) w->pad.internal_border) / (int) w->pad.char_width;
    if (cols <= 0)
	cols = 1;
    if (rows != w->pad.rows || cols != w->pad.cols)
	ResizeText (w, rows, cols);
}

static void
ClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    PadWidget	new = (PadWidget) gnew;

    (void) greq;
    (void) args;
    (void) count;
    getSize (new, new->pad.rows, new->pad.cols,
	     &new->core.width, &new->core.height);
    new->pad.lines = 0;
    ResizeText (new, new->pad.rows, new->pad.cols);
}

static int
XToCol(PadWidget w, cairo_t *cr, int row, int x)
{
    char    *c;
    int	    col;

    c = w->pad.lines[row].text;
    for (col = 0; col < w->pad.cols - 1; col++)
	if (x < text_width (cr, c, col))
	    break;
    return col;
}

static void
DrawText (PadWidget w, cairo_t *cr, int row, int start_col, int end_col)
{
    int		change_col;
    char	attr;
    char	*a, *t;
    PadLinePtr	line;
    int		x, y;
    int		width;

    line = &w->pad.lines[row];
    a = line->attr + start_col;
    t = line->text + start_col;
    change_col = start_col;
    x = (w)->pad.internal_border + w->pad.char_hAdjust + text_width(cr, line->text, start_col);
    y = (w)->pad.internal_border + w->pad.char_vAdjust + row * w->pad.char_height;
    while (start_col < end_col) {
	attr = *a;
	do {
	    ++a;
	    ++change_col;
	} while (change_col < end_col && *a == attr);

	XRenderColor *fg = &w->ksimple.foreground;
	XRenderColor *bg = NULL;

	if (attr & XkwPadBold)
	    fg = &w->pad.bold;

	if (attr & XkwPadInverse) {
	    bg = fg;
	    fg = &w->ksimple.background;
	}

	width = show_text(cr, fg, bg, x, y, t, change_col - start_col);
	if (attr & XkwPadUnderline)
	{
	    cairo_rectangle(cr, x, y + w->pad.underline_pos,
			    width, w->pad.underline_thickness);
	    cairo_fill(cr);
	}
/*
	if (attr & XkwPadOutline)
	{
	    int	    o_col, o_x, o_y, o_w, o_h;

	    o_x = x;
	    o_y = y - w->pad.font->ascent;
	    o_h = w->pad.font->ascent + w->pad.font->descent;
	    for (o_col = start_col; o_col < change_col; o_col++) {
		o_w = XTextWidth (w->pad.font, is->text + o_col, 1);
		if (o_w && o_h)
		{
		    XDrawRectangle (dpy, win, gc,
				    o_x, o_y, o_w - 1, o_h - 1);
		}
	    }
	}
*/
	x += width;
	t += (change_col - start_col);
	start_col = change_col;
    }
}

static void
RedrawText (PadWidget w, cairo_t *cr, int row, int start_col, int end_col)
{
    char    *t, *a;

    t = w->pad.lines[row].text + start_col;
    a = w->pad.lines[row].attr + start_col;
    while (start_col < end_col && *t == ' ' && *a == XkwPadNormal) {
	t++;
	a++;
	start_col++;
    }

    t = w->pad.lines[row].text + end_col - 1;
    a = w->pad.lines[row].attr + end_col - 1;
    while (end_col > start_col && *t == ' ' && *a == XkwPadNormal) {
	t--;
	a--;
	end_col--;
    }

    if (start_col < end_col)
	DrawText (w, cr, row, start_col, end_col);
}

static void
Redisplay (Widget gw, XEvent *event, Region region)
{
    PadWidget   w = (PadWidget) gw;
    int row;

    if (!XtIsRealized (gw))
	return;
    if (!event || (event->type == Expose && event->xexpose.count == 0))
    {
	cairo_t *cr = draw_begin(w, region);

	for (row = 0; row < w->pad.rows; row++)
	    RedrawText (w, cr, row, 0, w->pad.cols);

	draw_end(w, region, cr);
    }
}

static void
Resize (Widget gw)
{
    PadWidget   w = (PadWidget) gw;

    setSize (w);
}

void
XkwPadUpdate (Widget gw)
{
    Redisplay(gw, NULL, NULL);
}

void
XkwPadText (Widget gw, int row, int col, char *text, int len)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    line;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    line = &w->pad.lines[row];
    if (col + len > w->pad.cols)
	len = w->pad.cols - col;
    memcpy (line->text + col, text, len);
}

void
XkwPadAttributes (Widget gw, int row, int col, char *attr, int len)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    line;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    line = &w->pad.lines[row];
    if (col + len > w->pad.cols)
	len = w->pad.cols - col;
    memcpy (line->attr + col, attr, len);
}

void
XkwPadTextAndAttributes (Widget gw, int row, int col, char *text, char *attr, int len)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    line;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    line = &w->pad.lines[row];
    if (col + len > w->pad.cols)
	len = w->pad.cols - col;
    memcpy (line->text + col, text, len);
    memcpy (line->attr + col, attr, len);
}

void
XkwPadClearToEnd (Widget gw, int row, int col)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    line;
    char	    *t, *a;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    line = &w->pad.lines[row];
    t = line->text + col;
    a = line->attr + col;
    col = w->pad.cols - col;
    while (col--)
    {
	*t++ = ' ';
	*a++ = XkwPadNormal;
    }
}

void
XkwPadClear (Widget gw)
{
    PadWidget	    w = (PadWidget) gw;
    int		    row;

    for (row = 0; row < w->pad.rows; row++)
	XkwPadClearToEnd (gw, row, 0);
}

static void
ScrollBuffer (PadWidget w, PadLinePtr b, int start_row, int end_row, int dist)
{
    int		    first_row, row, next_row;
    PadLineRec	    tmp1, tmp2;
    int		    n;

    if (end_row <= start_row)
	return;
    n = end_row - start_row;
    first_row = start_row;
    while (n) {
	tmp2 = b[first_row];
	row = first_row;
	do
	{
	    next_row = row + dist;
	    if (next_row < start_row)
		next_row = next_row + end_row - start_row;
	    else if (next_row >= end_row)
		next_row = next_row - end_row + start_row;
	    tmp1 = b[next_row];
	    b[next_row] = tmp2;
	    tmp2 = tmp1;
	    n--;
	    row = next_row;
	} while (row != first_row);
	first_row++;
    }
    n = dist;
    if (n < 0)
    {
	n = -n;
	row = end_row - n;
    }
    else
	row = start_row;
    while (n--)
    {
	Clear (&b[row], w->pad.cols);
	row++;
    }
}

void
XkwPadScroll (Widget gw, int start_row, int end_row, int dist)
{
    PadWidget	    w = (PadWidget) gw;

    ScrollBuffer (w, w->pad.lines, start_row, end_row, dist);
}

void
XkwPadXYToRowCol (Widget gw, int x, int y, int *rowp, int *colp)
{
    PadWidget	    w = (PadWidget) gw;
    int		    row, col;

    cairo_t *cr = get_cairo(w);
    row = RowPos (w, y);
    if (row < 0)
	row = 0;
    if (row >= w->pad.rows)
	row = w->pad.rows - 1;
//    if (w->pad.fixed_width)
//	col = ColPos (w, x);
//    else
    col = XToCol (w, cr, row, x);
    if (col < 0)
	col = 0;
    if (col >= w->pad.cols)
	col = w->pad.cols - 1;
    *rowp = row;
    *colp = col;
    cairo_destroy(cr);
}

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    PadWidget	    cur = (PadWidget) gcur,
		    req = (PadWidget) greq,
		    new = (PadWidget) gnew;
    Boolean	    redraw = FALSE, newsize = FALSE;
    Dimension	    width, height;

    (void) args;
    (void) count;
    if (memcmp(&req->ksimple.foreground, &cur->ksimple.foreground, sizeof(XRenderColor)) != 0)
	redraw = TRUE;
    if (req->pad.font.font_face != cur->pad.font.font_face || req->pad.font.size != cur->pad.font.size)
	newsize = TRUE;
    if (req->pad.rows != cur->pad.rows ||
	req->pad.cols != cur->pad.cols)
    {
	newsize = TRUE;
    }
    if (newsize)
    {
	getSize (new, req->pad.rows, req->pad.cols, &width, &height);
	new->pad.rows = cur->pad.rows;
	new->pad.cols = cur->pad.cols;
	XtMakeResizeRequest (gnew, width, height, &width, &height);
	redraw = TRUE;
    }
    return redraw;
}

static XtResource resources[] = {
#define offset(field) XtOffsetOf(PadRec, pad.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNfont, XtCFont, XtRXkwFont, sizeof (XkwFont),
      offset (font), XtRString, XtDefaultFont },
    { XtNbold, XtCBold, XtRRenderColor, sizeof (XRenderColor),
      offset (bold), XtRString, "red" },
    { XtNnumRows, XtCNumRows, XtRDimension, sizeof (Dimension),
      offset (rows), XtRImmediate, (XtPointer) 1},
    { XtNnumCols, XtCNumCols, XtRDimension, sizeof (Dimension),
      offset (cols), XtRImmediate, (XtPointer) 1},
    {XtNinternalBorderWidth, XtCInternalBorderWidth, XtRDimension, sizeof (Dimension),
     offset(internal_border), XtRImmediate, (XtPointer) 2},
    {XtNresizeCallback, XtCCallback, XtRCallback,sizeof(XtPointer),
      offset(resize_callbacks), XtRCallback, (XtPointer)NULL},
#undef offset
};

PadClassRec padClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &ksimpleClassRec,
    /* class_name		*/	"Pad",
    /* widget_size		*/	sizeof(PadRec),
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
    /* compress_exposure	*/	XtExposeCompressSeries|XtExposeGraphicsExpose|XtExposeNoExpose,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	Resize,
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
  /* ksimple */
  {
    0,
  },
  { /* pad fields */
    /* empty			*/	0
  }
};

WidgetClass padWidgetClass = (WidgetClass)&padClassRec;
