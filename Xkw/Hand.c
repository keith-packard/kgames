/* $XConsortium: Hand.c,v 1.18 91/07/26 15:21:52 keith Exp $ */
/*
 * Copyright 1991 Massachusetts Institute of Technology
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

/*
 * Hand - display collection of cards
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Converters.h>
#include <stdio.h>
#include <ctype.h>
#include <Xkw/HandP.h>

#define offset(field)	XtOffsetOf(HandRec, hand.field)

static XtResource resources[] = {
    {XtNcardWidth, XtCCardWidth, XtRDimension, sizeof (Dimension),
     offset(card_width), XtRImmediate, (XtPointer) 0 },
    {XtNcardHeight, XtCCardHeight, XtRDimension, sizeof (Dimension),
     offset(card_height), XtRImmediate, (XtPointer) 0 },
    {XtNdisplayWidth, XtCDisplayWidth, XtRDimension, sizeof (Dimension),
     offset(display_width), XtRImmediate, (XtPointer) 0 },
    {XtNdisplayHeight, XtCDisplayHeight, XtRDimension, sizeof (Dimension),
     offset(display_height), XtRImmediate, (XtPointer) 0 },
    {XtNdisplayX, XtCDisplayX, XtRDimension, sizeof (Dimension),
     offset(display_x), XtRImmediate, (XtPointer) 0 },
    {XtNdisplayY, XtCDisplayY, XtRDimension, sizeof (Dimension),
     offset(display_y), XtRImmediate, (XtPointer) 0 },
    {XtNinternalBorderWidth, XtCInternalBorderWidth, XtRDimension, sizeof (Dimension),
     offset(internal_border), XtRImmediate, (XtPointer) 0},
    {XtNnumRows, XtCNumRows, XtRDimension, sizeof (Dimension),
     offset(num_rows), XtRImmediate, (XtPointer) 1 },
    {XtNnumCols, XtCNumCols, XtRDimension, sizeof (Dimension),
     offset(num_cols), XtRImmediate, (XtPointer) 1 },
    {XtNrowsHint, XtCRowsHint, XtRBoolean, sizeof (Boolean),
     offset(rows_hint), XtRImmediate, (XtPointer) False},
    {XtNcolsHint, XtCColsHint, XtRBoolean, sizeof (Boolean),
     offset(cols_hint), XtRImmediate, (XtPointer) False},
    {XtNrowOffset, XtCRowOffset, XtRDimension, sizeof (Dimension),
     offset(row_offset), XtRImmediate, (XtPointer) 0 },
    {XtNcolOffset, XtCColOffset, XtRDimension, sizeof (Dimension),
     offset(col_offset), XtRImmediate, (XtPointer) 0 },
    {XtNrowMajor, XtCRowMajor, XtRBoolean, sizeof (Boolean),
     offset(row_major), XtRImmediate, (XtPointer) False },
    {XtNdisplayCallback, XtCDisplayCallback, XtRCallback, sizeof (XtPointer),
     offset(display_callback), XtRCallback, (XtPointer) NULL},
    {XtNinputCallback, XtCInputCallback, XtRCallback, sizeof (XtPointer),
     offset(input_callback), XtRCallback, (XtPointer) NULL},
    {XtNrowInsert, XtCInsert, XtRBoolean, sizeof (Boolean),
     offset(row_insert), XtRImmediate, (XtPointer) False},
    {XtNcolInsert, XtCInsert, XtRBoolean, sizeof (Boolean),
     offset(col_insert), XtRImmediate, (XtPointer) False},
    {XtNimmediateUpdate, XtCImmediateUpdate, XtRBoolean, sizeof (Boolean),
     offset(immediate_update), XtRImmediate, (XtPointer) True},
};

#undef offset

static char defaultTranslations[] =  "<BtnDown>: select()";

#define SuperClass  ((KSimpleWidgetClass)&ksimpleClassRec)

static int
handWidth (HandWidget w, int num_cols)
{
    return w->hand.card_width + 2 * w->hand.internal_border +
	   (num_cols - 1) * w->hand.col_offset;
}

static int
handHeight (HandWidget w, int num_rows)
{
    return w->hand.card_height + 2 * w->hand.internal_border +
	   (num_rows - 1) * w->hand.row_offset;
}

static void
getHandSize (HandWidget w, Dimension *widthp, Dimension *heightp)
{
    *widthp = handWidth (w, w->hand.num_cols);
    *heightp = handHeight (w, w->hand.num_rows);
}

static int
BestOffset (int want, int have, int desired, int item_size, int num_items)
{
    int	val, min;
    int	last_showing;

    if (want <= have || num_items <= 1)
	return desired;
    if (have < 0)
	have = 0;
    last_showing = item_size;
    min = item_size / 4;
    do {
	last_showing /= 2;
	val = (have - last_showing) / (num_items - 1);
    } while (val < min && val < last_showing);
    if (val > desired)
	val = desired;
    return val;
}

static int
BestColOffset (HandWidget w, int num_cols)
{
    int	borders = 2 * w->hand.internal_border;

    return BestOffset (handWidth (w, num_cols) - borders,
		       w->core.width - borders,
		       w->hand.col_offset, w->hand.card_width,
		       num_cols);
}

static int
BestRowOffset (HandWidget w, int num_rows)
{
    int	borders = 2 * w->hand.internal_border;

    return BestOffset (handHeight (w, num_rows) - borders,
		       w->core.height - borders,
		       w->hand.row_offset, w->hand.card_height,
		       num_rows);
}

static int
ColsInRow (HandWidget w, int row)
{
    int	    maxCol = -1;
    CardPtr c;
    for (c = w->hand.bottomCard; c; c = c->prev)
	if (c->row == row && c->col > maxCol)
	    maxCol = c->col;
    return maxCol;
}

static int
RowsInCol (HandWidget w, int col)
{
    int	maxRow = -1;
    CardPtr c;
    for (c = w->hand.bottomCard; c; c = c->prev)
	if (c->col == col && c->row > maxRow)
	    maxRow = c->row;
    return maxRow;
}

static int
XPos (HandWidget w, int row, int col)
{
    int	offset;
    CardPtr c;
    int	numCol;
    int numDefault;
    int	defaultOffset;
    int	lastCol;

    if (w->hand.row_major)
    {
	numCol = ColsInRow (w, row) + 1;
	if (w->hand.cols_hint)
	    defaultOffset = BestColOffset (w, numCol);
	else
	    defaultOffset = w->hand.real_col_offset;
	lastCol = 0;
	offset = 0;
	numDefault = 0;
	for (c = w->hand.bottomCard; c; c = c->prev)
	{
	    if (c->row == row && c->col >= lastCol && c->col < col)
	    {
		numDefault += (c->col - lastCol);
		if (c->offset == XkwHandDefaultOffset ||
		    c->offset > defaultOffset)
		    numDefault++;
		else
		    offset += c->offset;
		lastCol = c->col + 1;
	    }
	}
	if (col > lastCol)
	    numDefault += (col - lastCol);
	if (numDefault)
	    offset += numDefault * defaultOffset;
    }
    else
	offset = col * w->hand.real_col_offset;
    return offset + w->hand.internal_border;
}

static int
ColFromX (HandWidget w, int x, int row)
{
    int	offset;
    int	adjust;
    int	x1, x2;
    int	col;

    if (w->hand.row_major)
    {
	x2 = w->hand.internal_border;
	for (col = 1; ; col++)
	{
	    x1 = x2;
	    x2 = XPos (w, row, col);
	    adjust = 0;
	    if (x2 - x1 > w->hand.card_width)
		adjust = ((x2 - x1) - w->hand.card_width) / 2;
	    if (x < x2 - adjust)
		return col - 1;
	}
    }
    else
    {
	offset = w->hand.real_col_offset;
	adjust = -w->hand.internal_border;
	if (offset > w->hand.card_width)
	    adjust += (offset - w->hand.card_width) / 2;
	return (x + adjust) / offset;
    }
}

static int
YPos (HandWidget w, int row, int col)
{
    CardPtr c;
    int	    offset;
    int	    numRow;
    int	    numDefault;
    int	    defaultOffset;
    int	    lastRow;

    if (!w->hand.row_major)
    {
	numRow = RowsInCol (w, col) + 1;
	if (w->hand.rows_hint)
	    defaultOffset = BestRowOffset (w, numRow);
	else
	    defaultOffset = w->hand.real_row_offset;
	lastRow = 0;
	offset = 0;
	numDefault = 0;
	for (c = w->hand.bottomCard; c; c = c->prev)
	{
	    if (c->col == col && c->row >= lastRow && c->row < row)
	    {
		numDefault += (c->row - lastRow);
		if (c->offset == XkwHandDefaultOffset ||
		    c->offset > defaultOffset)
		    numDefault++;
		else
		    offset += c->offset;
		lastRow = c->row + 1;
	    }
	}
	if (row > lastRow)
	    numDefault += (row - lastRow);
	if (numDefault)
	    offset += numDefault * defaultOffset;
    }
    else
	offset = row * w->hand.real_row_offset;
    return offset + w->hand.internal_border;
}

static int
RowFromY (HandWidget w, int y, int col)
{
    int	offset;
    int	adjust;
    int	y1, y2;
    int	row;

    if (!w->hand.row_major)
    {
	y2 = w->hand.internal_border;
	for (row = 1; ; row++)
	{
	    y1 = y2;
	    y2 = YPos (w, row, col);
	    adjust = 0;
	    if (y2 - y1 > w->hand.card_height)
		adjust = ((y2 - y1) - w->hand.card_height) / 2;
	    if (y < y2 - adjust)
		return row - 1;
	}
    }
    else
    {
	offset = w->hand.real_row_offset;
	adjust = -w->hand.internal_border;
	if (offset > w->hand.card_height)
	    adjust += (offset - w->hand.card_height) / 2;
	return (y + adjust) / offset;
    }
}

static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    HandWidget	req = (HandWidget) greq,
		new = (HandWidget) gnew;

    (void) args;
    (void) count;
    getHandSize (req, &new->core.width, &new->core.height);
    new->hand.real_row_offset = new->hand.row_offset;
    new->hand.real_col_offset = new->hand.col_offset;
    new->hand.topCard = NULL;
    new->hand.bottomCard = NULL;
    new->hand.damage = NULL;
}

#define MotionMask ( \
	PointerMotionMask | Button1MotionMask | \
	Button2MotionMask | Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask )
#define PointerGrabMask ( \
	ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask | \
	PointerMotionHintMask | KeymapStateMask | \
	MotionMask )

static void
Realize (Widget widget, XtValueMask *value_mask, XSetWindowAttributes *attributes)
{
    unsigned int    event_mask = 0;
#define MAX_BUT	256
    unsigned char   mapping[MAX_BUT];
    int	    i, max;

    *value_mask |= CWBitGravity;
    attributes->bit_gravity = NorthWestGravity;

    (*SuperClass->core_class.realize)(widget, value_mask, attributes);
    if (*value_mask & CWEventMask)
	event_mask = attributes->event_mask;
    event_mask &= PointerGrabMask;
    if (event_mask & ButtonPressMask)
    {
	max = XGetPointerMapping (XtDisplay (widget), mapping, MAX_BUT);
	for (i = 0; i < max; i++)
	{
	    if (mapping[i] != 0)
		XtGrabButton (widget, i, AnyModifier, True, event_mask,
			      GrabModeAsync, GrabModeAsync, None, None);
	}
    }
}

static void
Destroy (Widget gw)
{
    HandWidget	w = (HandWidget) gw;
    CardPtr	c, n;

    for (c = w->hand.topCard; c; c = n) {
	n = c->next;
	Dispose (c);
    }
}

static XtGeometryResult
QueryGeometry (Widget gw, XtWidgetGeometry *intended, XtWidgetGeometry *prefered)
{
    HandWidget	w = (HandWidget) gw;
    Dimension	width, height;
    XtGeometryResult	result;

    getHandSize (w, &width, &height);
    result = XtGeometryYes;
    prefered->request_mode = 0;
    if (intended->request_mode & CWWidth) {
	if (intended->width != width) {
	    if (width == w->core.width) {
		result = XtGeometryNo;
	    } else if (result != XtGeometryNo) {
	    	prefered->request_mode |= CWWidth;
	    	prefered->width = width;
	    	result = XtGeometryAlmost;
	    }
	}
    }
    if (intended->request_mode & CWHeight) {
	if (intended->height != height) {
	    if (height == w->core.height) {
		result = XtGeometryNo;
	    } else if (result != XtGeometryNo) {
	    	prefered->request_mode |= CWHeight;
	    	prefered->height = height;
	    	result = XtGeometryAlmost;
	    }
	}
    }
    return result;
}

static Bool XYInCard (HandWidget w, CardPtr c, int x, int y)
{
    return c->x <= x && x < c->x + w->hand.card_width &&
	   c->y <= y && y < c->y + w->hand.card_height;
}

static CardPtr XYToCard (HandWidget w, int x, int y)
{
    CardPtr c;

    for (c = w->hand.topCard; c; c = c->next) {
	if (XYInCard (w, c, x, y))
	    return c;
    }
    return NULL;
}

static void ActionSelect (Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    HandWidget	    w = (HandWidget) gw;
    CardPtr	    c;
    HandInputRec    input;
    int		    row, col;
    XtPointer	    private;

    if (event->type == KeyPress && *num_params > 1)
    {
	if (w->hand.row_major)
	{
	    row = atoi (params[1]);
	    col = w->hand.num_cols - 1;
	}
	else
	{
	    col = atoi (params[1]);
	    row = w->hand.num_rows - 1;
	}
	private = 0;
    }
    else
    {
	c = XYToCard (w, event->xbutton.x, event->xbutton.y);
	if (c)
	{
	    row = c->row;
	    col = c->col;
	    private = c->private;
	}
	else
	{
	    if (w->hand.row_major)
	    {
		row = RowFromY (w, event->xbutton.y, -1);
		col = ColFromX (w, event->xbutton.x, row);
	    }
	    else
	    {
		col = ColFromX (w, event->xbutton.x, -1);
		row = RowFromY (w, event->xbutton.y, col);
	    }
	    if (row < 0)
		row = 0;
	    else if (w->hand.row_major && w->hand.num_rows <= row)
		row = w->hand.num_rows - 1;
	    if (col < 0)
		col = 0;
	    else if (!w->hand.row_major && w->hand.num_cols <= col)
		col = w->hand.num_cols - 1;
	    private = 0;
	}
    }
    input.w = gw;
    input.row = row;
    input.col = col;
    input.private = private;
    input.params = params;
    input.event = *event;
    input.num_params = num_params;
    XtCallCallbackList ((Widget) w, w->hand.input_callback, (XtPointer) &input);
}

static void
CardRectangle(HandWidget w, CardPtr card, XRectangle *c)
{
    c->x = card->x;
    c->y = card->y;
    c->width = w->hand.card_width;
    c->height = w->hand.card_height;
}

static void
DamageCard(HandWidget w, CardPtr card)
{
    XRectangle c;

    if (card->shown) {
	CardRectangle(w, card, &c);
	if (!w->hand.damage)
	    w->hand.damage = XCreateRegion();
	XUnionRectWithRegion(&c, w->hand.damage, w->hand.damage);
    }
}

static Bool
CardInRegion (HandWidget w, CardPtr card, Region region)
{
    if (region == NULL)
	return True;

    XRectangle c;
    CardRectangle(w, card, &c);
    return XRectInRegion(region, c.x, c.y, c.width, c.height);
}

/*
 * widget was resized, adjust row and column offsets.  The expose
 * event will cause the card positions to get recomputed which will
 * then cause them all to be redrawn
 */

