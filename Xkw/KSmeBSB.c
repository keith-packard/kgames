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
 * KSmeBSB.c - Source code file for BSB Menu Entry object.
 *
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
#include <X11/Xos.h>
#include <X11/Xmu/Drawing.h>
#include <X11/Xaw/Cardinals.h>
#include <Xkw/KSimpleMenu.h>
#include <Xkw/KSmeBSBP.h>
#include <Xkw/Xkw.h>

#define ONE_HUNDRED 100

/*
 * Class Methods
 */
static void FlipColors(Widget);
static void XkwKSmeBSBClassInitialize(void);
static void XkwKSmeBSBInitialize(Widget, Widget, ArgList, Cardinal*);
static void XkwKSmeBSBDestroy(Widget);
static XtGeometryResult XkwKSmeBSBQueryGeometry(Widget, XtWidgetGeometry*,
					       XtWidgetGeometry*);
static void XkwKSmeBSBRedisplay(Widget, XEvent*, Region);
static Boolean XkwKSmeBSBSetValues(Widget, Widget, Widget,
				  ArgList, Cardinal*);

/*
 * Prototypes
 */
static void GetBitmapInfo(Widget, Bool);
static void GetDefaultSize(Widget, Dimension*, Dimension*);
static void DrawBitmaps(Widget, cairo_t *);

/*
 * Initialization
 */
#define offset(field) XtOffsetOf(KSmeBSBRec, ksme_bsb.field)
static XtResource resources[] = {
  {
    XtNlabel,
    XtCLabel,
    XtRString,
    sizeof(String),
    offset(label),
    XtRString,
    NULL
  },
  {
    XtNvertSpace,
    XtCVertSpace,
    XtRInt,
    sizeof(int),
    offset(vert_space),
    XtRImmediate,
    (XtPointer)25
  },
  {
    XtNleftBitmap,
    XtCLeftBitmap,
    XtRBitmap,
    sizeof(Pixmap),
    offset(left_bitmap),
    XtRImmediate,
    (XtPointer)None
  },
  {
    XtNjustify,
    XtCJustify,
    XtRJustify,
    sizeof(XtJustify),
    offset(justify),
    XtRImmediate,
    (XtPointer)XtJustifyLeft
  },
  {
    XtNrightBitmap,
    XtCRightBitmap,
    XtRBitmap,
    sizeof(Pixmap),
    offset(right_bitmap),
    XtRImmediate,
    (XtPointer)None
  },
  {
    XtNleftMargin,
    XtCHorizontalMargins,
    XtRDimension,
    sizeof(Dimension),
    offset(left_margin),
    XtRImmediate,
    (XtPointer)4
  },
  {
    XtNrightMargin,
    XtCHorizontalMargins,
    XtRDimension,
    sizeof(Dimension),
    offset(right_margin),
    XtRImmediate,
    (XtPointer)4
  },
    { XtNbackgroundColor, XtCBackground, XtRRenderColor, sizeof (XRenderColor),
      offset (background), XtRString, XtDefaultBackground },
    { XtNforegroundColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (foreground), XtRString, XtDefaultForeground },
    { XtNdpi, XtCDpi, XtRDpi, sizeof(double),
      offset (dpi), XtRString, "" },
    { XtNfont, XtCFont, XtRXkwFont, sizeof (XkwFont),
      offset (font), XtRString, XtDefaultFont },
};
#undef offset

#define superclass (&ksmeClassRec)
KSmeBSBClassRec ksmeBSBClassRec = {
  /* rectangle */
  {
    (WidgetClass)superclass,		/* superclass */
    "KSmeBSB",				/* class_name */
    sizeof(KSmeBSBRec),			/* size */
    XkwKSmeBSBClassInitialize,		/* class_init */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    XkwKSmeBSBInitialize,		/* initialize */
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
    XkwKSmeBSBDestroy,			/* destroy */
    XtInheritResize,			/* resize */
    XkwKSmeBSBRedisplay,		/* expose */
    XkwKSmeBSBSetValues,		/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* intrinsics version */
    NULL,				/* callback offsets */
    NULL,				/* tm_table */
    XkwKSmeBSBQueryGeometry,		/* query_geometry */
    NULL,				/* display_accelerator */
    NULL,				/* extension */
  },
  /* sme */
  {
    FlipColors,				/* highlight */
    FlipColors,				/* unhighlight */
    XtInheritNotify,			/* notify */
    NULL,				/* extension */
  },
  /* sme_bsb */
  {
    NULL,				/* extension */
  },
};
WidgetClass ksmeBSBObjectClass = (WidgetClass)&ksmeBSBClassRec;

