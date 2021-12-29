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
	XtSetArg(arg[0], XtNinternalBorderWidth, 0);
        XtSetArg(arg[1], XtNwantForward, False);
        XtSetArg(arg[2], XtNborderWidth, 0);
	XtSetValues(drag, arg, 3);
	dragParent = parent;
	XtSetMappedWhenManaged(drag, FALSE);
    }
}

static void
Drag (Widget child, XEvent *event)
{
    Position	x = event->xmotion.x - off_x;
    Position	y = event->xmotion.y - off_y;
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
HandDrag(HandInputPtr input)
{
    switch (input->action) {
    case HandActionStart:
        start_x = input->event.xbutton.x;
        start_y = input->event.xbutton.y;
        if (input->card) {
            off_x = start_x - input->card->x;
            off_y = start_y - input->card->y;
        } else {
            off_x = 0;
            off_y = 0;
        }
        XkwTranslateCoordsPosition(dragParent, input->w, &start_x, &start_y);
	break;
    case HandActionDrag:
	if (!dragging) {
            /* Filter out drags that don't move far */

            Position x = input->event.xmotion.x;
            Position y = input->event.xmotion.y;
            XkwTranslateCoordsPosition(dragParent, input->w, &x, &y);
            if ((x - start_x) * (x - start_x) + (y - start_y) * (y - start_y) < 100)
                break;

            if (input->start.card && !HandCardIsEmpty(input->start.w, input->start.private)) {
                CardPtr c;
                HandWidget w = (HandWidget) input->start.w;
                int row = 0;
                int col = 0;
                Boolean first = TRUE;

                xkw_foreach_startat_rev(c, input->start.card, &w->hand.cards, list) {
                    if (w->hand.row_major) {
                        if (c->row != input->start.row ||
                            (!first && w->hand.col_offset >= w->hand.card_width))
                            break;
                        col = c->col - input->start.col;
                    } else {
                        if (c->col != input->start.col ||
                            (!first && w->hand.row_offset >= w->hand.card_height))
                            break;
                        row = c->row - input->start.row;
                    }
                    HandHideCard(input->start.w, c);
                    HandAddCard(drag, c->private, row, col, XkwHandDefaultOffset);
                    first = FALSE;
                }
                Arg args[2];
                XtSetArg(args[0], XtNcolOffset, w->hand.col_offset);
                XtSetArg(args[1], XtNrowOffset, w->hand.row_offset);
                XtSetValues(drag, args, 2);
                HandSetPreferredSize(drag);
                dragging = TRUE;
                Drag(input->w, &input->event);
                XtMapWidget(drag);
            }
	} else {
	    Drag(input->w, &input->event);
	}
	break;
    case HandActionStop:
        if (dragging) {
            dragging = FALSE;
            XtUnmapWidget(drag);
            HandRemoveAllCards(drag);
            HandShowAllCards(input->start.w);
        }
        break;
    case HandActionExpand:
        break;
    }
}