static void
Resize (Widget gw)
{
    HandWidget	w = (HandWidget) gw;
    Dimension col_offset, row_offset;

    if (*SuperClass->core_class.resize != NULL)
	(*SuperClass->core_class.resize)(gw);

    col_offset = BestColOffset (w, w->hand.num_cols);
    row_offset = BestRowOffset (w, w->hand.num_rows);
    if (col_offset != w->hand.real_col_offset || row_offset != w->hand.real_row_offset)
    {
	w->hand.real_col_offset = col_offset;
	w->hand.real_row_offset = row_offset;
	if (XtIsRealized(gw))
	    XClearArea(XtDisplay(gw), XtWindow(gw), 0, 0, 0, 0, True);
    }
}

static void
UpdateCards (HandWidget w)
{
    CardPtr c;
    for (c = w->hand.bottomCard; c; c = c->prev) {
	int x = XPos(w, c->row, c->col);
	int y = YPos(w, c->row, c->col);

	if (!c->shown || x != c->x || y != c->y) {
	    if (c->shown)
		DamageCard(w, c);
	    c->x = x;
	    c->y = y;
	    c->shown = True;
	    DamageCard(w, c);
	}
    }
}

static void
Paint (HandWidget w, Region region)
{
    if (XtIsRealized((Widget) w)) {
	cairo_t 	*cr = XkwDrawBegin ((Widget) w, region);
	HandDisplayRec	display;
	CardPtr		c;

	display.w = (Widget) w;
	display.cr = cr;
	/* redisplay cards */
	for (c = w->hand.bottomCard; c; c = c->prev) {
	    if (CardInRegion(w, c, region)) {
		cairo_save(cr);
		cairo_translate(cr, c->x, c->y);
		display.private = c->private;
		XtCallCallbackList ((Widget) w, w->hand.display_callback, (XtPointer) &display);
		cairo_restore(cr);
	    }
	}
	XkwDrawEnd((Widget) w, region, cr);
    }
    if (w->hand.damage) {
	XDestroyRegion(w->hand.damage);
	w->hand.damage = NULL;
    }
}

