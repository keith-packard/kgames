/*
Copyright 1989, 1994, 1998  The Open Group

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
 */

/*
 * Date:    September 26, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium
 *          kit@expo.lcs.mit.edu
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>
#include <Xkw/KSmeP.h>
#include <Xkw/Xkw.h>

/*
 * Class Methods
 */
static void Highlight(Widget);
static void Notify(Widget);
static void Unhighlight(Widget);
static void XkwKSmeClassPartInitialize(WidgetClass);
static void XkwKSmeInitialize(Widget, Widget, ArgList, Cardinal*);
static XtGeometryResult XkwKSmeQueryGeometry(Widget, XtWidgetGeometry*,
					    XtWidgetGeometry*);

/*
 * Initialization
 */
#define offset(field) XtOffsetOf(KSmeRec, ksme.field)
static XtResource resources[] = {
  {
    XtNcallback,
    XtCCallback,
    XtRCallback,
    sizeof(XtPointer),
    offset(callbacks),
    XtRCallback,
    NULL
  },
  {
    XtNinternational,
    XtCInternational,
    XtRBoolean,
    sizeof(Boolean),
    offset(international),
    XtRImmediate,
    (XtPointer)False
  },
};
#undef offset

#define Superclass	(&rectObjClassRec)
KSmeClassRec ksmeClassRec = {
  /* rectangle */
  {
    (WidgetClass)Superclass,		/* superclass */
    "KSme",				/* class_name */
    sizeof(KSmeRec),			/* widget_size */
    XkwInitializeWidgetSet,		/* class_initialize */
    XkwKSmeClassPartInitialize,		/* class_part_initialize */
    False,				/* class_initialized */
    XkwKSmeInitialize,			/* initialize */
    NULL,				/* initialize_hook */
    NULL,				/* realize */
    NULL,				/* actions */
    0,					/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    False,				/* compress_motion */
    False,				/* compress_exposure */
    False,				/* compress_enterleave */
    False,				/* visible_interest */
    NULL,				/* destroy */
    XtInheritResize,			/* resize */
    NULL,				/* expose */
    NULL,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* intrinsics_version */
    NULL,				/* callback offsets */
    NULL,				/* tm_table */
    XkwKSmeQueryGeometry,		/* query_geometry */
    NULL,				/* display_accelerator */
    NULL,				/* extension */
  },
  /* sme */
  {
    Highlight,				/* highlight */
    Unhighlight,			/* unhighlight */
    Notify,				/* notify */
    NULL,				/* extension */
  }
};

WidgetClass ksmeObjectClass = (WidgetClass)&ksmeClassRec;

/*
 * Implementation
 */
/*
 * Function:
 *	XkwKSmeClassPartInitialize
 *
 * Parameters:
 *	cclass - widget classs of this widget
 *
 * Description:
 *	Handles inheritance of class functions.
 */
static void
XkwKSmeClassPartInitialize(WidgetClass cclass)
{
    KSmeObjectClass m_ent, superC;

    m_ent = (KSmeObjectClass)cclass;
    superC = (KSmeObjectClass)m_ent->rect_class.superclass;

    if (m_ent->sme_class.highlight == XtInheritHighlight)
	m_ent->sme_class.highlight = superC->sme_class.highlight;

    if (m_ent->sme_class.unhighlight == XtInheritUnhighlight)
	m_ent->sme_class.unhighlight = superC->sme_class.unhighlight;

    if (m_ent->sme_class.notify == XtInheritNotify)
	m_ent->sme_class.notify = superC->sme_class.notify;
}

/*
 * Function:
 *	XkwKSmeInitialize
 *
 * Parameters:
 *	request - widget requested by the argument list
 *	cnew	- new widget with both resource and non  resource values
 *
 * Description:
 *	Initializes the simple menu widget entry
 */
/*ARGSUSED*/
static void
XkwKSmeInitialize(Widget request, Widget cnew,
		 ArgList args, Cardinal *num_args)
{
    KSmeObject entry = (KSmeObject)cnew;

    (void) request;
    (void) args;
    (void) num_args;
    entry->rectangle.border_width = 0;
}

/*
 * Function:
 *	Highlight
 *
 * Parameters:
 *	w - menu entry
 *
 * Description:
 *	Default highlight proceedure for menu entries.
 */
/*ARGSUSED*/
static void
Highlight(Widget w)
{
    (void) w;
}

/*
 * Function:
 *	Unhighlight
 *
 * Parameters:
 *	w - menu entry
 *
 * Description:
 *	Default unhighlight proceedure for menu entries.
 */
/*ARGSUSED*/
static void
Unhighlight(Widget w)
{
    (void) w;
}

/*
 * Function:
 *	Notify
 *
 * Parameters:
 *	w - menu entry
 *
 * Description:
 *	Calls the callback proceedures for this entry.
 */
static void
Notify(Widget w)
{
    XtCallCallbacks(w, XtNcallback, NULL);
}

/*
 * Function:
 *	QueryGeometry
 *
 * Parameeters:
 *	w	   - menu entry object
 *	itended	   - intended and return geometry info
 *	return_val -
 *
 * Description:
 *	Returns the preferred geometry for this widget.
 *
 * Returns:
 *	Geometry Result
 *
 * Note:
 *	See the Intrinsics manual for details on what this function is for.
 */
static XtGeometryResult
XkwKSmeQueryGeometry(Widget w, XtWidgetGeometry *intended,
		    XtWidgetGeometry *return_val)
{
    KSmeObject entry = (KSmeObject)w;
    Dimension width;
    XtGeometryResult ret_val = XtGeometryYes;
    XtGeometryMask mode = intended->request_mode;

    width = 1;

    if (((mode & CWWidth) && intended->width != width) || !(mode & CWWidth)) {
	return_val->request_mode |= CWWidth;
	return_val->width = width;
	mode = return_val->request_mode;

	if ((mode & CWWidth) && width == XtWidth(entry))
	    return (XtGeometryNo);
	return (XtGeometryAlmost);
    }

    return (ret_val);
}
