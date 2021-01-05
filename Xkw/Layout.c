/*
 * $XConsortium: Layout.c,v 1.1 91/09/13 18:51:44 keith Exp $
 *
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
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xmu/Misc.h>
#include <X11/Xmu/Converters.h>

#include "LayoutP.h"

#include <ctype.h>

#include <stdio.h>

/*****************************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************************/

#define offset(field) XtOffsetOf(LayoutRec, layout.field)

static XtResource resources[] = {
    {XtNlayout, XtCLayout, XtRLayout, sizeof (BoxPtr),
	offset(layout), XtRLayout, NULL },
    {XtNdebug, XtCBoolean, XtRBoolean, sizeof(Boolean),
	offset(debug), XtRImmediate, (XtPointer) FALSE},
    {XtNresize, XtCResize, XtRBoolean, sizeof(Boolean),
	offset(resize), XtRImmediate, (XtPointer) TRUE},
};

#undef offset

#ifdef MOTIF
#define SuperClass ((ConstraintWidgetClass)&xmManagerClassRec)
#else
#define SuperClass ((ConstraintWidgetClass)&constraintClassRec)
#endif

static void
LayoutFreeLayout (BoxPtr box);

static void
LayoutLayout (LayoutWidget l, Bool attemptResize);

static void
LayoutGetNaturalSize (LayoutWidget l, Dimension *widthp, Dimension *heightp);

static Boolean
ChildInLayout (LayoutWidget l, Widget child);

/************************************************************
 *
 * Semi-public routines.
 *
 ************************************************************/

/*	Function Name: ClassInitialize
 *	Description: The Layout widgets class initialization proc.
 *	Arguments: none.
 *	Returns: none.
 */

/*ARGSUSED*/
static Boolean
CvtStringToLayout (Display *dpy,
		   XrmValue *args,
		   Cardinal *num_args,
		   XrmValue *from,
		   XrmValue *to,
		   XtPointer *converter_data)
{
    (void) dpy;
    (void) args;
    (void) num_args;
    (void) converter_data;
    LayYYsetsource ((char *) from->addr);
    LayYYsetdest ((BoxPtr *) to->addr);
    if (LayYYparse () == 0)
	return TRUE;
    else
	return FALSE;
}

static void
DisposeLayout (XtAppContext app,
	       XrmValue *to,
	       XtPointer data,
	       XrmValuePtr args,
	       Cardinal *num_args)
{
    (void) app;
    (void) data;
    (void) args;
    (void) num_args;
    LayoutFreeLayout (* (LayoutPtr *) to->addr);
}

static void
ClassInitialize(void)
{
    XtSetTypeConverter ( XtRString, XtRLayout, CvtStringToLayout,
		    NULL, (Cardinal)0, XtCacheNone,
 		    DisposeLayout );
}

static XtGeometryResult
GeometryManager(Widget child,
		XtWidgetGeometry *request,
		XtWidgetGeometry *reply)
{
    LayoutWidget    w = (LayoutWidget) XtParent(child);
    SubInfoPtr	    p = SubInfo(child);
    int		    bw;
    Bool	    changed, bwChanged;

    (void) reply;
    if (!ChildInLayout(w, child)) {
	/* copy values from request to child */
	if (request->request_mode & CWX)
	    child->core.x = request->x;
	if (request->request_mode & CWY)
	    child->core.y = request->y;
	if (request->request_mode & CWWidth)
	    child->core.width = request->width;
	if (request->request_mode & CWHeight)
	    child->core.height = request->height;
	if (request->request_mode & CWBorderWidth)
	    child->core.border_width = request->border_width;
	return XtGeometryYes;
    }

    bw = p->naturalBw;
    changed = FALSE;
    bwChanged = FALSE;
    if (request->request_mode & CWBorderWidth &&
	request->border_width != child->core.border_width)
    {
	p->naturalBw = bw;
	bw = request->border_width;
	changed = TRUE;
	bwChanged = TRUE;
    }
    if (bwChanged || ((request->request_mode & CWWidth) &&
		      request->width != child->core.width))
    {
	p->naturalSize[LayoutHorizontal] = request->width + bw * 2;
	changed = TRUE;
    }
    if (bwChanged || ((request->request_mode & CWHeight) &&
		      request->height != child->core.height))
    {
	p->naturalSize[LayoutVertical] = request->height + bw * 2;
	changed = TRUE;
    }
    /*
     * Allow arbitrary restacking by just doing it here; the layout
     * process doesn't need to know about it
     */
    if (request->request_mode & CWStackMode)
    {
	XWindowChanges	ch;


	ch.stack_mode = request->stack_mode;
	if (request->request_mode & CWSibling)
	    ch.sibling = XtWindow (request->sibling);

	XConfigureWindow (XtDisplay (child), XtWindow (child),
			  request->request_mode & (CWStackMode|CWSibling),
			  &ch);
    }
    if (changed)
	LayoutLayout (w, TRUE);
    return XtGeometryDone;
}