/*
 * Function:
 *	XkwKSmeBSBClassInitialize
 *
 * Description:
 *	Initializes the KSmeBSBObject.
 */
static void
XkwKSmeBSBClassInitialize(void)
{
    XkwInitializeWidgetSet();
    XtAddConverter(XtRString, XtRJustify, XmuCvtStringToJustify, NULL, 0);
    XtSetTypeConverter(XtRJustify, XtRString, XmuCvtJustifyToString,
		       NULL, 0, XtCacheNone, NULL);
}

/*
 * Function:
 *	XkwKSmeBSBInitialize
 *
 * Parameters:
 *	request	- widget requested by the argument list
 *	cnew	- new widget with both resource and non resource values
 *
 * Description:
 *	Initializes the simple menu widget entry.
 */
/*ARGSUSED*/
static void
XkwKSmeBSBInitialize(Widget request, Widget cnew,
		    ArgList args, Cardinal *num_args)
{
    KSmeBSBObject entry = (KSmeBSBObject)cnew;

    if (entry->ksme_bsb.label == NULL)
	entry->ksme_bsb.label = XtName(cnew);
    else
	entry->ksme_bsb.label = XtNewString(entry->ksme_bsb.label);

    GetDefaultSize(cnew, &entry->rectangle.width, &entry->rectangle.height);

    entry->ksme_bsb.left_bitmap_width = entry->ksme_bsb.left_bitmap_height = 0;
    entry->ksme_bsb.right_bitmap_width = entry->ksme_bsb.right_bitmap_height = 0;

    GetBitmapInfo(cnew, True);	/* Left Bitmap Info */
    GetBitmapInfo(cnew, False);	/* Right Bitmap Info */
}

/*
 * Function:
 *	XkwKSmeBSBDestroy
 *
 * Parameters:
 *	w - simple menu widget entry
 */
static void
XkwKSmeBSBDestroy(Widget w)
{
    KSmeBSBObject entry = (KSmeBSBObject)w;

    if (entry->ksme_bsb.label != XtName(w))
	XtFree(entry->ksme_bsb.label);
}

static void
init_cairo(KSmeBSBObject w, cairo_t *cr)
{
    cairo_set_font_face(cr, w->ksme_bsb.font.font_face);
    cairo_set_font_size(cr, w->ksme_bsb.font.size * w->ksme_bsb.dpi / 72.0);
}

static cairo_t *
get_cairo(KSmeBSBObject w)
{
    cairo_t *cr = XkwGetCairo((Widget) w);
    init_cairo(w, cr);
    return cr;
}

static cairo_t *
draw_begin(KSmeBSBObject w, Region region)
{
    cairo_t *cr = XkwDrawBegin((Widget) w, region);
    init_cairo(w, cr);
    return cr;
}

static void
draw_end(KSmeBSBObject w, Region region, cairo_t *cr)
{
    XkwDrawEnd((Widget) w, region, cr);
}

/*
 * Function:
 *	XkwKSmeBSBRedisplay
 *
 * Parameters:
 *	w      - simple menu widget entry
 *	event  - X event that caused this redisplay
 *	region - region the needs to be repainted
 *
 * Description:
 *	Redisplays the contents of the widget.
 */
/* ARGSUSED */
static void
XkwKSmeBSBRedisplay(Widget w, XEvent *event, Region region)
{
    KSmeBSBObject entry = (KSmeBSBObject)w;

    cairo_t *cr = draw_begin(entry, region);

    if (XtIsSensitive(w) && XtIsSensitive(XtParent(w))) {
	if (w == XkwKSimpleMenuGetActiveEntry(XtParent(w))) {
	    XkwSetSource(cr, &entry->ksme_bsb.foreground);
	    cairo_rectangle(cr, 0, 0,
			    XtWidth(entry), XtHeight(entry));
	    cairo_fill(cr);
	    XkwSetSource(cr, &entry->ksme_bsb.background);
	}
    }
    else
	XkwSetSourceInterp(cr, &entry->ksme_bsb.foreground, &entry->ksme_bsb.background);

    if (entry->ksme_bsb.label != NULL) {
	int margin = entry->ksme_bsb.left_margin;

	cairo_text_extents_t text_extents;
	cairo_font_extents_t font_extents;
	cairo_font_extents(cr, &font_extents);
	cairo_text_extents(cr, entry->ksme_bsb.label, &text_extents);

	double x;
	switch (entry->ksme_bsb.justify) {
	case XtJustifyLeft:
	    x = margin;
	    break;
	case XtJustifyRight:
	    x = XtWidth(entry) - text_extents.width - margin;
	    break;
	default:
	    x = (XtWidth(entry) - text_extents.width) / 2 - text_extents.x_bearing;
	    break;
	}
	double y = (XtHeight(entry) - text_extents.height) / 2 - text_extents.y_bearing;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, entry->ksme_bsb.label);
    }

    DrawBitmaps(w, cr);
    draw_end(entry, region, cr);
}


