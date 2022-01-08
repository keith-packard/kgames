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

# include	<X11/Intrinsic.h>
# include	<X11/StringDefs.h>
# include	<X11/Xos.h>
# include	<X11/Xutil.h>
# include       "HandP.h"
# include	<stdlib.h>

static Boolean  initialized;
static Boolean	dragging;
static Widget	drag;
static Widget	dragParent;
static Position start_x, start_y;
static Position off_x, off_y;

void
HandDragInit(Widget parent, WidgetClass class)
{
    if (!initialized) {
	Arg	arg[3];
        initialized = TRUE;
	drag = XtCreateManagedWidget("draghand", class, parent, NULL, 0);

        /* Cards widget smashes these values at initialize time, so
         * force them by using SetValues afterwards
         */
	XtSetArg(arg[0], XtNinternalBorderWidth, 0);
        XtSetArg(arg[1], XtNwantForward, False);
        XtSetArg(arg[2], XtNborderWidth, 0);
        XtSetValues(drag, arg, 3);
	dragParent = parent;
	XtSetMappedWhenManaged(drag, FALSE);
    }
}

static void
Drag (Widget child, Position x, Position y)
{
    XkwTranslateCoordsPosition(dragParent, child, &x, &y);

    XtWidgetGeometry request = {
	.x = x,
	.y = y,
	.stack_mode = Above,
	.request_mode = CWX | CWY | CWStackMode
    };

    XtMakeGeometryRequest(drag, &request, NULL);
}

void
HandDrag(HandInputPtr input, Action action)
{
    switch (action) {
    case ActionStart:
        start_x = input->start.x;
        start_y = input->start.y;
        if (input->start.card) {
            off_x = start_x - input->start.card->x;
            off_y = start_y - input->start.card->y;
        } else {
            off_x = 0;
            off_y = 0;
        }
        XkwTranslateCoordsPosition(dragParent, input->start.w, &start_x, &start_y);
	break;
    case ActionDrag:
	if (!dragging) {
            if (!input->start.card)
                break;

            if (HandCardIsEmpty(input->start.w, input->start.private))
                break;

            CardPtr c;
            HandWidget w = (HandWidget) input->start.w;
            int row = 0;
            int col = 0;
            int rows = 0;
            int cols = 0;
            Region region = NULL;

            xkw_foreach_startat_rev(c, input->start.card, &w->hand.cards, list) {
                Boolean pick = FALSE;
                switch(w->hand.select) {
                case HandSelectOverlap:
                    if (!region || HandCardInRegion(w, c, region)) {
                        XRectangle rect;

                        HandCardRectangle(w, c, &rect);
                        if (!region)
                            region = XCreateRegion();
                        XUnionRectWithRegion(&rect, region, region);

                        pick = TRUE;
                    }
                    break;
                case HandSelectOne:
                    if (c == input->start.card)
                        pick = TRUE;
                    break;
                case HandSelectAll:
                    pick = TRUE;
                    break;
                }
                if (!pick)
                    break;
                col = c->col - input->start.col;
                row = c->row - input->start.row;
                if (row >= rows)
                    rows = row + 1;
                if (col >= cols)
                    cols = col + 1;
                HandHideCard(input->start.w, c);
                HandAddCard(drag, c->private, row, col,
                            XkwHandDefaultOffset);
            }

            if (region)
                XDestroyRegion(region);

            Arg args[4];
            XtSetArg(args[0], XtNcolOffset, w->hand.col_offset);
            XtSetArg(args[1], XtNrowOffset, w->hand.row_offset);
            XtSetArg(args[2], XtNnumRows, rows);
            XtSetArg(args[3], XtNnumCols, cols);
            XtSetValues(drag, args, 4);
            HandSetPreferredSize(drag);
	}
        Drag(input->current.w, input->current.x - off_x, input->current.y - off_y);
        if (!dragging) {
            dragging = TRUE;
            XtMapWidget(drag);
        }
	break;
    case ActionStop:
        if (dragging) {
            dragging = FALSE;
            XtUnmapWidget(drag);
            HandRemoveAllCards(drag);
            HandShowAllCards(input->start.w);
        }
        break;
    case ActionExpand:
        break;
    }
}