static void
Initialize(Widget request, Widget new, Arg *args, Cardinal *count)
{
    (void) request;
    (void) new;
    (void) args;
    (void) count;
}

static void
GetDesiredSize (Widget child)
{
    XtWidgetGeometry	desired;
    SubInfoPtr		p;

    XtQueryGeometry (child, (XtWidgetGeometry *) NULL, &desired);
    p = SubInfo (child);
    p->naturalBw = desired.border_width;
    p->naturalSize[LayoutHorizontal] = desired.width + desired.border_width * 2;
    p->naturalSize[LayoutVertical] = desired.height + desired.border_width * 2;
}

static void
ChangeManaged(Widget gw)
{
    LayoutWidget	w = (LayoutWidget) gw;
    Widget		*children;

    ForAllChildren (w, children)
	GetDesiredSize (*children);
    LayoutLayout ((LayoutWidget) w, TRUE);
}

static void InsertChild (Widget child)
{
    (*SuperClass->composite_class.insert_child) (child);
    GetDesiredSize (child);
}

static void
Resize(Widget gw)
{
    LayoutLayout ((LayoutWidget) gw, FALSE);
}

static Boolean
SetValues(Widget gold, Widget greq, Widget gnew, Arg *args, Cardinal *count)
{
    LayoutWidget    old = (LayoutWidget) gold,
		    req = (LayoutWidget) greq,
		    new = (LayoutWidget) gnew;

    (void) req;
    (void) args;
    (void) count;
    if (old->layout.layout != new->layout.layout)
	LayoutLayout (new, TRUE);
    return FALSE;
} /* SetValues */

static XtGeometryResult
QueryGeometry (Widget gw, XtWidgetGeometry *request, XtWidgetGeometry *prefered_return)
{
    LayoutWidget	w = (LayoutWidget) gw;
    XtGeometryResult	result;
    XtWidgetGeometry	prefered_size;

    if (request && !(request->request_mode & (CWWidth|CWHeight)))
	return XtGeometryYes;
    LayoutGetNaturalSize (w, &prefered_size.width, &prefered_size.height);
    prefered_return->request_mode = 0;
    result = XtGeometryYes;
    if (!request) {
	prefered_return->width = prefered_size.width;
	prefered_return->height= prefered_size.height;
	if (prefered_size.width != w->core.width) {
	    prefered_return->request_mode |= CWWidth;
	    result = XtGeometryAlmost;
	}
	if (prefered_size.height != w->core.height) {
	    prefered_return->request_mode |= CWHeight;
	    result = XtGeometryAlmost;
	}
    } else {
    	if (request->request_mode & CWWidth) {
	    if (prefered_size.width > request->width)
	    {
	    	if (prefered_size.width == w->core.width)
		    result = XtGeometryNo;
	    	else if (result != XtGeometryNo) {
		    result = XtGeometryAlmost;
		    prefered_return->request_mode |= CWWidth;
		    prefered_return->width = prefered_size.width;
	    	}
	    }
    	}
    	if (request->request_mode & CWHeight) {
	    if (prefered_size.height > request->height)
	    {
	    	if (prefered_size.height == w->core.height)
		    result = XtGeometryNo;
	    	else if (result != XtGeometryNo) {
		    result = XtGeometryAlmost;
		    prefered_return->request_mode |= CWHeight;
		    prefered_return->height = prefered_size.height;
	    	}
	    }
    	}
    }
    return result;
}

