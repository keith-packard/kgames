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

static XtResource resources[] = {
#define offset(field) XtOffsetOf(PadRec, pad.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNfont, XtCFont, XtRXkwFont, sizeof (XkwFont),
      offset (font), XtRString, XtDefaultFont },
    { XtNbackgroundColor, XtCBackground, XtRRenderColor, sizeof (XRenderColor),
      offset (background), XtRString, XtDefaultBackground },
    { XtNforegroundColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (foreground), XtRString, XtDefaultForeground },
    { XtNdpi, XtCDpi, XtRDpi, sizeof(double),
      offset (dpi), XtRString, "" },
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
    l->serial = 0;
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
	{
	    CopyText (&oldb[row], &b[row], 0, max_col);
	    b[row].serial = oldb[row].serial;
	    b[row].id = oldb[row].id;
	}
	for (row = 0; row < old_rows; row++)
	    DisposeText (&oldb[row]);
	Dispose (oldb);
    }
    *bp = b;
}

static void
ResizeText (PadWidget w, Dimension rows, Dimension cols)
{
    int	    row;

    ResizeBuffer (&w->pad.is, w->pad.rows, w->pad.cols, rows, cols);
    ResizeBuffer (&w->pad.want, w->pad.rows, w->pad.cols, rows, cols);
    w->pad.rows = rows;
    w->pad.cols = cols;
    for (row = 0; row < rows; row++)
	w->pad.is[row].id = w->pad.want[row].id = NextSerial (w);
    if (w->pad.resize_callbacks != NULL)
	XtCallCallbackList ((Widget) w, w->pad.resize_callbacks, (XtPointer) NULL);
}

static cairo_t *
get_cairo(PadWidget w)
{
    cairo_t *cr = XkwGetCairo((Widget) w);
    cairo_set_font_face(cr, w->pad.font.font_face);
    cairo_set_font_size(cr, w->pad.font.size * w->pad.dpi / 72.0);
    return cr;
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
    XkwSetSource(cr, bg);
    cairo_rectangle(cr, x, y - font_extents.ascent, x + text_extents.width, y + font_extents.descent);
    cairo_fill(cr);
    XkwSetSource(cr, fg);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
    if (save != '\0')
	text[len] = save;
    return text_extents.width;
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
   return text_extents.width;
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
    w->pad.char_height = font_extents.height;
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
    new->pad.is = 0;
    new->pad.want = 0;
    new->pad.serial = 0;
    new->pad.copy = 0;
    ResizeText (new, new->pad.rows, new->pad.cols);
}

static int
XToCol(PadWidget w, cairo_t *cr, int row, int x)
{
    char    *c;
    int	    col;

    c = w->pad.is[row].text;
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
    char	*is_a, *is_t;
    PadLinePtr	is;
    int		x, y;
    int		width;

    is = &w->pad.is[row];
    is_a = is->attr + start_col;
    is_t = is->text + start_col;
    change_col = start_col;
    x = TextX (w, start_col);
    y = TextY (w, row);
    while (start_col < end_col) {
	attr = *is_a;
	do {
	    ++is_a;
	    ++change_col;
	} while (change_col < end_col && *is_a == attr);
	XRenderColor *fg, *bg;
	if (attr & XkwPadBold)
	{
	    fg = &w->pad.bold;
	    bg = &w->pad.background;
	    if (attr & XkwPadInverse) {
		fg = &w->pad.background;
		bg = &w->pad.bold;
	    }
	}
	else
	{
	    fg = &w->pad.foreground;
	    bg = &w->pad.background;
	    if (attr & XkwPadInverse) {
		fg = &w->pad.background;
		bg = &w->pad.foreground;
	    }
	}

	width = show_text(cr, fg, bg, x, y, is_t, change_col - start_col);
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
	is_t += (change_col - start_col);
	start_col = change_col;
    }
}

static void
RedrawText (PadWidget w, cairo_t *cr, int row, int start_col, int end_col)
{
    char    *t, *a;

    t = w->pad.is[row].text + start_col;
    a = w->pad.is[row].attr + start_col;
    while (start_col < end_col && *t++ == ' ' && *a++ == XkwPadNormal)
	start_col++;
    t = w->pad.is[row].text + end_col;
    a = w->pad.is[row].attr + end_col;
    while (end_col > start_col && *--t == ' ' && *--a == XkwPadNormal)
	end_col--;
    if (start_col < end_col)
	DrawText (w, cr, row, start_col, end_col);
}

static int
UntilEqual(PadWidget w, int start)
{
    PadLinePtr	want = &w->pad.want[start],
		is = &w->pad.is[start];

    while (start < w->pad.rows && want->id != is->id) {
	want++;
	is++;
	start++;
    }

    return start;
}

