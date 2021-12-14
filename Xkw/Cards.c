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
#include <X11/Xmu/Converters.h>
#include <Xkw/Xkw.h>
#include <cairo/cairo-xlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "CardsP.h"
#include "cards-svg.h"

#define offset(field)	XtOffsetOf(CardsRec, cards.field)

#define COLOR_UNSET 10

static XtResource resources[] = {
    {XtNcardWidth, XtCCardWidth, XtRDimension, sizeof(Dimension),
     offset(card_width), XtRImmediate, (XtPointer) 0},
    {XtNoverlap, XtCOverlap, XtRCardsOverlap, sizeof (CardsOverlap),
     offset(overlap), XtRImmediate, (XtPointer) CardsOverlapNeither},
    {XtNback, XtCBack, XtRBitmap, sizeof (Pixmap),
     offset(back), XtRString, (XtPointer) NULL },
    {XtNobverseColor, XtNobverseColor, XtRPixel, sizeof (Pixel),
     offset(obverse_color), XtRString, (XtPointer) "white"},
    {XtNinverseColor, XtCInverseColor, XtRPixel, sizeof (Pixel),
     offset(inverse_color), XtRString, (XtPointer) "Sea Green"},
    {XtNcolor, XtCColor, XtRBoolean, sizeof (Boolean),
     offset(color), XtRImmediate, (XtPointer) COLOR_UNSET },
};

#undef offset
#undef hoffset

#include	<X11/Xmu/Drawing.h>

#define	ROUND_W	7
#define	ROUND_H	7
#define INSET 1

#include	"playing_card"

#define NUM_RANKS   13
#define NUM_SUITS   4
#define CARDS_PER_DECK 52

#include	<X11/xpm.h>

#include	"cards-svg.h"

static Boolean	any_svg, made_svg;

static RsvgHandle	*svg_map[CARDS_PER_DECK];

#define SuperClass  ((HandWidgetClass)&handClassRec)

static void
DisplayCallback (Widget gw, XtPointer closure, XtPointer data);

static Status
GetSize (Display *dpy, Pixmap p, unsigned int *widthp, unsigned int *heightp)
{
    int x, y;
    unsigned int borderWidth, depth;
    Window root;

    return XGetGeometry (dpy, p, &root, &x, &y, widthp, heightp, &borderWidth, &depth);
}

static inline int CARD_WIDTH(CardsWidget w)
{
    return w->cards.card_width;
}

static inline int CARD_HEIGHT(CardsWidget w)
{
    return (int) (w->cards.card_width * svg_card_height / svg_card_width);
}

#define ScreenNo(w) XScreenNumberOfScreen (XtScreen (w))

static void make_card_maps (CardsWidget w);

static void
setSizeVars (CardsWidget req, CardsWidget new)
{
    Arg	args[10];
    int	i = 0;
    int	col_offset, row_offset;
    int	display_x, display_y, display_width, display_height;
    Boolean row_major;
    Boolean row_major_set;

    (void) req;
    /* Default card size is based on the DPI value */
    if (new->cards.card_width == 0)
	new->cards.card_width = 100 * new->ksimple.dpi / 72;

    int	width = CARD_WIDTH(new);
    int	height = CARD_HEIGHT(new);

    XtSetArg (args[i], XtNcardWidth, width); i++;
    XtSetArg (args[i], XtNcardHeight, height); i++;
    display_x = 0;
    display_y = 0;
    display_width = width;
    display_height = height;
    new->cards.offset_other = XkwHandDefaultOffset;
    row_major_set = False;
    if (new->cards.overlap & CardsOverlapHorizontal)
    {
	row_major = True;
	row_major_set = True;
	display_x = ROUND_W;
	display_width -= display_x * 2;
	col_offset = CARD_WIDTH(new) / 5;
	new->cards.offset_other = col_offset / 2;
	display_x = CARD_WIDTH(new) / 4;
	display_width = width - display_x * 2;
    }
    else
    {
	display_y = CARD_WIDTH(new) / 5;
	display_height = height - display_y * 2;
	col_offset = width + width / 10;
    }
    if (new->cards.overlap & CardsOverlapVertical)
    {
	if (!row_major_set)
	{
	    row_major = False;
	    row_major_set = True;
	}
	row_offset = CARD_HEIGHT(new) / 5;
	if (!row_major)
	    new->cards.offset_other = row_offset / 2;
    }
    else
	row_offset = height + width / 10;
    XtSetArg (args[i], XtNcolOffset, col_offset); i++;
    XtSetArg (args[i], XtNrowOffset, row_offset); i++;
    XtSetArg (args[i], XtNinternalBorderWidth, width / 20); i++;
    XtSetArg (args[i], XtNdisplayX, display_x); i++;
    XtSetArg (args[i], XtNdisplayY, display_y); i++;
    XtSetArg (args[i], XtNdisplayWidth, display_width); i++;
    XtSetArg (args[i], XtNdisplayHeight, display_height); i++;
    if (row_major_set)
    {
	XtSetArg (args[i], XtNrowMajor, row_major); i++;
    }
    XtSetValues ((Widget) new, args, i);
}