/*
 * Layout section.  Exports LayoutGetNaturalSize and
 * LayoutLayout to above section
 */

static void
PrintGlue (GlueRec g)
{
    if (g.order == 0 || g.value != 1.0)
	printf ("%g", g.value);
    if (g.order > 0)
    {
	printf (" inf");
	if (g.order > 1)
	    printf (" %d", g.order);
    }
}

static void
PrintDirection (LayoutDirection dir)
{
    switch (dir) {
    case LayoutHorizontal:
	printf ("horizontal");
	break;
    case LayoutVertical:
	printf ("vertical");
	break;
    case LayoutOverlay:
	printf ("overlay");
	break;
    default:
	printf ("Unknown layout direction %d\n", dir);
	break;

    }
}

static void
TabTo(int level)
{
    while (level--)
	printf ("  ");
}

static void
PrintBox (BoxPtr box, int level)
{
    BoxPtr	child;

    TabTo (level);
    switch (box->type) {
    case BoxBox:
	PrintDirection (box->u.box.dir);
	break;
    case WidgetBox:
	printf ("%s ", XrmQuarkToString (box->u.widget.quark));
	break;
    case GlueBox:
	printf ("glue ");
	break;
    case VariableBox:
	printf ("variable %s\n", XrmQuarkToString (box->u.variable.quark));
	return;
    }
    printf ("< ");
    printf (" + "); PrintGlue (box->params.stretch[LayoutHorizontal]);
    printf (" - "); PrintGlue (box->params.shrink[LayoutHorizontal]);
    printf (" * ");
    printf (" + "); PrintGlue (box->params.stretch[LayoutVertical]);
    printf (" - "); PrintGlue (box->params.shrink[LayoutVertical]);
    printf (" >");
    printf (" size: %d x %d", box->size[0], box->size[1]);
    printf (" natural: %d x %d\n", box->natural[0], box->natural[1]);
    if (box->type == BoxBox)
	for (child = box->u.box.firstChild; child; child = child->nextSibling)
	    PrintBox (child, level+1);
}

static ExprPtr
LookupVariable (BoxPtr child, XrmQuark quark)
{
    BoxPtr	parent, box;

    while ((parent = child->parent))
    {
	for (box = parent->u.box.firstChild;
	     box != child;
	     box = box->nextSibling)
	{
	    if (box->type == VariableBox && box->u.variable.quark == quark)
		return box->u.variable.expr;
	}
	child = parent;
    }
    return 0;
}

static double
Evaluate (LayoutWidget l, BoxPtr box, ExprPtr expr, double natural)
{
    double	left, right, down;
    Widget	widget;
    SubInfoPtr	info;

    switch (expr->type) {
    case Constant:
	return expr->u.constant;
    case Binary:
	left = Evaluate (l, box, expr->u.binary.left, natural);
	right = Evaluate (l, box, expr->u.binary.right, natural);
	switch (expr->u.binary.op) {
        case Plus:
	    return left + right;
	case Minus:
	    return left - right;
	case Times:
	    return left * right;
	case Divide:
	    return left / right;
	case Percent:
	    return right * left / 100.0;
	case Minimum:
	    if (right < left)
		return right;
	    return left;
	case Maximum:
	    if (right > left)
		return right;
	    return left;
	default:
	    return left;
	}
    case Unary:
	down = Evaluate (l, box, expr->u.unary.down, natural);
	switch (expr->u.unary.op) {
	case Percent:
	    return natural * down / 100.0;
	case Minus:
	    return -down;
	case Minimum:
	case Maximum:
	    return down;
	case Plus:
	    return down;
	default:
	    return down;
	}
    case Width:
	widget = QuarkToWidget (l, expr->u.width);
	if (!widget)
	    return 0;
	info = SubInfo (widget);
	return info->naturalSize[LayoutHorizontal];
    case Height:
	widget = QuarkToWidget (l, expr->u.height);
	if (!widget)
	    return 0;
	info = SubInfo (widget);
	return info->naturalSize[LayoutVertical];
    case Variable:
	expr = LookupVariable (box, expr->u.variable);
	if (!expr)
	{
	    char    buf[256];
	    sprintf (buf, "Layout: undefined variable %s\n",
		     XrmQuarkToString (expr->u.variable));
	    XtError (buf);
	    return 0.0;
	}
	return Evaluate (l, box, expr, natural);
    default:
	return 0.0;
    }
}