/*
 * Function:
 *	XkwKSmeBSBSetValues
 *
 * Parameters:
 *	current	- current state of the widget
 *	request	- what was requested
 *	cnew	- what the widget will become
 *
 * Description:
 *	Relayout the menu when one of the resources is changed.
 */

/*ARGSUSED*/
static Boolean
XkwKSmeBSBSetValues(Widget current, Widget request, Widget cnew,
		   ArgList args, Cardinal *num_args)
{
    KSmeBSBObject entry = (KSmeBSBObject)cnew;
    KSmeBSBObject old_entry = (KSmeBSBObject)current;
    Boolean ret_val = False;

    if (old_entry->ksme_bsb.label != entry->ksme_bsb.label) {
	if (old_entry->ksme_bsb.label != XtName(cnew))
	    XtFree((char *)old_entry->ksme_bsb.label);

	if (entry->ksme_bsb.label != XtName(cnew))
	    entry->ksme_bsb.label = XtNewString(entry->ksme_bsb.label);

	ret_val = True;
    }

    if (entry->rectangle.sensitive != old_entry->rectangle.sensitive)
	ret_val = True;

    if (entry->ksme_bsb.left_bitmap != old_entry->ksme_bsb.left_bitmap) {
	GetBitmapInfo(cnew, True);
	ret_val = True;
    }

    if (entry->ksme_bsb.right_bitmap != old_entry->ksme_bsb.right_bitmap) {
	GetBitmapInfo(cnew, False);
	ret_val = True;
    }

    if (memcpy(&old_entry->ksme_bsb.foreground, &entry->ksme_bsb.foreground, sizeof(XRenderColor)) != 0 ||
	memcpy(&old_entry->ksme_bsb.background, &entry->ksme_bsb.background, sizeof(XRenderColor)) != 0)
	ret_val = True;

    if (old_entry->ksme_bsb.font.font_face != entry->ksme_bsb.font.font_face ||
	old_entry->ksme_bsb.font.size != entry->ksme_bsb.font.size)
    {
	ret_val = True;
    }

    if (ret_val) {
	Dimension width, height;

	GetDefaultSize(cnew, &width, &height);
	XtMakeResizeRequest(cnew, width, height, NULL, NULL);
    }

    return (ret_val);
}

/*
 * Function:
 *	XkwKSmeBSBQueryGeometry
 *
 * Parameters:
 *	w	   - menu entry object
 *	itended	   - intended and return geometry info
 *	return_val - ""
 *
 * Returns:
 *	Geometry Result
 *
 * Description:
 *	  Returns the preferred geometry for this widget.
 *	  See the Intrinsics manual for details on what this function is for.
 */
static XtGeometryResult
XkwKSmeBSBQueryGeometry(Widget w, XtWidgetGeometry *intended,
		       XtWidgetGeometry *return_val)
{
    KSmeBSBObject entry = (KSmeBSBObject)w;
    Dimension width, height;
    XtGeometryResult ret_val = XtGeometryYes;
    XtGeometryMask mode = intended->request_mode;

    GetDefaultSize(w, &width, &height);

    if (((mode & CWWidth) && intended->width != width) || !(mode & CWWidth)) {
	return_val->request_mode |= CWWidth;
	return_val->width = width;
	ret_val = XtGeometryAlmost;
    }

    if (((mode & CWHeight) && intended->height != height) || !(mode & CWHeight)) {
	return_val->request_mode |= CWHeight;
	return_val->height = height;
	ret_val = XtGeometryAlmost;
    }

    if (ret_val == XtGeometryAlmost) {
	mode = return_val->request_mode;
	if (((mode & CWWidth) && width == XtWidth(entry)) &&
	    ((mode & CWHeight) && height == XtHeight(entry)))
	    return (XtGeometryNo);
    }

    return (ret_val);
}