static Boolean
CvtStringToCardsOverlap (Display *dpy,
			 XrmValue *args,
			 Cardinal *num_args,
			 XrmValue *from,
			 XrmValue *to,
			 XtPointer *converter_data)
{
    char    *s = (char *) from->addr;
    CardsOverlap    *result = (CardsOverlap *) to->addr;

    (void) dpy;
    (void) args;
    (void) num_args;
    (void) converter_data;
    if (!strcmp (s, "neither"))
	*result = CardsOverlapNeither;
    else if (!strcmp (s, "vertical"))
	*result = CardsOverlapVertical;
    else if (!strcmp (s, "horizontal"))
	*result = CardsOverlapHorizontal;
    else if (!strcmp (s, "both"))
	*result = CardsOverlapBoth;
    else
	return FALSE;
    return TRUE;
}

static void
ClassInitialize(void)
{
    XkwInitializeWidgetSet();
    XtSetTypeConverter ( XtRString, XtRCardsOverlap, CvtStringToCardsOverlap,
		    NULL, (Cardinal)0, XtCacheNone,
 		    (XtDestructor)NULL );
}

#define CardOffset(w,card)  ((card)->suit > CardsSpade ? (w)->cards.offset_other : XkwHandDefaultOffset)

XtPointer
CardsAddCard (gw, card, row, col)
    Widget	    gw;
    CardsCardPtr    card;
    int		    row, col;
{
    CardsWidget	w = (CardsWidget) gw;
    return HandAddCard (gw, (XtPointer) card, row, col, CardOffset(w,card));
}

void
CardsReplaceCard (gw, data, card)
    Widget	    gw;
    XtPointer	    data;
    CardsCardPtr    card;
{
    CardsWidget	w = (CardsWidget) gw;
    HandReplaceCard (gw, data, (XtPointer) card, CardOffset(w,card));
}

#define UsualSuspects(w)	Display *dpy = XtDisplay ((Widget) w); \
				Window	window = XtWindow ((Widget) w)

#define UsualDpy(w)		Display *dpy = XtDisplay ((Widget) w)

#define GetScreen(w)		int screen = ScreenNo(w)

#define SetClip(dpy,gc,clip,has)    if (((clip) != NULL) != *(has)) {	\
    if ((*(has) = ((clip) != NULL)))					\
	XSetClipRectangles(dpy, gc, 0, 0,				\
			   clip, 1, YXBanded);				\
    else								\
	XSetClipMask(dpy,gc,None);					\
    }

static void
InputCallback (Widget w, XtPointer closure, XtPointer data)
{
    CardDrag((HandInputPtr) data);
}

static void
Initialize (Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    CardsWidget	req = (CardsWidget) greq,
		new = (CardsWidget) gnew;

    (void) args;
    (void) count;

    CardDragInit(XtParent(gnew));

    new->hand.force_erase = True;

    setSizeVars (req, new);

    XtAddCallback (gnew, XtNdisplayCallback, DisplayCallback, (XtPointer) gnew);
    XtAddCallback (gnew, XtNinputCallback, InputCallback, (XtPointer) gnew);

    /* back surface */
    Pixmap back = new->cards.back;
    unsigned int width, height;

    UsualDpy(new);
    GetScreen(new);

    if (back) {
	GetSize (dpy, back, &width, &height);
    } else {
	back = XCreatePixmapFromBitmapData (dpy, RootWindow (dpy, screen),
					    playing_card_bits, playing_card_width, playing_card_height,
					    new->cards.inverse_color, new->cards.obverse_color,
					    DefaultDepth(dpy, screen));
	width = playing_card_width;
	height = playing_card_height;
    }

    cairo_surface_t *back_surface = cairo_xlib_surface_create (dpy,
							       back,
							       XtScreen(new)->root_visual,
							       width,
							       height);

    new->cards.back_pattern = cairo_pattern_create_for_surface(back_surface);

    cairo_surface_destroy(back_surface);

    cairo_pattern_set_extend(new->cards.back_pattern, CAIRO_EXTEND_REPEAT);

    new->cards.back_delta_x = (CARD_WIDTH(new) - width) / 2;
    new->cards.back_delta_y = (CARD_HEIGHT(new) - height) / 2;
}

