/***********************************************************

Copyright 1987, 1988, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
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
 * KCommand.c - Command button widget
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/Drawing.h>
#include <X11/Xmu/Misc.h>
#include <Xkw/KCommandP.h>
#include <Xkw/Xkw.h>

#define DEFAULT_HIGHLIGHT_THICKNESS 2
#define DEFAULT_SHAPE_HIGHLIGHT 32767
#define STR_EQUAL(str1, str2)	(str1 == str2 || strcmp(str1, str2) == 0)

/*
 * Class Methods
 */
static void XkwKCommandClassInitialize(void);
static void XkwKCommandInitialize(Widget, Widget, ArgList, Cardinal*);
static void XkwKCommandRealize(Widget, Mask*, XSetWindowAttributes*);
static void XkwKCommandResize(Widget);
static void XkwKCommandRedisplay(Widget, XEvent*, Region);
static Boolean XkwKCommandSetValues(Widget, Widget, Widget, ArgList, Cardinal*);
static void XkwKCommandGetValuesHook(Widget, ArgList, Cardinal*);

/*
 * Prototypes
 */
static void PaintKCommandWidget(Widget, XEvent*, Region, Bool);
static Region HighlightRegion(KCommandWidget);
static void XkwKCommandToggle(Widget);

/*
 * Actions
 */
static void Highlight(Widget, XEvent*, String*, Cardinal*);
static void Notify(Widget, XEvent*, String*, Cardinal*);
static void Reset(Widget, XEvent*, String*, Cardinal*);
static void Set(Widget, XEvent*, String*, Cardinal*);
static void Unhighlight(Widget, XEvent*, String*, Cardinal*);
static void Unset(Widget, XEvent*, String*, Cardinal*);

/*
 * Initialization
 */
static char defaultTranslations[] =
"<Enter>:"	"highlight()\n"
"<Leave>:"	"reset()\n"
"<Btn1Down>:"	"set()\n"
"<Btn1Up>:"	"notify() unset()\n"
;

#define offset(field) XtOffsetOf(KCommandRec, kcommand.field)
static XtResource resources[] = {
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      offset(callbacks), XtRCallback, NULL },
    { XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      offset(highlight_thickness), XtRImmediate, (XtPointer)DEFAULT_SHAPE_HIGHLIGHT },
};
#undef offset

static XtActionsRec actionsList[] = {
  {"set",		Set},
  {"notify",		Notify},
  {"highlight",		Highlight},
  {"reset",		Reset},
  {"unset",		Unset},
  {"unhighlight",	Unhighlight}
};

#define SuperClass ((KLabelWidgetClass)&klabelClassRec)

KCommandClassRec kcommandClassRec = {
  /* core */
  {
    (WidgetClass)SuperClass,		/* superclass		  */
    "Command",				/* class_name		  */
    sizeof(KCommandRec),		/* size			  */
    XkwKCommandClassInitialize,		/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    False,				/* class_inited		  */
    XkwKCommandInitialize,		/* initialize		  */
    NULL,				/* initialize_hook	  */
    XkwKCommandRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* num_resources	  */
    NULLQUARK,				/* xrm_class		  */
    False,				/* compress_motion	  */
    True,				/* compress_exposure	  */
    True,				/* compress_enterleave	  */
    False,				/* visible_interest	  */
    NULL,				/* destroy		  */
    XkwKCommandResize,			/* resize		  */
    XkwKCommandRedisplay,		/* expose		  */
    XkwKCommandSetValues,		/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    XkwKCommandGetValuesHook,		/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,		/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
  },
  /* klabel */
  {
    0,					/* not used */
  },
  /* kcommand */
  {
    0,					/* not used */
  },
};

WidgetClass kcommandWidgetClass = (WidgetClass)&kcommandClassRec;

/*
 * Implementation
 */

/*ARGSUSED*/
static void
XkwKCommandInitialize(Widget request, Widget cnew,
		      ArgList args, Cardinal *num_args)
{
    KCommandWidget cbw = (KCommandWidget)cnew;
    cbw->kcommand.set = False;
    cbw->kcommand.highlighted = HighlightNone;
}