/*
 * Redisplay function.  Queue the rectangle as an erased area
 * and redisplay when we've gotten all of the events
 */
static void
Redisplay (Widget gw, XEvent *event, Region region)
{
    HandWidget	    w = (HandWidget) gw;

    if (!XtIsRealized (gw))
	return;

    UpdateCards(w);
    if (region && w->hand.damage) {
	XUnionRegion(region, w->hand.damage, w->hand.damage);
	region = w->hand.damage;
    }
    Paint (w, region);
}

/*
 * When values are set which change the desired size, ask for
 * the new size
 */

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    HandWidget	cur = (HandWidget) gcur,
		req = (HandWidget) greq,
 		new = (HandWidget) gnew;
    Dimension	curwidth, curheight, reqwidth, reqheight;
    Dimension	width, height;

    (void) args;
    (void) count;
    (void) new;
    getHandSize (cur, &curwidth, &curheight);
    getHandSize (req, &reqwidth, &reqheight);
    if (curwidth != reqwidth || curheight != reqheight)
    {
	XtMakeResizeRequest (gnew, reqwidth, reqheight, &width, &height);
	if (width != curwidth || height != curheight)
	    Resize (gnew);
    }
    return TRUE;
}

void
HandUpdateDisplay (Widget gw)
{
    HandWidget w = (HandWidget) gw;

    UpdateCards (w);
    if (w->hand.damage)
	Paint(w, w->hand.damage);
}