static void
DisposeExpr (ExprPtr expr)
{
    if (!expr)
	return;
    switch (expr->type) {
    case Constant:
	break;
    case Binary:
	DisposeExpr (expr->u.binary.left);
	DisposeExpr (expr->u.binary.right);
	break;
    case Unary:
	DisposeExpr (expr->u.unary.down);
	break;
    case Width:
	break;
    case Height:
	break;
    default:
	break;
    }
    Dispose (expr);
}

#define CheckGlue(l, box, glue, n) { \
    if (glue.expr) \
	glue.value = Evaluate (l, box, glue.expr, n); \
    if (glue.order == 0 && glue.value == 0) \
	glue.order = -1; \
    else if (glue.order == -1 && glue.value != 0) \
	glue.order = 0; \
}

static Boolean
ChildInBox(LayoutWidget l, BoxPtr box, Widget child)
{
    while (box) {
	switch (box->type) {
	case WidgetBox:
	    if (child == QuarkToWidget(l, box->u.widget.quark))
		return True;
	    break;
	case GlueBox:
	    break;
	case BoxBox:
	    if (ChildInBox(l, box->u.box.firstChild, child))
		return True;
	    break;
	case VariableBox:
	    break;
	}
	box = box->nextSibling;
    }
    return False;
}

static Boolean
ChildInLayout(LayoutWidget l, Widget child)
{
    return ChildInBox(l, l->layout.layout, child);
}

#define DoStretch(l, box, dir) \
    CheckGlue (l, box, box->params.stretch[dir], (double) box->natural[dir]);

#define DoShrink(l, box, dir) \
    CheckGlue (l, box, box->params.shrink[dir], (double) box->natural[dir])