static Region
HighlightRegion(KCommandWidget cbw)
{
    static Region outerRegion = NULL, innerRegion, emptyRegion;
    XRectangle rect;

    if (cbw->kcommand.highlight_thickness == 0 ||
        cbw->kcommand.highlight_thickness > Min(XtWidth(cbw), XtHeight(cbw)) / 2)
	return (NULL);

    if (outerRegion == NULL) {
	/* save time by allocating scratch regions only once. */
	outerRegion = XCreateRegion();
	innerRegion = XCreateRegion();
	emptyRegion = XCreateRegion();
    }

    rect.x = rect.y = 0;
    rect.width = XtWidth(cbw);
    rect.height = XtHeight(cbw);
    XUnionRectWithRegion(&rect, emptyRegion, outerRegion);
    rect.x = rect.y = cbw->kcommand.highlight_thickness;
    rect.width -= cbw->kcommand.highlight_thickness * 2;
    rect.height -= cbw->kcommand.highlight_thickness * 2;
    XUnionRectWithRegion(&rect, emptyRegion, innerRegion);
    XSubtractRegion(outerRegion, innerRegion, outerRegion);

    return (outerRegion);
}

/***************************
*  Action Procedures
***************************/
static void
XkwKCommandToggle(Widget w)
{
    KCommandWidget kcw = (KCommandWidget)w;
    Arg args[2];
    Cardinal num_args;
    XRenderColor foreground = kcw->klabel.background;
    XRenderColor background = kcw->klabel.foreground;

    num_args = 0;
    XkwSetArg(args[num_args], XtNbackgroundColor,
	     &background);		++num_args;
    XkwSetArg(args[num_args], XtNforegroundColor,
	     &foreground);		++num_args;
    XtSetValues(w, args, num_args);
}

/*ARGSUSED*/
static void
Set(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KCommandWidget cbw = (KCommandWidget)w;

    if (cbw->kcommand.set)
	return;

    XkwKCommandToggle(w);
    cbw->kcommand.set= True;
}

/*ARGSUSED*/
static void
Unset(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KCommandWidget cbw = (KCommandWidget)w;

    if (!cbw->kcommand.set)
	return;

    cbw->kcommand.set = False;
    XkwKCommandToggle(w);
}

/*ARGSUSED*/
static void
Reset(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KCommandWidget cbw = (KCommandWidget)w;

    if (cbw->kcommand.set) {
	cbw->kcommand.highlighted = HighlightNone;
	Unset(w, event, params, num_params);
    }
    else
	Unhighlight(w, event, params, num_params);
}

/*ARGSUSED*/
static void
Highlight(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KCommandWidget cbw = (KCommandWidget)w;

    if (*num_params == (Cardinal)0)
	cbw->kcommand.highlighted = HighlightWhenUnset;
    else {
	if (*num_params != (Cardinal)1)
	    XtWarning("Too many parameters passed to highlight action table.");
	switch (params[0][0]) {
	    case 'A':
	    case 'a':
		cbw->kcommand.highlighted = HighlightAlways;
		break;
	    default:
		cbw->kcommand.highlighted = HighlightWhenUnset;
		break;
	}
    }

    if (XtIsRealized(w))
	PaintKCommandWidget(w, event, HighlightRegion(cbw), True);
}

/*ARGSUSED*/
static void
Unhighlight(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KCommandWidget cbw = (KCommandWidget)w;

    cbw->kcommand.highlighted = HighlightNone;
    if (XtIsRealized(w))
	PaintKCommandWidget(w, event, HighlightRegion(cbw), True);
}

/*ARGSUSED*/
static void
Notify(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KCommandWidget cbw = (KCommandWidget)w;

    /* check to be sure state is still Set so that user can cancel
       the action (e.g. by moving outside the window, in the default
       bindings.
    */
    if (cbw->kcommand.set)
	XtCallCallbackList(w, cbw->kcommand.callbacks, (XtPointer) NULL);
}

static void
XkwKCommandRedisplay(Widget w, XEvent *event, Region region)
{
    PaintKCommandWidget(w, event, region, False);
}