static void
ClearLines (PadWidget w, cairo_t *cr, int start, int amt)
{
    XkwSetSource(cr, &w->pad.background);
    cairo_rectangle(cr, 0, YPos (w, start), w->core.width, amt * w->pad.char_height);
    cairo_fill(cr);
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

static Boolean
AddLines(PadWidget w, cairo_t *cr, int at, int num)
{
    int	bottom = UntilEqual(w, at + num);

    if (num == 0 || num >= ((bottom - 1) - at))
	return False;	/* We did nothing */

//    CopyLines (w, at, bottom, -num);
    ClearLines (w, cr, at, num);
    ScrollBuffer (w, w->pad.is, at, bottom, num);
    return True;	/* We did something. */
}

static Boolean
DelLines(PadWidget w, cairo_t *cr, int at, int num)
{
    int	bottom = UntilEqual(w, at + num);

    if (num == 0 || num >= ((bottom - 1) - at))
	return False;

    ClearLines (w, cr, bottom - num, num);
    ScrollBuffer (w, w->pad.is, at, bottom, -num);
    return True;
}

static void
DoInsertDelete (PadWidget w, cairo_t *cr, int start)
{
    PadLinePtr  is, want;
    int		i, j;

    /* Some changes have been made.  Try for insert or delete lines.
    If either case has happened, Addlines and/or DelLines will do
    necessary scrolling, also CONVERTING w->pad.is to account for the
    physical changes.  The comparison continues from where the
    insertion/deletion takes place; this doesn't happen very often,
    usually it happens with more than one window with the same
    buffer. */

    is = &w->pad.is[start];
    want = &w->pad.want[start];
    for (i = start; i < w->pad.rows; i++, is++, want++)
	if (want->id != is->id)
	    break;

    for (; i < w->pad.rows; i++) {
	for (j = i + 1; j < w->pad.rows; j++) {
	    want = &w->pad.want[j];
	    is = &w->pad.is[j];
	    if (want->id == is->id)
		break;
	    if (want->id == w->pad.is[i].id) {
		if (AddLines(w, cr, i, j - i)) {
		    DoInsertDelete(w, cr, j);
		    return;
		}
		break;
	    }
	    if ((want = &w->pad.want[i])->id == is->id) {
		if (DelLines(w, cr, i, j - i)) {
		    DoInsertDelete(w, cr, i);
		    return;
		}
		break;
	    }
	}
    }
}

/*
 * Redisplay -- repaint damaged areas on the screen
 *  This is complicated by the CopyArea calls which may have
 *  copied exposed regions on the screen
 */

static void
Redisplay (Widget gw, XEvent *event, Region region)
{
    PadWidget   w = (PadWidget) gw;
    int		start_row, end_row, row;
    int		start_col = 0, end_col = 0;
    PadCopyPtr  copy;
    unsigned long   expose_serial;
    Boolean	*repaint;
    int		amt;
    Boolean	*r;

    (void) region;
    if (!XtIsRealized (gw))
	return;
    if (event->type != NoExpose)
    {
	cairo_t *cr = get_cairo(w);
	/* Mark rows for redisplay */
	repaint = Some (Boolean, w->pad.rows);
	for (row = 0; row < w->pad.rows; row++) repaint[row] = False;
	start_row = RowPos (w, event->xexpose.y);
	if (start_row < 0)
	    start_row = 0;
	end_row = RowPos (w, event->xexpose.y + event->xexpose.height - 1);
	if (end_row >= w->pad.rows)
	    end_row = w->pad.rows - 1;
	for (row = start_row; row <= end_row; row++)
	    repaint[row] = True;

	/* Track effects of CopyArea calls on exposure events */
	copy = w->pad.copy;
	expose_serial = event->xexpose.serial;
	for (copy = w->pad.copy; copy; copy = copy->next)
	    if (copy->copy_serial > expose_serial)
		break;
	for (; copy; copy = copy->next)
	{
	    bcopy (repaint + copy->src, repaint + copy->dst, copy->amt * sizeof (Boolean));
	    r = repaint;
	    if (copy->src > copy->dst)
		r += copy->dst + copy->amt;
	    else
		r += copy->src;
	    amt = copy->amt;
	    while (amt--)
		*r++ = False;
	}

	/* repaint the resultant rows */
#if 0
	if (w->pad.fixed_width)
	{
	    start_col = ColPos (w, event->xexpose.x);
	    if (start_col < 0)
		start_col = 0;
	    end_col = ColPos (w, event->xexpose.x + event->xexpose.width - 1);
	    if (end_col >= w->pad.cols)
		end_col = w->pad.cols - 1;
	}
#endif
	for (row = 0; row < w->pad.rows; row++)
	{
	    if (!repaint[row])
		continue;
//	    if (!w->pad.fixed_width)
	    {
		start_col = XToCol (w, cr, row, event->xexpose.x);
		end_col = XToCol (w, cr, row, event->xexpose.x + event->xexpose.width - 1);
	    }
	    RedrawText (w, cr, row, start_col, end_col + 1);
	}
	Dispose (repaint);
	cairo_destroy(cr);
	if (event->xexpose.count != 0)
	    return;
    }
    if (event->type != Expose)
    {
	copy = w->pad.copy;
	if (!copy)
	    abort ();
	if (event->xexpose.serial != copy->copy_serial)
	    abort ();
	w->pad.copy = copy->next;
	Dispose (copy);
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
    PadWidget	w = (PadWidget) gw;
    int		row, start_col, end_col;
    int		cols;
    PadLinePtr	is, want;
    char	*is_t, *want_t, *is_a, *want_a;
    Boolean	DoneInsertDelete = False;

    if (!XtIsRealized (gw))
	return;
    cairo_t *cr = get_cairo(w);
    is = w->pad.is;
    want = w->pad.want;
    cols = w->pad.cols;
    for (row = 0; row < w->pad.rows; row++, is++, want++)
    {
	if (is->serial == want->serial)
	    continue;

	if (is->id != want->id && !DoneInsertDelete)
	{
	    DoInsertDelete (w, cr, row);
	    DoneInsertDelete = True;
	}
	/*
	 * This is simplistic -- painting a single range of
	 * text which covers all the changed characters
	 */

	/* search for start of mismatching characters */
	is_t = is->text;
	is_a = is->attr;

	want_t = want->text;
	want_a = want->attr;
	for (start_col = 0; start_col < cols; start_col++)
	    if (*is_t++ != *want_t++ || *is_a++ != *want_a++)
		break;

	/* search for end of mismatching characters */
	is_t = is->text + cols;
	is_a = is->attr + cols;

	want_t = want->text + cols;
	want_a = want->attr + cols;
	for (end_col = cols; end_col > start_col; end_col--)
	    if (*--is_t != *--want_t || *--is_a != *--want_a)
		break;

	/* paint the range of mismatching characters */
	if (start_col < end_col)
	{
	    CopyText (want, is, start_col, end_col - start_col);
//	    if (!w->pad.fixed_width)
	    {
		start_col = 0;
		end_col = w->pad.cols;
		XkwSetSource(cr, &w->pad.background);
		cairo_rectangle(cr, 0, YPos(w, row),
				w->core.width, w->pad.char_height);
		cairo_fill(cr);
	    }
	    DrawText (w, cr, row, start_col, end_col);
	}
	is->serial = want->serial;
	is->id = want->id;
    }
}

void
XkwPadText (Widget gw, int row, int col, char *text, int len)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    want;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    want = &w->pad.want[row];
    if (col + len > w->pad.cols)
	len = w->pad.cols - col;
    bcopy (text, want->text + col, len);
    want->serial = NextSerial(w);
}

void
XkwPadAttributes (Widget gw, int row, int col, char *attr, int len)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    want;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    want = &w->pad.want[row];
    if (col + len > w->pad.cols)
	len = w->pad.cols - col;
    bcopy (attr, want->attr + col, len);
    want->serial = NextSerial(w);
}