/* compute the natural sizes of a box */
static void
ComputeNaturalSizes (LayoutWidget l, BoxPtr box, LayoutDirection dir)
{
    BoxPtr	child;
    Widget	w;
    SubInfoPtr	info;
    int		minStretchOrder, minShrinkOrder;
    LayoutDirection thisDir;

    switch (box->type) {
    case WidgetBox:
	w = box->u.widget.widget = QuarkToWidget (l, box->u.widget.quark);
	if (!w)
	{
	    box->natural[LayoutHorizontal] = 0;
	    box->natural[LayoutVertical] = 0;
	}
	else
	{
	    info = SubInfo (w);
	    box->natural[LayoutHorizontal] = info->naturalSize[LayoutHorizontal];
	    box->natural[LayoutVertical] = info->naturalSize[LayoutVertical];
	}
	DoStretch (l, box, LayoutHorizontal);
	DoShrink (l, box, LayoutHorizontal);
	DoStretch (l, box, LayoutVertical);
	DoShrink (l, box, LayoutVertical);
	break;
    case GlueBox:
	if (box->u.glue.expr[1])
	{
	    box->natural[LayoutHorizontal] = Evaluate(l, box, box->u.glue.expr[LayoutHorizontal], 0.0);
	    box->natural[LayoutVertical] = Evaluate(l, box, box->u.glue.expr[LayoutVertical], 0.0);
	    DoStretch (l, box, LayoutHorizontal);
	    DoShrink (l, box, LayoutHorizontal);
	    DoStretch (l, box, LayoutVertical);
	    DoShrink (l, box, LayoutVertical);
	}
	else
	{
	    if (dir == LayoutHorizontal || dir == LayoutVertical)
	    {
		box->natural[dir] = Evaluate (l, box, box->u.glue.expr[0], 0.0);
		box->natural[!dir] = 0;
		DoStretch (l, box, dir);
		DoShrink (l, box, dir);
	    }
	    else
	    {
		box->natural[LayoutHorizontal] = 0;
		box->natural[LayoutVertical] = 0;
	    }
	}
	break;
    case BoxBox:
	thisDir = box->u.box.dir;
	box->natural[0] = 0;
	box->natural[1] = 0;
	minStretchOrder = 100000;
	minShrinkOrder = 100000;
	for (thisDir = LayoutHorizontal; thisDir <= LayoutVertical; thisDir++)
	{
	    if (thisDir == box->u.box.dir)
	    {
		ZeroGlue (box->params.shrink[thisDir]);
		ZeroGlue (box->params.stretch[thisDir]);
	    }
	    else
	    {
		box->params.shrink[thisDir].order = 100000;
		box->params.shrink[thisDir].value = 0;
		box->params.stretch[thisDir].order = 100000;
		box->params.stretch[thisDir].value = 0;
	    }
	}
	for (child = box->u.box.firstChild; child; child = child->nextSibling)
	{
	    if (child->type == VariableBox)
		continue;
	    ComputeNaturalSizes (l, child, box->u.box.dir);
	    for (thisDir = LayoutHorizontal; thisDir <= LayoutVertical; thisDir++)
	    {
		if (thisDir == box->u.box.dir)
		{
		    /*
		     * along box axis:
		     *  normal size += child normal size
		     *  shrink += child shrink
		     *  stretch += child stretch
		     */
		    box->natural[thisDir] += child->natural[thisDir];
		    AddGlue (box->params.shrink[thisDir],
			     box->params.shrink[thisDir],
			     child->params.shrink[thisDir]);
		    AddGlue (box->params.stretch[thisDir],
			     box->params.stretch[thisDir],
			     child->params.stretch[thisDir]);
		}
		else
		{
		    /*
		     * normal to box axis:
		     *  normal size = maximum child normal size of minimum shrink order
		     *  shrink = difference between normal size and minimum shrink
		     *  stretch = minimum child stretch
		     */
		    if (box->natural[thisDir] >= child->natural[thisDir])
		    {
			if (child->params.stretch[thisDir].order < minShrinkOrder)
			{
			    box->natural[thisDir] = child->natural[thisDir];
			    minStretchOrder = child->params.stretch[thisDir].order;
			    if (child->params.shrink[thisDir].order < minShrinkOrder)
				minShrinkOrder = child->params.shrink[thisDir].order;
			}
		    }
		    else
		    {
			if (child->params.shrink[thisDir].order <= minStretchOrder)
			{
			    box->natural[thisDir] = child->natural[thisDir];
			    minShrinkOrder = child->params.shrink[thisDir].order;
			    if (child->params.stretch[thisDir].order < minStretchOrder)
				minStretchOrder = child->params.stretch[thisDir].order;
			}
		    }
		    MinGlue (box->params.stretch[thisDir],
			     box->params.stretch[thisDir],
			     child->params.stretch[thisDir]);
		    MinGlue (box->params.shrink[thisDir],
			     box->params.shrink[thisDir],
			     child->params.shrink[thisDir]);
		}
	    }
	}
	for (thisDir = LayoutHorizontal; thisDir <= LayoutVertical; thisDir++)
	{
	    if (thisDir != box->u.box.dir)
	    {
		if (box->params.shrink[thisDir].order <= 0)
		{
		    int	    minSize;
		    int	    largestMinSize;

		    largestMinSize = 0;
		    for (child = box->u.box.firstChild; child; child = child->nextSibling)
		    {
			if (child->type == VariableBox)
			    continue;
			if (child->params.shrink[thisDir].order <= 0)
			{
			    minSize = child->natural[thisDir] -
			    child->params.shrink[thisDir].value;
			    if (minSize > largestMinSize)
				largestMinSize = minSize;
			}
		    }
		    box->params.shrink[thisDir].value = box->natural[thisDir] -
		    largestMinSize;
		    if (box->params.shrink[thisDir].value == 0)
			box->params.shrink[thisDir].order = -1;
		    else
			box->params.shrink[thisDir].order = 0;
		}
	    }
	}
	break;
    case VariableBox:
	box->natural[LayoutHorizontal] = 0;
	box->natural[LayoutVertical] = 0;
	break;
    }
}