/* Insert a card */
XtPointer
HandAddCard (Widget	gw,
	     XtPointer	private,
	     int	row,
	     int	col,
	     int	offset)
{
    HandWidget	    w = (HandWidget) gw;
    CardPtr	    c, sib;
    int		    maxPos;

    /* compute values for row/col if requested */
    if (row == InsertRow)
    {
	maxPos = -1;
	for (c = w->hand.topCard; c; c = c->next)
	{
	    if (w->hand.row_major)
	    {
		if (c->row > maxPos)
		    maxPos = c->row;
	    }
	    else
	    {
		if (c->col == col && c->row > maxPos)
		    maxPos = c->row;
	    }
	}
	row = maxPos + 1;
    }
    if (col == InsertCol)
    {
	maxPos = -1;
	for (c = w->hand.topCard; c; c = c->next)
	{
	    if (!w->hand.row_major)
	    {
		if (c->col > maxPos)
		    maxPos = c->col;
	    }
	    else
	    {
		if (c->row == row && c->col > maxPos)
		    maxPos = c->col;
	    }
	}
	col = maxPos + 1;
    }
    /* find the card below this one */
    sib = NULL;
    for (c = w->hand.bottomCard; c; c = c->prev)
    {
	if (w->hand.row_major)
	{
	    if ((c->row == row && c->col <= col) || c->row < row)
		sib = c;
	    else
		break;
	}
	else
	{
	    if ((c->col == col && c->row <= row) || c->col < col)
		sib = c;
	    else
		break;
	}
    }
    c = New(CardRec);
    c->private = private;
    c->row = row;
    c->col = col;
    c->offset = offset;
    c->shown = False;
    /* insert the new card on the list */
    c->next = sib;	/* c is above sib */

    if (sib)
	c->prev = sib->prev;
    else
	c->prev = w->hand.bottomCard;

    if (c->next)
	c->next->prev = c;
    else
	w->hand.bottomCard = c;

    if (c->prev)
	c->prev->next = c;
    else
	w->hand.topCard = c;

    /* adjust higher cards rows */
    if (w->hand.row_insert)
    {
	for (sib = c->prev; sib; sib = sib->prev)
	    if (sib->col == c->col && sib->row == row)
		sib->row = ++row;
    }
    /* adjust higher cards columns */
    if (w->hand.col_insert)
    {
	for (sib = c->prev; sib; sib = sib->prev)
	    if (sib->row == c->row && sib->col == col)
		sib->col = ++col;
    }
    if (XtIsRealized(gw) && w->hand.immediate_update)
	HandUpdateDisplay (gw);
    return (XtPointer) c;
}