void
XkwPadTextAndAttributes (Widget gw, int row, int col, char *text, char *attr, int len)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    want;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    want = &w->pad.want[row];
    if (col + len > w->pad.cols)
	len = w->pad.cols - col;
    bcopy (text, want->text + col, len);
    bcopy (attr, want->attr + col, len);
    want->serial = NextSerial(w);
}

void
XkwPadClearToEnd (Widget gw, int row, int col)
{
    PadWidget	    w = (PadWidget) gw;
    PadLinePtr	    want;
    char	    *t, *a;

    if (row >= w->pad.rows || col >= w->pad.cols)
	return;
    want = &w->pad.want[row];
    t = want->text + col;
    a = want->attr + col;
    col = w->pad.cols - col;
    while (col--)
    {
	*t++ = ' ';
	*a++ = XkwPadNormal;
    }
    want->serial = NextSerial (w);
}

void
XkwPadClear (Widget gw)
{
    PadWidget	    w = (PadWidget) gw;
    int		    row;

    for (row = 0; row < w->pad.rows; row++)
	XkwPadClearToEnd (gw, row, 0);
}

void
XkwPadScroll (Widget gw, int start_row, int end_row, int dist)
{
    PadWidget	    w = (PadWidget) gw;

    ScrollBuffer (w, w->pad.want, start_row, end_row, dist);
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
    if (memcmp(&req->pad.foreground, &cur->pad.foreground, sizeof(XRenderColor)) != 0)
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

PadClassRec padClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
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
  { /* simple fields */
    /* empty			*/	0
  },
  { /* pad fields */
    /* empty			*/	0
  }
};

WidgetClass padWidgetClass = (WidgetClass)&padClassRec;