/* given the boxs geometry, set the geometry of the pieces */

#define GluePart(a,b,dist)	((a) ? ((int) (((a) * (dist)) / (b) + \
					((dist >= 0) ? 0.5 : -0.5))) : 0)

static Bool
ComputeSizes (BoxPtr box)
{
    LayoutDirection dir;
    BoxPtr	    child;
    GlueRec	    stretch;
    GlueRec	    shrink;
    GlueRec	    totalGlue[2];
    double	    remainingGlue = 0.0;
    GluePtr	    glue;
    int		    size;
    int		    totalSizes;
    int		    finalSize[2];
    int		    totalChange[2];
    int		    change;
    int		    remainingChange = 0;
    Bool	    shrinking = 0;
    Bool	    happy;
    int		    i;
    int		    maxGlue = 0;

    happy = True;
    for (dir = LayoutHorizontal; dir <= LayoutVertical; dir++)
    {
	if (dir == box->u.box.dir)
	{
	    size = box->size[dir];

	    stretch = box->params.stretch[dir];
	    shrink = box->params.shrink[dir];

	    /* pick the correct adjustment parameters based on the change direction */

	    totalChange[0] = size - box->natural[dir];

	    shrinking = totalChange[0] < 0;

	    totalChange[1] = 0;
	    totalGlue[1].order = 100000;
	    totalGlue[1].value = 0;
	    maxGlue = 1;
	    if (shrinking)
	    {
		totalGlue[0] = shrink;
		/* for first-order infinites, shrink it to zero and then
		 * shrink the zero-orders
		 */
		if (shrink.order == 1) {
		    totalSizes = 0;
		    remainingGlue = 0;
		    for (child = box->u.box.firstChild;
			 child;
			 child = child->nextSibling)
		    {
			if (child->type == VariableBox)
			    continue;
			switch (child->params.shrink[dir].order) {
			case 0:
			    remainingGlue += child->params.shrink[dir].value;
			    break;
			case 1:
			    totalSizes += child->natural[dir];
			    break;
			}
		    }
		    /*
		     * If the amount of first-order shrinkability is less
		     * than the change in size, shrink the zero-order objects
		     * by the difference
		     */
		    if (totalSizes < -totalChange[0])
		    {
			totalGlue[1] = shrink;
			totalGlue[0].order = 0;
			totalGlue[0].value = remainingGlue;
			totalChange[1] = -totalSizes;
			totalChange[0] = totalChange[0] - totalChange[1];
			/*
			 * whoa -- running out of both zero and first order
			 * shrink; shrink zero by max, shrink first by rest
			 */
			if (remainingGlue < -totalChange[0])
			{
			    totalChange[1] += (totalChange[0] + remainingGlue);
			    totalChange[0] = -remainingGlue;
			}
			maxGlue = 2;
		    }
		}
		if (totalGlue[0].order <= 0 &&
		    -totalChange[0] > totalGlue[0].value)
		{
		    totalChange[0] = -totalGlue[0].value;
		}
	    }
	    else
		totalGlue[0] = stretch;
	}
    }

    /* adjust each box */
    for (dir = LayoutHorizontal; dir <= LayoutVertical; dir++)
    {
	finalSize[dir] = 0;
	if (dir == box->u.box.dir)
	{
	    remainingGlue = totalGlue[0].value + totalGlue[1].value;
	    remainingChange = totalChange[0] + totalChange[1];
	}
    }
    for (child = box->u.box.firstChild; child; child = child->nextSibling)
    {
	if (child->type == VariableBox)
	    continue;
	for (dir = LayoutHorizontal; dir <= LayoutVertical; dir++)
	{
	    if (dir == box->u.box.dir)
	    {
		if (shrinking)
		    glue = &child->params.shrink[dir];
		else
		    glue = &child->params.stretch[dir];

		child->size[dir] = child->natural[dir];
		for (i = 0; i < maxGlue; i++)
		{
		    if (glue->order == totalGlue[i].order)
		    {
			remainingGlue -= glue->value;
			if (remainingGlue <= 0)
			    change = remainingChange;
			else
			    change = GluePart (glue->value,
					       totalGlue[i].value,
					       totalChange[i]);
			if (glue->order == -1 && change)
			    happy = False;
			child->size[dir] += change;
			remainingChange -= change;
		    }
		}
		finalSize[dir] += child->size[dir];
	    }
	    else
	    {
		child->size[dir] = box->size[dir];
		finalSize[dir] = box->size[dir];
	    }
	}
	if (child->type == BoxBox)
	    if (!ComputeSizes (child))
		happy = False;
    }
    for (dir = LayoutHorizontal; dir <= LayoutVertical; dir++)
	if (finalSize[dir] != box->size[dir])
	    happy = False;
    return happy;
}