/* remove a card */
void
HandRemoveCard (Widget gw, XtPointer card)
{
    HandWidget	    w = (HandWidget) gw;
    CardPtr	    c, sib;
    int		    row, col;

    for (c = w->hand.topCard; c; c = c->next)
	if (c == (CardPtr) card)
	    break;
    if (!c)
	return;
    if (w->hand.row_insert)
    {
	row = c->row;
	for (sib = c->prev; sib; sib = sib->prev)
	    if (sib->col == c->col && sib->row == row + 1)
		sib->row = row++;
    }
    if (w->hand.col_insert)
    {
	col = c->col;
	for (sib = c->prev; sib; sib = sib->prev)
	    if (sib->row == c->row && sib->col == col + 1)
		sib->col = col++;
    }
    if (c->prev)
	c->prev->next = c->next;
    else
	w->hand.topCard = c->next;
    if (c->next)
	c->next->prev = c->prev;
    else
	w->hand.bottomCard = c->prev;
    DamageCard(w, c);
    Dispose (c);
    if (XtIsRealized (gw) && w->hand.immediate_update)
	HandUpdateDisplay (gw);
}

/* change the private value for a card and force a redisplay */
void
HandReplaceCard (Widget gw, XtPointer card, XtPointer private, int offset)
{
    HandWidget	    w = (HandWidget) gw;
    CardPtr c;
    for (c = w->hand.topCard; c; c = c->next)
	if (c == (CardPtr) card)
	    break;
    if (!c)
	return;
    c->private = private;
    c->offset = offset;
    DamageCard(w, c);
    if (XtIsRealized (gw) && w->hand.immediate_update)
    	HandUpdateDisplay (gw);
}

