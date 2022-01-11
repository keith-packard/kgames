/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * Reversi.c - Reversi widget
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xmu/Drawing.h>
#include <math.h>
#include "ReversiP.h"

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

#define STONE_WIDTH 0.75

#define offset(field) XtOffsetOf(ReversiRec, field)

static XtResource resources[] = {
    {XtNstoneCallback, XtCStoneCallback, XtRCallback, sizeof (XtPointer),
	offset(reversi.callbacks), XtRCallback, (XtPointer) NULL},
    {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	offset(core.background_pixel), XtRString, "green"},
    {XtNbackgroundColor, XtCBackground, XtRRenderColor, sizeof(XRenderColor),
     offset(ksimple.background), XtRString, "green"},
    {XtNwhite, XtCForeground, XtRRenderColor, sizeof(XRenderColor),
	offset(reversi.white), XtRString, "white"},
    {XtNblack, XtCForeground, XtRRenderColor, sizeof(XRenderColor),
	offset(reversi.black), XtRString, "black"},
    {XtNgrid, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
	offset(reversi.grid), XtRString, "black"},
};

#define superclass		(&ksimpleClassRec)

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void ClassInitialize(void)
{
    XkwInitializeWidgetSet();
}

/* ARGSUSED */
static void Initialize(Widget request, Widget new, Arg *args, Cardinal *count)
{
    ReversiWidget rw = (ReversiWidget) new;
    int		x, y;

    (void) request;
    (void) args;
    (void) count;

    if (rw->core.width == 0)
        rw->core.width = 100;
    if (rw->core.height == 0)
        rw->core.height = 100;

    for (y = 0; y < BOARD_HEIGHT; y++)
	for (x = 0; x < BOARD_HEIGHT; x++) {
	    rw->reversi.board[x][y] = StoneNone;
	    rw->reversi.animate[x][y].state = AnimateNone;
	}
} /* Initialize */

/*
 * Repaint the widget window
 */

static void PaintEntry (ReversiWidget rw, cairo_t *cr, int x, int y)
{
    ReversiStone    value;

    switch (rw->reversi.animate[x][y].state)
    {
    case AnimateNone:
	value = rw->reversi.board[x][y];
	break;
    case AnimateA:
	value = rw->reversi.animate[x][y].A;
	break;
    case AnimateB:
	value = rw->reversi.animate[x][y].B;
	break;
    default:
	return;
    }
    switch (value)
    {
    case StoneBlack:
	XkwSetSource(cr, &rw->reversi.black);
	cairo_arc(cr, x + 0.5, y + 0.5, STONE_WIDTH / 2, 0, M_PI * 2);
	break;
    case StoneWhite:
	XkwSetSource(cr, &rw->reversi.white);
	cairo_arc(cr, x + 0.5, y + 0.5, STONE_WIDTH / 2, 0, M_PI * 2);
	break;
    default:
	XkwSetSource(cr, &rw->ksimple.background);
	cairo_rectangle(cr, x + 0.1, y + 0.1, 0.8, 0.8);
	break;
    }
    cairo_fill(cr);
}

static void
PaintGrid (ReversiWidget rw, cairo_t *cr)
{
    int i;
    XkwSetSource(cr, &rw->reversi.grid);
    for (i = 1; i < BOARD_HEIGHT; i++)
    {
	cairo_move_to(cr, i, 0);
	cairo_line_to(cr, i, BOARD_WIDTH);
    }
    for (i = 1; i < BOARD_WIDTH; i++)
    {
	cairo_move_to(cr, 0, i);
	cairo_line_to(cr, BOARD_HEIGHT, i);
    }
    cairo_stroke(cr);
}

static cairo_t *
draw_begin(ReversiWidget w, Region region)
{
    cairo_t *cr = XkwDrawBegin((Widget) w, region);
    cairo_scale(cr, w->core.width / 8, w->core.height / 8);
    cairo_set_line_width(cr, 0.025);
    return cr;
}

/* ARGSUSED */
static void Redisplay(Widget w, XEvent *event, Region region)
{
    ReversiWidget rw = (ReversiWidget) w;
    int	x, y;

    if (!XtIsRealized(w))
	return;
    (void) event;
    (void) region;
    cairo_t *cr = draw_begin(rw, region);
    PaintGrid (rw, cr);
    for (y = 0; y < BOARD_HEIGHT; y++)
	for (x = 0; x < BOARD_WIDTH; x++)
	    PaintEntry (rw, cr, x, y);
    XkwDrawEnd(w, region, cr);
}

/*
 * Set specified arguments into widget
 */