static void
SetSizes (BoxPtr box, Position x, Position y)
{
    BoxPtr	child;
    int		width, height;
    int		bw;
    Widget	w;
    SubInfoPtr	info;

    switch (box->type) {
    case WidgetBox:
	w = box->u.widget.widget;
	if (w)
	{
	    info = (SubInfoPtr) w->core.constraints;
	    width = box->size[LayoutHorizontal];
	    height = box->size[LayoutVertical];
	    bw = info->naturalBw;
	    width -= bw * 2;
	    height -= bw * 2;
	    /* Widgets which grow too small are placed off screen */
	    if (width <= 0 || height <= 0)
	    {
		width = 1;
		height = 1;
		bw = 0;
		x = -1;
		y = -1;
	    }
	    XtConfigureWidget (w, x, y, width, height, bw);
	}
	break;
    case GlueBox:
	break;
    case BoxBox:
	for (child = box->u.box.firstChild; child; child = child->nextSibling)
	{
	    if (child->type == VariableBox)
		continue;
	    SetSizes (child, x, y);
	    if (box->u.box.dir == LayoutHorizontal)
		x += child->size[LayoutHorizontal];
	    else if (box->u.box.dir == LayoutVertical)
		y += child->size[LayoutVertical];
	}
	break;
    case VariableBox:
	break;
    }
}

static void
LayoutFreeLayout (BoxPtr box)
{
    BoxPtr  child, next;

    switch (box->type) {
    default:
	break;
    case BoxBox:
	for (child = box->u.box.firstChild; child; child = next)
	{
	    next = child->nextSibling;
	    LayoutFreeLayout (child);
	}
	break;
    case GlueBox:
	DisposeExpr (box->u.glue.expr[LayoutHorizontal]);
	DisposeExpr (box->u.glue.expr[LayoutVertical]);
	break;
    }
    DisposeExpr (box->params.stretch[LayoutHorizontal].expr);
    DisposeExpr (box->params.shrink[LayoutHorizontal].expr);
    DisposeExpr (box->params.stretch[LayoutVertical].expr);
    DisposeExpr (box->params.shrink[LayoutVertical].expr);
    Dispose (box);
}


static void
LayoutGetNaturalSize (LayoutWidget l, Dimension *widthp, Dimension *heightp)
{
    BoxPtr		box;

    box = l->layout.layout;
    if (box)
    {
	ComputeNaturalSizes (l, box, LayoutHorizontal);
	*widthp = box->natural[LayoutHorizontal];
	*heightp = box->natural[LayoutVertical];
    }
    else
    {
	*widthp = 0;
	*heightp = 0;
    }
}