/*
 * Function:
 *	PaintKCommandWidget
 * Parameters:
 *	w      - kcommand widget
 *	region - region to paint (passed to the superclass)
 *                 change - did it change either set or highlight state?
 */
static void
PaintKCommandWidget(Widget w, XEvent *event, Region region, Bool change)
{
    KCommandWidget cbw = (KCommandWidget)w;

    if (cbw->kcommand.highlight_thickness)
    {
	Bool very_thick = cbw->kcommand.highlight_thickness
	    > Min(XtWidth(cbw), XtHeight(cbw)) / 2;

	/*
	 * If we are set then use the same colors as if we are not highlighted
	 */

	if (!((!change && cbw->kcommand.highlighted == HighlightNone)
	      || (cbw->kcommand.highlighted == HighlightWhenUnset
		  && cbw->kcommand.set)))
	{
	    cairo_t *cr = XkwGetCairo(w);

	    if (cbw->kcommand.highlighted != HighlightNone)
		XkwSetSource(cr, &cbw->klabel.foreground);
	    else
		XkwSetSource(cr, &cbw->klabel.background);

	    if (very_thick)
		cairo_paint(cr);
	    else {
		cairo_set_line_width(cr, cbw->kcommand.highlight_thickness);
		double offset = cbw->kcommand.highlight_thickness / 2.0;
		cairo_rectangle(cr, offset, offset,
				XtWidth(cbw) - cbw->kcommand.highlight_thickness,
				XtHeight(cbw) - cbw->kcommand.highlight_thickness);
		cairo_fill(cr);
	    }
	}
    }

    (*SuperClass->core_class.expose)(w, event, region);
}

/*ARGSUSED*/
static Boolean
XkwKCommandSetValues(Widget current, Widget request, Widget cnew,
		    ArgList args, Cardinal *num_args)
{
    KCommandWidget oldcbw = (KCommandWidget)current;
    KCommandWidget cbw = (KCommandWidget)cnew;

    if (oldcbw->core.sensitive != cbw->core.sensitive && !cbw->core.sensitive) {
	cbw->kcommand.highlighted = HighlightNone;
    }

    if (cbw->kcommand.set) {
	unsigned int i;
	XRenderColor foreground, background;

	foreground = oldcbw->klabel.foreground;
	background = oldcbw->klabel.background;
	for (i = 0; i < *num_args; i++) {
	    if (STR_EQUAL(args[i].name, XtNforegroundColor))
		background = cbw->klabel.foreground;
	    else if (STR_EQUAL(args[i].name, XtNbackgroundColor))
		foreground = cbw->klabel.background;
	}
	cbw->klabel.foreground = foreground;
	cbw->klabel.background = background;
    }

    return True;
}

static void
XkwKCommandGetValuesHook(Widget w, ArgList args, Cardinal *num_args)
{
    KCommandWidget cbw = (KCommandWidget)w;
    unsigned int i;

    for (i = 0; i < *num_args; i++) {
	if (STR_EQUAL(args[i].name, XtNforegroundColor))
	    *((XRenderColor*)args[i].value) = cbw->kcommand.set ?
		cbw->klabel.background : cbw->klabel.foreground;
	else if (STR_EQUAL(args[i].name, XtNbackgroundColor))
	    *((XRenderColor*)args[i].value) = cbw->kcommand.set ?
		cbw->klabel.foreground : cbw->klabel.background;
    }
}

static void
XkwKCommandClassInitialize(void)
{
    XkwInitializeWidgetSet();
    XtSetTypeConverter(XtRShapeStyle, XtRString, XmuCvtShapeStyleToString,
		       NULL, 0, XtCacheNone, NULL);
}

static void
XkwKCommandRealize(Widget w, Mask *valueMask, XSetWindowAttributes *attributes)
{
    (*kcommandWidgetClass->core_class.superclass->core_class.realize)
	(w, valueMask, attributes);
}

static void
XkwKCommandResize(Widget w)
{
    if (kcommandWidgetClass->core_class.superclass->core_class.resize)
	(*kcommandWidgetClass->core_class.superclass->core_class.resize)(w);
}