/* ARGSUSED */
static Boolean SetValues(Widget current, Widget request, Widget new, Arg *args, Cardinal *count)
{
    (void) current;
    (void) request;
    (void) new;
    (void) args;
    (void) count;
    return False;
}

void
XkwReversiSetSpot (
		   Widget	w,
		   int		x,
		   int		y,
		   ReversiStone	value
		   )
{
    ReversiWidget   rw = (ReversiWidget) w;

    if (rw->reversi.animate[x][y].state != AnimateNone)
    {
	XtRemoveTimeOut (rw->reversi.animate[x][y].timeout);
	rw->reversi.animate[x][y].state = AnimateNone;
    }

    rw->reversi.board[x][y] = value;
}

void
XkwReversiUpdate(Widget w)
{
    Redisplay(w, NULL, NULL);
}

static void
DoAnimate (XtPointer closure, XtIntervalId *interval)
{
    Animate *a = (Animate *) closure;

    (void) interval;
    if (a->togo == 0)
    {
	a->state = AnimateNone;
	a->timeout = 0;
    }
    else
    {
	--a->togo;
	if (a->state == AnimateA)
	    a->state = AnimateB;
	else
	    a->state = AnimateA;
	a->timeout = XtAddTimeOut (a->delay, DoAnimate, (XtPointer) a);
    }
    Redisplay((Widget) a->rw, NULL, NULL);
}

void
XkwReversiAnimateSpot (Widget		w,
		       int		x,
		       int		y,
		       ReversiStone	A,
		       ReversiStone	B,
		       unsigned long	delay,
		       int		repeat)
{
    ReversiWidget   rw = (ReversiWidget) w;
    Animate	    *a;

    a = &rw->reversi.animate[x][y];
    if (a->state != AnimateNone)
    {
	XtRemoveTimeOut (a->timeout);
    }
    a->state = AnimateNone;
    if (repeat == 0)
    {
	Redisplay(w, NULL, NULL);
	return;
    }
    a->delay = delay;
    a->togo = repeat - 1;
    a->x = x;
    a->y = y;
    a->rw = rw;
    a->A = A;
    a->B = B;
    a->state = AnimateA;
    if (!(a->timeout = XtAddTimeOut (delay, DoAnimate, (XtPointer) a)))
	a->state = AnimateNone;
    Redisplay(w, NULL, NULL);
}

static void
Select (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    ReversiWidget   rw = (ReversiWidget) w;
    int	    x, y;
    double  dx, dy;
    int	    rx, ry;
    ReversiMove	move;

    (void) params;
    (void) num_params;
    switch (event->type)
    {
    case ButtonPress:
    case ButtonRelease:
	x = event->xbutton.x;
	y = event->xbutton.y;
	break;
    case MotionNotify:
	x = event->xmotion.x;
	y = event->xmotion.y;
	break;
    default:
	return;
    }
    dx = (double) x * 8 / rw->core.width;
    dy = (double) y * 8 / rw->core.height;
    move.x = rx = dx;
    move.y = ry = dy;
    if (0 <= rx && rx < BOARD_WIDTH && 0 <= ry && ry < BOARD_HEIGHT)
	XtCallCallbackList ((Widget) rw, rw->reversi.callbacks, (XtPointer) &move);
}

static XtActionsRec actions[] =
{
    {"select",	    Select},
};

static char defaultTranslations[] =
    "<BtnDown>:	select()";

ReversiClassRec reversiClassRec = {
  {
/* core_class fields */
    /* superclass	  	*/	(WidgetClass) superclass,
    /* class_name	  	*/	"Reversi",
    /* widget_size	  	*/	sizeof(ReversiRec),
    /* class_initialize   	*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited       	*/	FALSE,
    /* initialize	  	*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize		  	*/	XtInheritRealize,
    /* actions		  	*/	actions,
    /* num_actions	  	*/	XtNumber (actions),
    /* resources	  	*/	resources,
    /* num_resources	  	*/	XtNumber(resources),
    /* xrm_class	  	*/	NULLQUARK,
    /* compress_motion	  	*/	TRUE,
    /* compress_exposure  	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest	  	*/	FALSE,
    /* destroy		  	*/	NULL,
    /* resize		  	*/	XtInheritResize,
    /* expose		  	*/	Redisplay,
    /* set_values	  	*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus	 	*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private   	*/	NULL,
    /* tm_table		   	*/	defaultTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  }, {
/* simple class */
    /* change_sensitive		*/	XtInheritChangeSensitive,
    /* extension		*/	NULL,
  /* ksimple */
  }, {
      0,
  }, {
/* reversi class */
      /* foo			*/	0,
  }
};

WidgetClass reversiWidgetClass = (WidgetClass)&reversiClassRec;