void
HandRectangleForPos (Widget gw, int row, int col, XRectangle *r)
{
    HandWidget	    w = (HandWidget) gw;

    r->x = XPos (w, row, col);
    r->y = YPos (w, row, col);
    r->width = w->hand.card_width;
    r->height = w->hand.card_height;
}

#if 0
static void
HandRectangleForCard (Widget gw, XtPointer card, XRectangle *r)
{
    HandWidget	    w = (HandWidget) gw;
    CardPtr	c;

    for (c = w->hand.topCard; c; c = c->next)
	if (c == (CardPtr) card)
	    break;
    if (!c)
    {
	r->x = 0;
	r->y = 0;
	r->width = 0;
	r->height = 0;
    }
    else
	HandRectangleForPos (gw, c->row, c->col, r);
}

static Boolean
HandXYToPos (Widget gw, int x, int y, int *rowp, int *colp)
{
    HandWidget	w = (HandWidget) gw;

    CardPtr	c;
    c = XYToCard (w, x, y);
    if (c)
    {
	*rowp = c->row;
	*colp = c->col;
	return True;
    }
    return False;
}
#endif

void
HandRemoveAllCards (Widget gw)
{
    HandWidget	    w = (HandWidget) gw;
    CardPtr	    c, n;

    for (c = w->hand.topCard; c; c = n) {
	n = c->next;
	free ((char *) c);
    }
    w->hand.topCard = 0;
    w->hand.bottomCard = 0;
    if (XtIsRealized (gw))
	Redisplay(gw, NULL, NULL);
}

static XtActionsRec actions[] = {
    { "select", ActionSelect },		    /* select card */
};

HandClassRec handClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) SuperClass,
    /* class_name		*/	"Hand",
    /* widget_size		*/	sizeof(HandRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	NULL,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultTranslations,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
  },
  {
    /* ksimple fields */
    /* empty			*/	0
  },
  { /* hand fields */
    /* ignore                   */	0
  }
};

WidgetClass handWidgetClass = (WidgetClass) &handClassRec;