/*
 * Function:
 *	FlipColors
 *
 * Parameters:
 *	w - bsb menu entry widget
 *
 * Description:
 *	Invert the colors of the current entry.
 */
static void
FlipColors(Widget w)
{
    XkwKSmeBSBRedisplay(w, NULL, NULL);
}

/*
 * Function:
 *	GetDefaultSize
 *
 * Parameters:
 *	w      - menu entry widget.
 *	width  - default width (return)
 *	height - default height (return)
 *
 * Description:
 *	Calculates the Default (preferred) size of this menu entry.
 */
static void
GetDefaultSize(Widget w, Dimension *width, Dimension *height)
{
    KSmeBSBObject entry = (KSmeBSBObject)w;

    cairo_t *cr = get_cairo(entry);
    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    cairo_text_extents(cr, entry->ksme_bsb.label, &text_extents);

    cairo_destroy(cr);
    if (entry->ksme_bsb.label == NULL)
	*width = 0;
    else
	*width = text_extents.width + entry->ksme_bsb.left_margin + entry->ksme_bsb.right_margin;

    *height = font_extents.height * (1.0  + entry->ksme_bsb.vert_space / 100.0);
}

/*
 * Function:
 *	DrawBitmaps
 *
 * Parameters:
 *	w  - simple menu widget entry
 *	gc - graphics context to use for drawing
 *
 * Description:
 *	Draws left and right bitmaps.
 */
static void
DrawBitmaps(Widget w, cairo_t *cr)
{
    KSmeBSBObject entry = (KSmeBSBObject)w;

    if (entry->ksme_bsb.left_bitmap == None &&
	entry->ksme_bsb.right_bitmap == None)
	return;

#if 0
    int x_loc, y_loc;
    /*
     * Draw Left Bitmap
     */
    if (entry->ksme_bsb.left_bitmap != None) {
	x_loc = ((entry->ksme_bsb.left_margin -
		  entry->ksme_bsb.left_bitmap_width) >> 1) + XtX(w);

	y_loc = XtY(entry) + ((XtHeight(entry) -
			       entry->ksme_bsb.left_bitmap_height) >> 1);

	XCopyPlane(XtDisplayOfObject(w), entry->ksme_bsb.left_bitmap,
		   XtWindowOfObject(w), gc, 0, 0,
		   entry->ksme_bsb.left_bitmap_width,
		   entry->ksme_bsb.left_bitmap_height, x_loc, y_loc, 1);
    }

    /*
     * Draw Right Bitmap
     */
    if (entry->ksme_bsb.right_bitmap != None) {
	x_loc = XtWidth(entry) - ((entry->ksme_bsb.right_margin +
				  entry->ksme_bsb.right_bitmap_width) >> 1) +
				  XtX(w);
	y_loc = XtY(entry) + ((XtHeight(entry) -
			      entry->ksme_bsb.right_bitmap_height) >> 1);

	XCopyPlane(XtDisplayOfObject(w), entry->ksme_bsb.right_bitmap,
		   XtWindowOfObject(w), gc, 0, 0,
		   entry->ksme_bsb.right_bitmap_width,
	 	   entry->ksme_bsb.right_bitmap_height, x_loc, y_loc, 1);
    }
#endif
}

/*
 * Function:
 *	GetBitmapInfo
 *
 * Parameters:
 *	w	- bsb menu entry object
 *	is_left - True: if we are testing left bitmap
 *		  False: if we are testing the right bitmap
 *
 * Description:
 *	Gets the bitmap information from either of the bitmaps.
 */
static void
GetBitmapInfo(Widget w, Bool is_left)
{
    KSmeBSBObject entry = (KSmeBSBObject)w;
    unsigned int depth, bw;
    Window root;
    int x, y;
    unsigned int width, height;

    if (is_left) {
	if (entry->ksme_bsb.left_bitmap != None &&
	    XGetGeometry(XtDisplayOfObject(w),
			 entry->ksme_bsb.left_bitmap, &root,
			 &x, &y, &width, &height, &bw, &depth))	{
	    entry->ksme_bsb.left_bitmap_width = width;
	    entry->ksme_bsb.left_bitmap_height = height;
	}
    }
    else if (entry->ksme_bsb.right_bitmap != None &&
	     XGetGeometry(XtDisplayOfObject(w),
			  entry->ksme_bsb.right_bitmap, &root,
			  &x, &y, &width, &height, &bw, &depth)) {
	entry->ksme_bsb.right_bitmap_width = width;
	entry->ksme_bsb.right_bitmap_height = height;
    }
}