static void
Destroy (Widget gw)
{
    CardsWidget	w = (CardsWidget) gw;

    cairo_pattern_destroy(w->cards.back_pattern);
}

static Boolean
SetValues (Widget gcur, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    CardsWidget	cur = (CardsWidget) gcur,
		req = (CardsWidget) greq,
 		new = (CardsWidget) gnew;

    (void) args;
    (void) count;

    new->hand.force_erase = False;
    any_svg = True;
    new->hand.force_erase = True;

    make_card_maps(cur);

    if (req->cards.overlap != cur->cards.overlap)
    {
	setSizeVars (req, new);
	return TRUE;
    }
    return TRUE;
}

static void
make_card_maps(CardsWidget w)
{
    int	i;

    if (any_svg && !made_svg)
    {
	made_svg = True;
	for (i = 0; i < CARDS_PER_DECK; i++)
	    svg_map[i] = XkwRsvgCreate(svg_cards[i]);
    }
}

static void
paint_card(CardsWidget w, cairo_t *cr, CardsRank rank, CardsSuit suit)
{
    int card_number = 0;

    switch (suit)
    {
    case CardsSpade:
	card_number = 3 * NUM_RANKS;
	break;
    case CardsHeart:
	card_number = 2 * NUM_RANKS;
	break;
    case CardsDiamond:
	card_number = NUM_RANKS;
	break;
    case CardsClub:
	card_number = 0;
	break;
    default:
	break;
    }
    card_number += CardsRankToInt (rank);

    RsvgHandle *rsvg_handle = svg_map[card_number];
    if (rsvg_handle) {
	XkwRsvgDraw(cr, CARD_WIDTH(w), CARD_HEIGHT(w), rsvg_handle);
    }
}

static void
FillCard (CardsWidget w, cairo_t *cr)
{
    double	width, height;
    double	lw;
    double	radius;

    width = CARD_WIDTH(w);
    height = CARD_HEIGHT(w);
    radius = width / 20.0;
    lw = width / 200.0;

    cairo_save(cr);
    cairo_translate(cr, lw/2.0, lw/2.0);
    XkwDrawRoundedRect(cr, width - lw, height - lw, radius);
    cairo_fill_preserve(cr);
    XkwSetSource(cr, &w->ksimple.foreground);
    cairo_set_line_width(cr, lw);
    cairo_stroke(cr);
    cairo_restore(cr);
}

static void
DisplayCallback (Widget gw, XtPointer closure, XtPointer data)
{
    CardsWidget	    w = (CardsWidget) gw;
    HandDisplayPtr  display = (HandDisplayPtr) data;
    CardsCardPtr    card = (CardsCardPtr) display->private;
    cairo_t	    *cr = display->cr;

    cairo_matrix_t back_matrix;

    switch (card->suit) {
    case CardsEmpty:
	XkwSetSource(cr, &w->ksimple.background);
	FillCard (w, cr);
	break;
    case CardsBack:
	/* change the origin so cards will have the same back anywhere
	 * on the table
	 */
	cairo_matrix_init(&back_matrix, 1, 0, 0, 1, 0, 0);
	cairo_pattern_set_matrix(w->cards.back_pattern,
				 &back_matrix);
	cairo_set_source(cr, w->cards.back_pattern);
	FillCard (w, cr);
	break;
    case CardsNone:
	break;
    default:
	paint_card (w, cr, card->rank, card->suit);
	break;
    }
}

CardsClassRec	cardsClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) SuperClass,
    /* class_name		*/	"Cards",
    /* widget_size		*/	sizeof(CardsRec),
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
    /* resize			*/	XtInheritResize,
    /* expose			*/	XtInheritExpose,
    /* set_values		*/	SetValues,
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
    /* extension		*/	NULL
  },
  { /* hand fields */
    /* ignore                   */	0
  },
  { /* cards fields */
    /* ignore			*/	0
  },
};

WidgetClass cardsWidgetClass = (WidgetClass) &cardsClassRec;
