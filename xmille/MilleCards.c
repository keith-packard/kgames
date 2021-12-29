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

#include "mille.h"
#include "uiXt.h"
#include "card.h"
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Converters.h>
#include <Xkw/Xkw.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "cards-svg.h"
#include "MilleCardsP.h"

#define SuperClass  ((HandWidgetClass)&handClassRec)

static void
ClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

static void
DisplayCallback (Widget w, XtPointer closure, XtPointer data)
{
    HandDisplayPtr  display = (HandDisplayPtr) data;
    int		    card_no;
    struct card	    *card;
    cairo_t	    *cr = display->cr;

    cairo_scale(cr, scale, scale);
    card_no = (int) (intptr_t) display->private;
    if (card_no == -2)
	card = &deck;
    else if (card_no < 0 || NUM_CARDS <= card_no)
	card = &blank;
    else
	card = &svg_cards[card_no];

    XkwRsvgDraw(cr, WIDTH, HEIGHT, card->rsvg_handle);
}

static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    Arg my_args[2];
    int i = 0;

    (void) greq;
    (void) args;
    (void) count;

    XtSetArg (my_args[i], XtNcardWidth, WIDTH * scale); i++;
    XtSetArg (my_args[i], XtNcardHeight, HEIGHT * scale); i++;
    XtSetValues(gnew, my_args, i);
    XtAddCallback (gnew, XtNdisplayCallback, DisplayCallback, (XtPointer) gnew);
    HandDragInit(XtParent(gnew), milleCardsWidgetClass);
}

static Boolean
MilleCardsCardIsEmpty(Widget gw, XtPointer private)
{
    int card_no = (int) (intptr_t) private;

    if (card_no == -2)
        return FALSE;
    return card_no < 0 || NUM_CARDS <= card_no;
}

MilleCardsClassRec	milleCardsClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) SuperClass,
    /* class_name		*/	"MilleCards",
    /* widget_size		*/	sizeof(MilleCardsRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	NULL,
    /* num_resources		*/	0,
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	XtInheritResize,
    /* expose			*/	XtInheritExpose,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	NULL,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	XtInheritChangeSensitive,
  },
  {
    /* ksimple fields */
    /* empty			*/	0
  },
  { /* hand fields */
      .card_is_empty = MilleCardsCardIsEmpty,
  },
  { /* cards fields */
    /* ignore			*/	0
  },
};

WidgetClass milleCardsWidgetClass = (WidgetClass) &milleCardsClassRec;