static void
LayoutLayout (LayoutWidget l, Bool attemptResize)
{
    BoxPtr		box;
    Dimension		prefered_width, prefered_height;

    box = l->layout.layout;
    if (!box)
	return;
    prefered_width = l->layout.prefered_width;
    prefered_height = l->layout.prefered_height;
    LayoutGetNaturalSize (l, &l->layout.prefered_width, &l->layout.prefered_height);
    if (l->core.width == 0 || l->core.height == 0)
    {
	l->core.width = l->layout.prefered_width;
	l->core.height = l->layout.prefered_height;
    }
    if (prefered_width == 0)
	prefered_width = l->layout.prefered_width;
    if (prefered_height == 0)
	prefered_height = l->layout.prefered_height;
    if (attemptResize && l->layout.resize &&
	(l->layout.prefered_width != prefered_width ||
	 l->layout.prefered_height != prefered_height))
    {
	(void) XtMakeResizeRequest ((Widget) l,
				    l->layout.prefered_width,
				    l->layout.prefered_height,
				    NULL, NULL);
    }
    box->size[LayoutHorizontal] = l->core.width;
    box->size[LayoutVertical] = l->core.height;
    ComputeSizes (box);
    if (l->layout.debug)
    {
	printf ("Layout widget %s\n", XtName ((Widget) l));
	PrintBox (box, 1);
	fflush (stdout);
    }
    SetSizes (box, 0, 0);
    if (l->layout.debug)
	printf ("Layout widget done with %s\n", XtName((Widget) l));
}

LayoutClassRec layoutClassRec = {
   {
/* core class fields */
    /* superclass         */   (WidgetClass) SuperClass,
    /* class name         */   "Layout",
    /* size               */   sizeof(LayoutRec),
    /* class_initialize   */   ClassInitialize,
    /* class_part init    */   NULL,
    /* class_inited       */   FALSE,
    /* initialize         */   Initialize,
    /* initialize_hook    */   NULL,
    /* realize            */   XtInheritRealize,
    /* actions            */   NULL,
    /* num_actions        */   0,
    /* resources          */   resources,
    /* resource_count     */   XtNumber(resources),
    /* xrm_class          */   NULLQUARK,
    /* compress_motion    */   0,
    /* compress_exposure  */   0,
    /* compress_enterleave*/   0,
    /* visible_interest   */   FALSE,
    /* destroy            */   NULL,
    /* resize             */   Resize,
    /* expose             */   NULL,
    /* set_values         */   SetValues,
    /* set_values_hook    */   NULL,
    /* set_values_almost  */   XtInheritSetValuesAlmost,
    /* get_values_hook    */   NULL,
    /* accept_focus       */   NULL,
    /* version            */   XtVersion,
    /* callback_private   */   NULL,
    /* tm_table           */   NULL,
    /* query_geometry	  */   QueryGeometry,
    /* display_accelerator*/   XtInheritDisplayAccelerator,
    /* extension          */   NULL
   }, {
/* composite class fields */
    /* geometry_manager   */   GeometryManager,
    /* change_managed     */   ChangeManaged,
    /* insert_child       */   InsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
   }, {
/* constraint class fields */
    /* subresources       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(LayoutConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
   },
#ifdef MOTIF
    /* manager class fields */
    {
	/* stolen from the one minute manager's Template.c */
	XtInheritTranslations,                    /* translations          */
	NULL,                                     /* syn_resources         */
	0,                                        /* num_syn_resources     */
	NULL,                                     /* syn_constraint_resources */
	0,                                        /* num_syn_constraint_resources */    XmInheritParentProcess,                   /* parent_process        */
	NULL,                                     /* extension             */
   },
#endif /* MOTIF */
    /* layout class fields */
    {
	0
    },
};

WidgetClass layoutWidgetClass = (WidgetClass) &layoutClassRec;
