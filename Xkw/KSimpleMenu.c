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
 * SimpleMenu.c - Source code file for SimpleMenu widget.
 *
 * Date:    April 3, 1989
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
#include <X11/Xmu/Initer.h>
#include <X11/Xaw/Cardinals.h>
#include <Xkw/KSimpleMenuP.h>
#include <Xkw/KSmeBSBP.h>
#include <Xkw/Xkw.h>

#define streq(a, b)	(strcmp((a), (b)) == 0)

#define ForAllChildren(smw, childP)				\
for ((childP) = (KSmeObject *)(smw)->composite.children;		\
     (childP) < (KSmeObject *)((smw)->composite.children		\
			      + (smw)->composite.num_children);	\
     (childP)++)

#ifndef OLDXAW
#define	SMW_UNMAPPING	0x01
#define SMW_POPLEFT	0x02
#endif

/*
 * Class Methods
 */
static void XkwKSimpleMenuChangeManaged(Widget);
static void XkwKSimpleMenuClassInitialize(void);
static void XkwKSimpleMenuClassPartInitialize(WidgetClass);
static XtGeometryResult XkwKSimpleMenuGeometryManager(Widget, XtWidgetGeometry*,
						     XtWidgetGeometry*);
static void XkwKSimpleMenuInitialize(Widget, Widget, ArgList, Cardinal*);
static void XkwKSimpleMenuRealize(Widget, XtValueMask*, XSetWindowAttributes*);
static void XkwKSimpleMenuRedisplay(Widget, XEvent*, Region);
static void XkwKSimpleMenuResize(Widget);
static Boolean XkwKSimpleMenuSetValues(Widget, Widget, Widget,
				      ArgList, Cardinal*);
static Boolean XkwKSimpleMenuSetValuesHook(Widget, ArgList, Cardinal*);
#define OLDXAW
#ifndef OLDXAW
static void PopupSubMenu(SimpleMenuWidget);
static void PopdownSubMenu(SimpleMenuWidget);
static void PopupCB(Widget, XtPointer, XtPointer);
#endif

/*
 * Prototypes
 */
static void AddPositionAction(XtAppContext, XPointer);
static void CalculateNewSize(Widget, Dimension*, Dimension*);
static void ChangeCursorOnGrab(Widget, XtPointer, XtPointer);
static void CreateLabel(Widget);
static KSmeObject DoGetEventEntry(Widget, int, int);
static Widget FindMenu(Widget, String);
static KSmeObject GetEventEntry(Widget, XEvent*);
static void Layout(Widget, Dimension*, Dimension*);
static void MakeResizeRequest(Widget);
static void MakeSetValuesRequest(Widget, unsigned int, unsigned int);
static void MoveMenu(Widget, int, int);
static void PositionMenu(Widget, XPoint*);

#define OLDXAW
/*
 * Actions
 */
static void Highlight(Widget, XEvent*, String*, Cardinal*);
static void Notify(Widget, XEvent*, String*, Cardinal*);
#ifndef OLDXAW
static void Popdown(Widget, XEvent*, String*, Cardinal*);
#endif
static void PositionMenuAction(Widget, XEvent*, String*, Cardinal*);
static void Unhighlight(Widget, XEvent*, String*, Cardinal*);

/*
 * Initialization
 */
#define offset(field)	XtOffsetOf(KSimpleMenuRec, ksimple_menu.field)

static XtResource resources[] = {
  /* label */
  {
    XtNlabel,
    XtCLabel,
    XtRString,
    sizeof(String),
    offset(label_string),
    XtRString,
    NULL
  },
  {
    XtNlabelClass,
    XtCLabelClass,
    XtRPointer,
    sizeof(WidgetClass),
    offset(label_class),
    XtRImmediate,
    NULL
  },

  /* layout */
  {
    XtNrowHeight,
    XtCRowHeight,
    XtRDimension,
    sizeof(Dimension),
    offset(row_height),
    XtRImmediate,
    (XtPointer)0
  },
  {
    XtNtopMargin,
    XtCVerticalMargins,
    XtRDimension,
    sizeof(Dimension),
    offset(top_margin),
    XtRImmediate,
    (XtPointer)0
  },
  {
    XtNbottomMargin,
    XtCVerticalMargins,
    XtRDimension,
    sizeof(Dimension),
    offset(bottom_margin),
    XtRImmediate,
    (XtPointer)0
  },
    { XtNbackgroundColor, XtCBackground, XtRRenderColor, sizeof (XRenderColor),
      offset (background), XtRString, XtDefaultBackground },
    { XtNforegroundColor, XtCForeground, XtRRenderColor, sizeof (XRenderColor),
      offset (foreground), XtRString, XtDefaultForeground },
#ifndef OLDXAW
  {
    XtNleftMargin,
    XtCHorizontalMargins,
    XtRDimension,
    sizeof(Dimension),
    offset(left_margin),
    XtRImmediate,
    (XtPointer)0
  },
  {
    XtNrightMargin,
    XtCHorizontalMargins,
    XtRDimension,
    sizeof(Dimension),
    offset(right_margin),
    XtRImmediate,
    (XtPointer)0
  },
#endif

  /* misc */
  {
    XtNallowShellResize,
    XtCAllowShellResize,
    XtRBoolean,
    sizeof(Boolean),
    XtOffsetOf(KSimpleMenuRec, shell.allow_shell_resize),
    XtRImmediate,
    (XtPointer)True
  },
  {
    XtNcursor,
    XtCCursor,
    XtRCursor,
    sizeof(Cursor),
    offset(cursor),
    XtRImmediate,
    (XtPointer)None
  },
  {
    XtNmenuOnScreen,
    XtCMenuOnScreen,
    XtRBoolean,
    sizeof(Boolean),
    offset(menu_on_screen),
    XtRImmediate,
    (XtPointer)True
  },
  {
    XtNpopupOnEntry,
    XtCPopupOnEntry,
    XtRWidget,
    sizeof(Widget),
    offset(popup_entry),
    XtRWidget,
    NULL
  },
  {
    XtNbackingStore,
    XtCBackingStore,
    XtRBackingStore,
    sizeof(int),
    offset(backing_store),
    XtRImmediate,
    (XtPointer)(Always + WhenMapped + NotUseful)
  },
#ifndef OLDXAW
  {
    XawNdisplayList,
    XawCDisplayList,
    XawRDisplayList,
    sizeof(XawDisplayList*),
    offset(display_list),
    XtRImmediate,
    NULL
  },
#endif
};
#undef offset

static char defaultTranslations[] =
"<Enter>:"	"highlight()\n"
"<Leave>:"	"unhighlight()\n"
"<BtnMotion>:"	"highlight()\n"
#ifndef OLDXAW
"<BtnUp>:"	"popdown() notify() unhighlight()\n"
#else
"<BtnUp>:"	"MenuPopdown() notify() unhighlight()\n"
#endif
;

static XtActionsRec actionsList[] =
{
  {"notify",            Notify},
  {"highlight",         Highlight},
  {"unhighlight",       Unhighlight},
#ifndef OLDXAW
  {"popdown",		Popdown},
  {"set-values",	XawSetValuesAction},
  {"get-values",	XawGetValuesAction},
  {"declare",		XawDeclareAction},
  {"call-proc",		XawCallProcAction},
#endif
};

static CompositeClassExtensionRec extension_rec = {
  NULL,					/* next_extension */
  NULLQUARK,				/* record_type */
  XtCompositeExtensionVersion,		/* version */
  sizeof(CompositeClassExtensionRec),	/* record_size */
  True,					/* accepts_objects */
};

#define Superclass	(&overrideShellClassRec)
KSimpleMenuClassRec ksimpleMenuClassRec = {
  /* core */
  {
    (WidgetClass)Superclass,		/* superclass */
    "SimpleMenu",			/* class_name */
    sizeof(KSimpleMenuRec),		/* size */
    XkwKSimpleMenuClassInitialize,	/* class_initialize */
    XkwKSimpleMenuClassPartInitialize,	/* class_part_initialize */
    False,				/* class_inited */
    XkwKSimpleMenuInitialize,		/* initialize */
    NULL,				/* initialize_hook */
    XkwKSimpleMenuRealize,		/* realize */
    actionsList,			/* actions */
    XtNumber(actionsList),		/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    NULL,				/* destroy */
    XkwKSimpleMenuResize,		/* resize */
    XkwKSimpleMenuRedisplay,		/* expose */
    XkwKSimpleMenuSetValues,		/* set_values */
    XkwKSimpleMenuSetValuesHook,		/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* intrinsics version */
    NULL,				/* callback offsets */
    defaultTranslations,		/* tm_table */
    NULL,				/* query_geometry */
    NULL,				/* display_accelerator */
    NULL,				/* extension */
  },
  /* composite */
  {
    XkwKSimpleMenuGeometryManager,	/* geometry_manager */
    XkwKSimpleMenuChangeManaged,		/* change_managed */
    XtInheritInsertChild,		/* insert_child */
    XtInheritDeleteChild,		/* delete_child */
    NULL,				/* extension */
  },
  /* shell */
  {
    NULL,				/* extension */
  },
  /* override */
  {
    NULL,				/* extension */
  },
  /* ksimple_menu */
  {
    NULL,				/* extension */
  },
};

WidgetClass ksimpleMenuWidgetClass = (WidgetClass)&ksimpleMenuClassRec;

/*
 * Implementation
 */
/*
 * Function:
 *	XkwKSimpleMenuClassInitialize
 *
 * Description:
 *	Class Initialize routine, called only once.
 */
static void
XkwKSimpleMenuClassInitialize(void)
{
    XkwInitializeWidgetSet();
    XtAddConverter(XtRString, XtRBackingStore, XmuCvtStringToBackingStore,
		   NULL, 0);
    XtSetTypeConverter(XtRBackingStore, XtRString, XmuCvtBackingStoreToString,
		       NULL, 0, XtCacheNone, NULL);
    XmuAddInitializer(AddPositionAction, NULL);
}

/*
 * Function:
 *	XkwKSimpleMenuClassPartInitialize
 *      Arguments: wc - the widget class of the subclass.
 *
 * Description:
 *	  Class Part Initialize routine, called for every subclass.  Makes
 *	sure that the subclasses pick up the extension record.
 */
static void
XkwKSimpleMenuClassPartInitialize(WidgetClass wc)
{
    KSimpleMenuWidgetClass smwc = (KSimpleMenuWidgetClass)wc;

    /*
     * Make sure that our subclass gets the extension rec too
     */
    extension_rec.next_extension = smwc->composite_class.extension;
    smwc->composite_class.extension = (XtPointer) &extension_rec;
}

/*
 *  Function:
 *	XkwKSimpleMenuInitialize
 *
 * Parameters:
 *	request - widget requested by the argument list
 *	cnew	- new widget with both resource and non resource values
 *
 * Description:
 *	Initializes the ksimple menu widget.
 */
/*ARGSUSED*/
static void
XkwKSimpleMenuInitialize(Widget request, Widget cnew,
			ArgList args, Cardinal *num_args)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)cnew;
    Dimension width, height;

    XmuCallInitializers(XtWidgetToApplicationContext(cnew));

    if (smw->ksimple_menu.label_class == NULL)
	smw->ksimple_menu.label_class = ksmeBSBObjectClass;

    smw->ksimple_menu.label = NULL;
    smw->ksimple_menu.entry_set = NULL;
    smw->ksimple_menu.recursive_set_values = False;
#ifndef OLDXAW
    smw->ksimple_menu.sub_menu = NULL;
    smw->ksimple_menu.state = 0;

    XtAddCallback(cnew, XtNpopupCallback, PopupCB, NULL);
#endif

    if (smw->ksimple_menu.label_string != NULL)
	CreateLabel(cnew);

    width = height = 0;
    CalculateNewSize(cnew, &width, &height);

    smw->ksimple_menu.menu_width = True;

    if (XtWidth(smw) == 0) {
	smw->ksimple_menu.menu_width = False;
	XtWidth(smw) = width;
    }

    smw->ksimple_menu.menu_height = True;

    if (XtHeight(smw) == 0) {
	smw->ksimple_menu.menu_height = False;
	XtHeight(smw) = height;
    }

    /*
     * Add a popup_callback routine for changing the cursor
     */
    XtAddCallback(cnew, XtNpopupCallback, ChangeCursorOnGrab, NULL);
}

/*
 * Function:
 *	XkwKSimpleMenuRedisplay
 *
 * Parameters:
 *	w      - ksimple menu widget
 *	event  - X event that caused this redisplay
 *	region - region the needs to be repainted
 *
 * Description:
 *	Redisplays the contents of the widget.
 */
/*ARGSUSED*/
static void
XkwKSimpleMenuRedisplay(Widget w, XEvent *event, Region region)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    KSmeObject *entry;
    KSmeObjectClass cclass;

    if (region == NULL) {
	cairo_t *cr = XkwGetCairo(w);
	XkwSetSource(cr, &smw->ksimple_menu.background);
	cairo_paint(cr);
	cairo_destroy(cr);
    }

    /*
     * Check and Paint each of the entries - including the label
     */
    ForAllChildren(smw, entry) {
	if (!XtIsManaged((Widget)*entry))
	    continue;

	if (region != NULL)
	    switch(XRectInRegion(region, XtX(*entry),XtY(*entry),
				 XtWidth(*entry), XtHeight(*entry))) {
		case RectangleIn:
		case RectanglePart:
		    break;
		default:
		    continue;
	    }

	cclass = (KSmeObjectClass)(*entry)->object.widget_class;

	if (cclass->rect_class.expose != NULL)
	    (cclass->rect_class.expose)((Widget)*entry, NULL, NULL);
    }
}

/*
 * Function:
 *	XkwKSimpleMenuRealize
 *
 * Parameters:
 *	w     - ksimple menu widget
 *	mask  - value mask for the window to create
 *	attrs - attributes for the window to create
 *
 * Description:
 *	Realizes the widget.
 */
static void
XkwKSimpleMenuRealize(Widget gw, XtValueMask *mask, XSetWindowAttributes *attrs)
{
    KSimpleMenuWidget w = (KSimpleMenuWidget)gw;
#ifndef OLDXAW
    XawPixmap *pixmap;
#endif

    attrs->cursor = w->ksimple_menu.cursor;
    *mask |= CWCursor;
    if (w->ksimple_menu.backing_store == Always ||
	w->ksimple_menu.backing_store == NotUseful ||
	w->ksimple_menu.backing_store == WhenMapped) {
	*mask |= CWBackingStore;
	attrs->backing_store = w->ksimple_menu.backing_store;
    }
    else
	*mask &= ~CWBackingStore;

    (*Superclass->core_class.realize)(gw, mask, attrs);

    w->ksimple_menu.surface = XkwGetSurface(gw);

#ifndef OLDXAW
    if (w->core.background_pixmap > XtUnspecifiedPixmap) {
	pixmap = XawPixmapFromXPixmap(w->core.background_pixmap, XtScreen(w),
				      w->core.colormap, w->core.depth);
	if (pixmap && pixmap->mask)
	    XawReshapeWidget(w, pixmap);
    }
#endif
}

/*
 * Function:
 *	XkwKSimpleMenuResize
 *
 * Parameters:
 *	w - ksimple menu widget
 *
 * Description:
 *	Handle the menu being resized.
 */
static void
XkwKSimpleMenuResize(Widget gw)
{
    KSimpleMenuWidget w = (KSimpleMenuWidget) gw;

    if (!XtIsRealized(gw))
	return;

    cairo_surface_destroy(w->ksimple_menu.surface);
    w->ksimple_menu.surface = XkwGetSurface(gw);

    Layout(gw, NULL, NULL);

    XkwKSimpleMenuRedisplay(gw, NULL, NULL);
}

/*
 * Function:
 *	XkwKSimpleMenuSetValues
 *
 * Parameters:
 *	current - current state of the widget
 *	request - what was requested
 *	cnew    - what the widget will become
 *
 * Description:
 *	Relayout the menu when one of the resources is changed.
 */
/*ARGSUSED*/
static Boolean
XkwKSimpleMenuSetValues(Widget current, Widget request, Widget cnew,
		       ArgList args, Cardinal *num_args)
{
    KSimpleMenuWidget smw_old = (KSimpleMenuWidget)current;
    KSimpleMenuWidget smw_new = (KSimpleMenuWidget)cnew;
    Boolean ret_val = False, layout = False;

    if (!XtIsRealized(current))
	return (False);

    if (!smw_new->ksimple_menu.recursive_set_values) {
	if (XtWidth(smw_new) != XtWidth(smw_old)) {
	    smw_new->ksimple_menu.menu_width = XtWidth(smw_new) != 0;
	    layout = True;
	}
	if (XtHeight(smw_new) != XtHeight(smw_old)) {
	    smw_new->ksimple_menu.menu_height = XtHeight(smw_new) != 0;
	    layout = True;
	}
    }

    if (smw_old->ksimple_menu.cursor != smw_new->ksimple_menu.cursor)
	XDefineCursor(XtDisplay(cnew), XtWindow(cnew),
		      smw_new->ksimple_menu.cursor);

    if (smw_old->ksimple_menu.label_string !=smw_new->ksimple_menu.label_string) {
	if (smw_new->ksimple_menu.label_string == NULL)	    /* Destroy */
	    XtDestroyWidget((Widget)smw_old->ksimple_menu.label);
	else if (smw_old->ksimple_menu.label_string == NULL) /* Create */
	    CreateLabel(cnew);
	else {						    /* Change */
	    Arg arglist[1];

	    XtSetArg(arglist[0], XtNlabel, smw_new->ksimple_menu.label_string);
	    XtSetValues((Widget)smw_new->ksimple_menu.label, arglist, ONE);
	}
    }

    if (smw_old->ksimple_menu.label_class != smw_new->ksimple_menu.label_class)
	XtAppWarning(XtWidgetToApplicationContext(cnew),
		     "No Dynamic class change of the KSimpleMenu Label.");

    if (smw_old->ksimple_menu.top_margin != smw_new->ksimple_menu.top_margin
	|| smw_old->ksimple_menu.bottom_margin
	!= smw_new->ksimple_menu.bottom_margin) {
	layout = True;
	ret_val = True;
    }

#ifndef OLDXAW
    if (smw_old->core.background_pixmap != smw_new->core.background_pixmap) {
	XawPixmap *opix, *npix;

	opix = XawPixmapFromXPixmap(smw_old->core.background_pixmap,
				    XtScreen(smw_old), smw_old->core.colormap,
				    smw_old->core.depth);
	npix = XawPixmapFromXPixmap(smw_new->core.background_pixmap,
				    XtScreen(smw_new), smw_new->core.colormap,
				    smw_new->core.depth);
	if ((npix && npix->mask) || (opix && opix->mask))
	    XawReshapeWidget(cnew, npix);
    }
#endif

    if (layout)
	Layout(cnew, NULL, NULL);

    return (ret_val);
}

/*
 * Function:
 *	XkwKSimpleMenuSetValuesHook
 *
 * Parameters:
 *	w	 - menu widget
 *	arglist	 - argument list passed to XtSetValues
 *	num_args - number of args
 *
 * Description:
 *	To handle a special case, this is passed the actual arguments.
 */
static Boolean
XkwKSimpleMenuSetValuesHook(Widget w, ArgList arglist, Cardinal *num_args)
{
    Cardinal i;
    Dimension width, height;

    width = XtWidth(w);
    height = XtHeight(w);

    for (i = 0 ; i < *num_args ; i++) {
	if (streq(arglist[i].name, XtNwidth))
	    width = (Dimension)arglist[i].value;
	if (streq(arglist[i].name, XtNheight))
	    height = (Dimension) arglist[i].value;
    }

    if (width != XtWidth(w) || height != XtHeight(w))
	MakeSetValuesRequest(w, width, height);

    return (False);
}

/*
 * Geometry Management routines
 */
/*
 * Function:
 *	XkwKSimpleMenuGeometryManager
 *
 * Parameters:
 *	w	- Menu Entry making the request
 *	request - requested new geometry
 *                 reply - the allowed geometry.
 *
 * Description:
 *	This is the KSimpleMenu Widget's Geometry Manager.
 *
 * Returns:
 *	XtGeometry{Yes, No, Almost}
 */
static XtGeometryResult
XkwKSimpleMenuGeometryManager(Widget w, XtWidgetGeometry *request,
			     XtWidgetGeometry *reply)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)XtParent(w);
    KSmeObject entry = (KSmeObject)w;
    XtGeometryMask mode = request->request_mode;
    XtGeometryResult answer;
    Dimension old_height, old_width;

    if (!(mode & CWWidth) && !(mode & CWHeight))
	return (XtGeometryNo);

    reply->width = request->width;
    reply->height = request->height;

    old_width = XtWidth(entry);
    old_height = XtHeight(entry);

    Layout(w, &reply->width, &reply->height);

    /*
     * Since we are an override shell and have no parent there is no one to
     * ask to see if this geom change is okay, so I am just going to assume
     * we can do whatever we want.  If you subclass be very careful with this
     * assumption, it could bite you.
     *
     * Chris D. Peterson - Sept. 1989.
     */
    if ((!(mode & CWWidth) || reply->width == request->width)
	&& (!(mode & CWHeight) || reply->height == request->height)) {
	if (mode & XtCWQueryOnly) {	/* Actually perform the layout */
	    XtWidth(entry) = old_width;
	    XtHeight(entry) = old_height;
	}
	else
	    Layout((Widget)smw, NULL, NULL);
	answer = XtGeometryDone;
    }
    else {
	XtWidth(entry) = old_width;
	XtHeight(entry) = old_height;

	if ((reply->width == request->width && !(mode & CWHeight))
	    || (reply->height == request->height && !(mode & CWWidth))
	    || (reply->width == request->width
	    && reply->height == request->height))
	    answer = XtGeometryNo;
	else {
	    answer = XtGeometryAlmost;
	    reply->request_mode = 0;
	    if (reply->width != request->width)
		reply->request_mode |= CWWidth;
	    if (reply->height != request->height)
		reply->request_mode |= CWHeight;
	}
    }

    return (answer);
}

/*
 * Function:
 *	XkwKSimpleMenuChangeManaged
 *
 * Parameters:
 *	w - ksimple menu widget
 *
 * Description:
 *	Called whenever a new child is managed.
 */
static void
XkwKSimpleMenuChangeManaged(Widget w)
{
    Layout(w, NULL, NULL);
}

/*
 * Global Action Routines
 *
 * These actions routines will be added to the application's
 * global action list
 */
/*
 * Function:
 *	PositionMenuAction
 *
 * Parameters:
 *	w	   - a widget (no the ksimple menu widget)
 *	event	   - the event that caused this action
 *	params	   - parameters passed to the routine.
 *                                      we expect the name of the menu here.
 *	num_params - ""
 *
 * Description:
 *	Positions the ksimple menu widget.
 */
/*ARGSUSED*/
static void
PositionMenuAction(Widget w, XEvent *event,
		   String *params, Cardinal *num_params)
{
    Widget menu;
    XPoint loc;

    if (*num_params != 1) {
	XtAppWarning(XtWidgetToApplicationContext(w),
		     "KSimpleMenuWidget: position menu action expects "
		     "only one parameter which is the name of the menu.");
	return;
    }

    if ((menu = FindMenu(w, params[0])) == NULL) {
	char error_buf[BUFSIZ];

	snprintf(error_buf, sizeof(error_buf),
		 "KSimpleMenuWidget: could not find menu named %s.",
		 params[0]);
	XtAppWarning(XtWidgetToApplicationContext(w), error_buf);
	return;
    }

    switch (event->type) {
	case ButtonPress:
	case ButtonRelease:
	    loc.x = event->xbutton.x_root;
	    loc.y = event->xbutton.y_root;
	    PositionMenu(menu, &loc);
	    break;
	case EnterNotify:
	case LeaveNotify:
	    loc.x = event->xcrossing.x_root;
	    loc.y = event->xcrossing.y_root;
	    PositionMenu(menu, &loc);
	    break;
	case MotionNotify:
	    loc.x = event->xmotion.x_root;
	    loc.y = event->xmotion.y_root;
	    PositionMenu(menu, &loc);
	    break;
	default:
	    PositionMenu(menu, NULL);
	    break;
    }
}

/*
 * Widget Action Routines
 */
/*
 * Function:
 *	Unhighlight
 *
 * Parameters:
 *	w	   - ksimple menu widget
 *	event	   - event that caused this action
 *	params	   - not used
 *	num_params - ""
 *
 * Description:
 *	Unhighlights current entry.
 */
/*ARGSUSED*/
static void
Unhighlight(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    KSmeObject entry = smw->ksimple_menu.entry_set;

    if (entry == NULL)
	return;

#ifndef OLDXAW
    if (!smw->ksimple_menu.sub_menu)
#endif
    {
	KSmeObjectClass cclass;

	smw->ksimple_menu.entry_set = NULL;
	cclass = (KSmeObjectClass)entry->object.widget_class;
	(cclass->sme_class.unhighlight)((Widget)entry);
    }
}

/*
 * Function:
 *	Highlight
 *
 * Parameters:
 *	w	   - ksimple menu widget
 *	event	   - event that caused this action
 *	params	   - not used
 *	num_params - ""
 *
 * Description:
 *	Highlights current entry.
 */
/*ARGSUSED*/
static void
Highlight(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    KSmeObject entry;

    if (!XtIsSensitive(w))
	return;

    entry = GetEventEntry(w, event);

    if (entry == smw->ksimple_menu.entry_set)
	return;

#ifndef OLDXAW
    if (!smw->ksimple_menu.sub_menu)
#endif
	Unhighlight(w, event, params, num_params);

    if (entry == NULL)
	return;

    if (!XtIsSensitive((Widget)entry))
	return;

#ifndef OLDXAW
    if (smw->ksimple_menu.sub_menu)
	PopdownSubMenu(smw);
#endif

    Unhighlight(w, event, params, num_params);

#ifndef OLDXAW
    if (!(smw->ksimple_menu.state & SMW_UNMAPPING))
#endif
    {
	KSmeObjectClass cclass;

	smw->ksimple_menu.entry_set = entry;
	cclass = (KSmeObjectClass)entry->object.widget_class;

	(cclass->sme_class.highlight)((Widget)entry);

#ifndef OLDXAW
	if (XtIsSubclass((Widget)entry, smeBSBObjectClass))
	    PopupSubMenu(smw);
#endif
    }
}

/*
 * Function:
 *	Notify
 *
 * Parameters:
 *	w	   - ksimple menu widget
 *	event	   - event that caused this action
 *	params	   - not used
 *	num_params - ""
 *
 * Description:
 *	Notify user of current entry.
 */
/*ARGSUSED*/
static void
Notify(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KSmeObject entry;
    KSmeObjectClass cclass;

    /* may be a propagated event from a sub menu, need to check it */
    if (XtWindow(w) != event->xany.window)
	return;
    entry = GetEventEntry(w, event);
    if (entry == NULL || !XtIsSensitive((Widget)entry))
	return;

    cclass = (KSmeObjectClass) entry->object.widget_class;
    (cclass->sme_class.notify)((Widget)entry);
}

/*
 * Public Functions
 */
/*
 * Function:
 *	XkwKSimpleMenuAddGlobalActions
 *
 * Arguments:
 *	app_con - appcontext
 *
 * Description:
 *	Adds the global actions to the ksimple menu widget.
 */
void
XkwKSimpleMenuAddGlobalActions(XtAppContext app_con)
{
    XtInitializeWidgetClass(ksimpleMenuWidgetClass);
    XmuCallInitializers(app_con);
}

/*
 * Function:
 *	XkwKSimpleMenuGetActiveEntry
 *
 * Parameters:
 *	w - smw widget
 *
 * Description:
 *	Gets the currently active (set) entry.
 *
 * Returns:
 *	The currently set entry or NULL if none is set
 */
Widget
XkwKSimpleMenuGetActiveEntry(Widget w)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;

    return ((Widget)smw->ksimple_menu.entry_set);
}

/*
 * Function:
 *	XkwKSimpleMenuClearActiveEntry
 *
 * Parameters:
 *	w - smw widget
 *
 * Description:
 *	Unsets the currently active (set) entry.
 */
void
XkwKSimpleMenuClearActiveEntry(Widget w)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;

    smw->ksimple_menu.entry_set = NULL;
}

/*
 * Private Functions
 */
/*
 * Function:
 *	CreateLabel
 *
 * Parameters:
 *	w - smw widget
 *
 * Description:
 * Creates the label object and makes sure it is the first child in
 * in the list.
 */
static void
CreateLabel(Widget w)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    Widget *child, *next_child;
    int i;
    Arg args[2];

    if (smw->ksimple_menu.label_string == NULL ||
	smw->ksimple_menu.label != NULL) {
	XtAppWarning(XtWidgetToApplicationContext(w),
		     "Xaw KSimple Menu Widget: label string is NULL or "
		     "label already exists, no label is being created.");
	return;
    }

    XtSetArg(args[0], XtNlabel, smw->ksimple_menu.label_string);
    XtSetArg(args[1], XtNjustify, XtJustifyCenter);
    smw->ksimple_menu.label = (KSmeObject)
	XtCreateManagedWidget("menuLabel",
			      smw->ksimple_menu.label_class, w, args, TWO);

    next_child = NULL;
    for (child = smw->composite.children + smw->composite.num_children,
	 i = smw->composite.num_children; i > 0; i--, child--) {
	if (next_child != NULL)
	    *next_child = *child;
	next_child = child;
    }
    *child = (Widget)smw->ksimple_menu.label;
}

/*
 * Function:
 *	Layout
 *
 * Arguments:
 *	w	   - See below
 *	width_ret  - returned width
 *	height_ret - returned height
 *
 * Note:
 * if width == NULL || height == NULL then it assumes the you do not care
 * about the return values, and just want a relayout.
 *
 * if this is not the case then it will set width_ret and height_ret
 * to be width and height that the child would get if it were layed out
 * at this time.
 *
 *	"w" can be the ksimple menu widget or any of its object children.
 */
static void
Layout(Widget w, Dimension *width_ret, Dimension *height_ret)
{
    KSmeObject current_entry;
    KSimpleMenuWidget smw;
    Dimension width, height;
    Boolean allow_change_size;
    Widget kid;
    Cardinal i, count, n;
    int width_kid, height_kid, tmp_w, tmp_h;
    short vadd, hadd, x_ins, y_ins;
    Dimension *widths;

    height = 0;

    if (XtIsSubclass(w, ksimpleMenuWidgetClass)) {
	smw = (KSimpleMenuWidget)w;
	current_entry = NULL;
    }
    else {
	smw = (KSimpleMenuWidget)XtParent(w);
	current_entry = (KSmeObject)w;
    }

    allow_change_size = (!XtIsRealized((Widget)smw)
			 || smw->shell.allow_shell_resize);

    for (i = smw->ksimple_menu.label ? 1 : 0;
	 i < smw->composite.num_children;
	 i++) {
	XtWidgetGeometry preferred;

	kid = smw->composite.children[i];
	if (!XtIsManaged(kid))
	    continue;
	if (smw->ksimple_menu.row_height != 0)
	    XtHeight(kid) = smw->ksimple_menu.row_height;
	XtQueryGeometry(kid, NULL, &preferred);
	if (preferred.request_mode & CWWidth)
	    XtWidth(kid) = preferred.width;
    }

    if (smw->ksimple_menu.label
	&& XtIsManaged((Widget)smw->ksimple_menu.label)) {
	XtWidgetGeometry preferred;

	kid = (Widget)smw->ksimple_menu.label;
	XtQueryGeometry(kid, NULL, &preferred);
	if (preferred.request_mode & CWWidth)
	    XtWidth(kid) = preferred.width;
	if (preferred.request_mode & CWHeight)
	    XtHeight(kid) = preferred.height;
    }

    /* reset */
    if (!smw->ksimple_menu.menu_width)
	XtWidth(smw) = 0;
    if (!smw->ksimple_menu.menu_height)
	XtHeight(smw) = 0;
    if (!XtWidth(smw) || !XtHeight(smw))
	MakeResizeRequest((Widget)smw);

    widths = (Dimension *)XtMalloc(sizeof(Dimension));
#ifndef OLDXAW
    hadd = smw->ksimple_menu.left_margin;
#else
    hadd = 0;
#endif
    vadd = smw->ksimple_menu.top_margin;
    if (smw->ksimple_menu.label)
	vadd += XtHeight(smw->ksimple_menu.label);

    count = 1;
    width = tmp_w = tmp_h = n = 0;
    height = vadd;

    for (i = smw->ksimple_menu.label ? 1 : 0;
	 i < smw->composite.num_children;
	 i++) {
	kid = smw->composite.children[i];
	if (!XtIsManaged(kid))
	    continue;
	width_kid = XtWidth(kid);
	height_kid = XtHeight(kid);

	if (n && (height + height_kid + smw->ksimple_menu.bottom_margin
		  > XtHeight(smw))) {
	    ++count;
	    widths = (Dimension *)XtRealloc((char *)widths,
					    sizeof(Dimension) * count);
	    widths[count - 1] = width_kid;
	    width += tmp_w;
	    tmp_w = width_kid;
	    height = height_kid + vadd;
	}
	else
	    height += height_kid;
	if (height > tmp_h)
	    tmp_h = height;
	if (width_kid > tmp_w)
	    widths[count - 1] = tmp_w = width_kid;
	++n;
    }

    height = tmp_h + smw->ksimple_menu.bottom_margin;
    width += tmp_w;

    if (smw->ksimple_menu.label && width < XtWidth(smw->ksimple_menu.label)) {
	float inc;

	inc = (XtWidth(smw->ksimple_menu.label) - width) / (float)count;
	width = XtWidth(smw->ksimple_menu.label);
	for (n = 0; n < count; n++)
	    widths[n] += inc;
    }

#ifndef OLDXAW
    width += hadd + smw->ksimple_menu.right_margin;
#endif

    x_ins = n = count = 0;
    tmp_w = widths[0];
    tmp_h = vadd;

    for (i = smw->ksimple_menu.label ? 1 : 0;
	 i < smw->composite.num_children;
	 i++) {
	kid = smw->composite.children[i];
	if (!XtIsManaged(kid))
	    continue;

	height_kid = XtHeight(kid);

	if (n && (tmp_h + height_kid + smw->ksimple_menu.bottom_margin
		  > XtHeight(smw))) {
	    x_ins = tmp_w;
	    y_ins = vadd;
	    ++count;
	    tmp_w += widths[count];
	    tmp_h = height_kid + vadd;
	}
	else {
	    y_ins = tmp_h;
	    tmp_h += height_kid;
	}
	++n;

	XtX(kid) = x_ins + hadd;
	XtY(kid) = y_ins;
	XtWidth(kid) = widths[count];
    }

    XtFree((char *)widths);

    if (allow_change_size)
	MakeSetValuesRequest((Widget) smw, width, height);

    if (smw->ksimple_menu.label) {
	XtX(smw->ksimple_menu.label) = 0;
	XtY(smw->ksimple_menu.label) = smw->ksimple_menu.top_margin;
	XtWidth(smw->ksimple_menu.label) = XtWidth(smw)
#ifndef OLDXAW
	    - (smw->ksimple_menu.left_margin + smw->ksimple_menu.right_margin)
#endif
	    ;
    }
    if (current_entry) {
	if (width_ret)
	    *width_ret = XtWidth(current_entry);
	if (height_ret)
	    *height_ret = XtHeight(current_entry);
    }
}

/*
 * Function:
 *	AddPositionAction
 *
 * Parameters:
 *	app_con - application context
 *	data	- (not used)
 *
 * Description:
 *	  Adds the XawPositionKSimpleMenu action to the global
 *                   action list for this appcon.
 */
/*ARGSUSED*/
static void
AddPositionAction(XtAppContext app_con, XPointer data)
{
    static XtActionsRec pos_action[] = {
	{"XawPositionKSimpleMenu",	PositionMenuAction},
    };

    XtAppAddActions(app_con, pos_action, XtNumber(pos_action));
}

/*
 * Function:
 *	FindMenu
 *
 * Parameters:
 *	widget - reference widget
 *	name   - menu widget's name
 *
 * Description:
 *	Find the menu give a name and reference widget
 *
 * Returns:
 *	The menu widget or NULL.
 */
static Widget
FindMenu(Widget widget, String name)
{
    Widget w, menu;

    for (w = widget; w != NULL; w = XtParent(w))
	if ((menu = XtNameToWidget(w, name)) != NULL)
	    return (menu);

    return (NULL);
}

/*
 * Function:
 *	PositionMenu
 *
 * Parameters:
 *	w	 - ksimple menu widget
 *	location - pointer the the position or NULL
 *
 * Description:
 *	Places the menu
 */
static void
PositionMenu(Widget w, XPoint *location)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    KSmeObject entry;
    XPoint t_point;

    if (location == NULL) {
	Window temp1, temp2;
	int root_x, root_y, tempX, tempY;
	unsigned int tempM;

	location = &t_point;
	if (XQueryPointer(XtDisplay(w), XtWindow(w), &temp1, &temp2,
			  &root_x, &root_y, &tempX, &tempY, &tempM) == False) {
	    XtAppWarning(XtWidgetToApplicationContext(w),
			 "Xaw KSimple Menu Widget: "
			 "Could not find location of mouse pointer");
	    return;
	}
	location->x = (short) root_x;
	location->y = (short) root_y;
    }

    /*
     * The width will not be correct unless it is realized
     */
    XtRealizeWidget(w);

    location->x -= XtWidth(w) >> 1;

    if (smw->ksimple_menu.popup_entry == NULL)
	entry = smw->ksimple_menu.label;
    else
	entry = smw->ksimple_menu.popup_entry;

    if (entry != NULL)
      location->y -= XtY(entry) + (XtHeight(entry) >> 1);

    MoveMenu(w, location->x, location->y);
}

/*
 * Function:
 *	MoveMenu
 *
 * Parameters:
 *	w - ksimple menu widget
 *	x - current location of the widget
 *	y - ""
 *
 * Description:
 *	  Actually moves the menu, may force it to
 *	to be fully visable if menu_on_screen is True.
 */
static void
MoveMenu(Widget w, int x, int y)
{
    Arg arglist[2];
    Cardinal num_args = 0;
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;

    if (smw->ksimple_menu.menu_on_screen) {
	int width = XtWidth(w) + (XtBorderWidth(w) << 1);
	int height = XtHeight(w) + (XtBorderWidth(w) << 1);

	if (x >= 0) {
	    int scr_width = WidthOfScreen(XtScreen(w));

	    if (x + width > scr_width)
		x = scr_width - width;
	}
	if (x < 0)
	    x = 0;

	if (y >= 0) {
	    int scr_height = HeightOfScreen(XtScreen(w));

	    if (y + height > scr_height)
		y = scr_height - height;
	}
	if (y < 0)
	    y = 0;
    }

    XtSetArg(arglist[num_args], XtNx, x); num_args++;
    XtSetArg(arglist[num_args], XtNy, y); num_args++;
    XtSetValues(w, arglist, num_args);
}

/*
 * Function:
 *	ChangeCursorOnGrab
 *
 * Parameters:
 *	w     - menu widget
 *	temp1 - not used
 *	temp2 - ""
 *
 * Description:
 *	  Changes the cursor on the active grab to the one
 *                   specified in out resource list.
 */
/*ARGSUSED*/
static void
ChangeCursorOnGrab(Widget w, XtPointer temp1, XtPointer temp2)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;

    /*
     * The event mask here is what is currently in the MIT implementation.
     * There really needs to be a way to get the value of the mask out
     * of the toolkit (CDP 5/26/89).
     */
    XChangeActivePointerGrab(XtDisplay(w), ButtonPressMask | ButtonReleaseMask,
			     smw->ksimple_menu.cursor,
			     XtLastTimestampProcessed(XtDisplay(w)));
}

/*
 * Function:
 *	MakeSetValuesRequest
 *
 * Parameters:
 *	w      - ksimple menu widget
 *	width  - size requested
 *	height - ""
 */
static void
MakeSetValuesRequest(Widget w, unsigned int width, unsigned int height)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    Arg arglist[2];
    Cardinal num_args = 0;

    if (!smw->ksimple_menu.recursive_set_values) {
	if (XtWidth(smw) != width || XtHeight(smw) != height) {
	    smw->ksimple_menu.recursive_set_values = True;
	    XtSetArg(arglist[num_args], XtNwidth, width);   num_args++;
	    XtSetArg(arglist[num_args], XtNheight, height); num_args++;
	    XtSetValues(w, arglist, num_args);
	}
	else if (XtIsRealized((Widget)smw))
	    XkwKSimpleMenuRedisplay((Widget)smw, NULL, NULL);
    }
    smw->ksimple_menu.recursive_set_values = False;
}

static KSmeObject
DoGetEventEntry(Widget w, int x_loc, int y_loc)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    KSmeObject *entry;

    ForAllChildren(smw, entry) {
	if (!XtIsManaged((Widget)*entry))
	    continue;

	if (x_loc > XtX(*entry)
	    && x_loc <= XtX(*entry) + XtWidth(*entry)
	    && y_loc > XtY(*entry)
	    &&  y_loc <= XtY(*entry) + XtHeight(*entry)) {
	    if (*entry == smw->ksimple_menu.label)
		return (NULL);	/* cannot select the label */
	    else
		return (*entry);
	}
    }

    return (NULL);
}

/*
 * Function:
 *	GetEventEntry
 *
 * Parameters:
 *	w     - ksimple menu widget
 *	event - X event
 *
 * Description:
 *	Gets an entry given an event that has X and Y coords.
 *
 * Returns:
 *	The entry that this point is in
 */
static KSmeObject
GetEventEntry(Widget w, XEvent *event)
{
    int x_loc, y_loc, x_root;
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;
    KSmeObject entry;
    int warp, move;

    switch (event->type) {
	case MotionNotify:
	    x_loc = event->xmotion.x;
	    y_loc = event->xmotion.y;
	    x_root = event->xmotion.x_root;
	    break;
	case EnterNotify:
	case LeaveNotify:
	    x_loc = event->xcrossing.x;
	    y_loc = event->xcrossing.y;
	    x_root = event->xcrossing.x_root;
	    break;
	case ButtonPress:
	case ButtonRelease:
	    x_loc = event->xbutton.x;
	    y_loc = event->xbutton.y;
	    x_root = event->xbutton.x_root;
	    break;
	default:
	    XtAppError(XtWidgetToApplicationContext(w),
		       "Unknown event type in GetEventEntry().");
	    return (NULL);
    }

    if (x_loc < 0 || x_loc >= XtWidth(smw) ||
	y_loc < 0 || y_loc >= XtHeight(smw))
	return (NULL);

    /* Move the menu if it's outside the screen, does not check
     * smw->ksimple_menu.menu_on_screen because menus is bigger than screen
     */
    if (x_root == WidthOfScreen(XtScreen(w)) - 1 &&
	XtX(w) + XtWidth(w) + (XtBorderWidth(w)) > x_root) {
	warp = -8;
	if (smw->ksimple_menu.entry_set) {
	    entry = DoGetEventEntry(w,
				    XtX(smw->ksimple_menu.entry_set)
				    + XtWidth(smw->ksimple_menu.entry_set) + 1,
				    y_loc);
	    Unhighlight(w, event, NULL, NULL);
	    if (entry) {
		warp = -(int)XtWidth(entry) >> 1;
		move = x_loc - XtWidth(entry) - XtX(entry) + XtBorderWidth(w);
	    }
	    else {
		warp = 0;
		move = WidthOfScreen(XtScreen(w)) -
		       (XtX(w) + XtWidth(w) + (XtBorderWidth(w) << 1));
	    }
	}
	else {
	    warp = 0;
	    move = WidthOfScreen(XtScreen(w)) -
		   (XtX(w) + XtWidth(w) + (XtBorderWidth(w) << 1));
	}
    }
    else if (x_root == 0 && XtX(w) < 0) {
	warp = 8;
	if (smw->ksimple_menu.entry_set) {
	    entry = DoGetEventEntry(w, XtX(smw->ksimple_menu.entry_set) - 1,
				    y_loc);
	    Unhighlight(w, event, NULL, NULL);
	    if (entry) {
		warp = XtWidth(entry) >> 1;
		move = x_loc - XtX(entry);
	    }
	    else
		move = x_loc + XtBorderWidth(w);
	}
	else
	    move = x_loc + XtBorderWidth(w);
    }
    else
	move = warp = 0;

    if (move)
	XtMoveWidget(w, XtX(w) + move, XtY(w));
    if (warp)
	XWarpPointer(XtDisplay(w), None, None, 0, 0, 0, 0, warp, 0);

    return (DoGetEventEntry(w, x_loc, y_loc));
}

#define maxi(a,b) ({ typeof(a) _ta = (a), _tb = (b); if (_ta > _tb) _tb = _ta; _tb; })

static void
CalculateNewSize(Widget w, Dimension *width_return, Dimension *height_return)
{
    KSimpleMenuWidget xaw = (KSimpleMenuWidget)w;
    Widget kid;
    Cardinal i;
    int width_kid, height_kid;
    int width, height, tmp_w, tmp_h, max_dim;
    short vadd, hadd;
    int n, columns, test_h, num_children = 0;
    Boolean try_layout = False;

#ifndef OLDXAW
    hadd = xaw->ksimple_menu.left_margin + xaw->ksimple_menu.right_margin;
#else
    hadd = 0;
#endif
    vadd = xaw->ksimple_menu.top_margin + xaw->ksimple_menu.bottom_margin;
    if (xaw->ksimple_menu.label)
	vadd += XtHeight(xaw->ksimple_menu.label);

    if (*height_return)
	max_dim = *height_return;
    else if (!XtHeight(w)) {
	max_dim = HeightOfScreen(XtScreen(w));
	try_layout = True;
    }
    else
	max_dim = XtHeight(w);
    max_dim -= vadd;

    width = height = tmp_w = tmp_h = n = test_h = 0;
    columns = 1;
    for (i = xaw->ksimple_menu.label ? 1 : 0;
	 i < xaw->composite.num_children;
	 i++) {
	kid = xaw->composite.children[i];
	if (!XtIsManaged(kid))
	    continue;
	++num_children;
	width_kid = XtWidth(kid);
	height_kid = XtHeight(kid);

	if (try_layout) {
	    if (!test_h)
		test_h = height_kid;
	    else if (test_h != height_kid)
		try_layout = False;
	}

	if (n && (height + height_kid > max_dim)) {
	    ++columns;
	    width += tmp_w;
	    tmp_w = width_kid;
	    height = height_kid;
	}
	else
	    height += height_kid;
	if (height > tmp_h)
	    tmp_h = height;
	if (width_kid > tmp_w)
	    tmp_w = width_kid;
	++n;
    }

    height = tmp_h + vadd;
    width += tmp_w + hadd;

    if (xaw->ksimple_menu.label)
	width = maxi(width, XtWidth(xaw->ksimple_menu.label) + hadd);

    *width_return = width;
    *height_return = height;

    if (try_layout && columns > 1 && num_children > 2) {
	int space;

	height = test_h * (xaw->ksimple_menu.label ?
			   num_children - 1 :
			   num_children);

	max_dim -= max_dim % test_h;
	space = max_dim - (height % max_dim);
	if (space >= test_h * columns) {
	    height = max_dim - space / columns;
	    if (height % test_h)
		height += test_h - (height % test_h);
	    *height_return = height + vadd;
	    CalculateNewSize(w, width_return, height_return);
	}
    }
}

static void
MakeResizeRequest(Widget w)
{
    int tries;
    Dimension width, height;

    width = XtWidth(w);
    height = XtHeight(w);

    for (tries = 0; tries < 100; tries++) {
	CalculateNewSize(w, &width, &height);
	if (width == XtWidth(w) && height == XtHeight(w))
	    break;
	if (XtMakeResizeRequest(w, width, height, &width, &height) ==
	    XtGeometryNo)
	break;
    }
}

#ifndef OLDXAW
static void
Popdown(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;

    while (XtParent(w) &&
	   XtIsSubclass(XtParent(w), ksimpleMenuWidgetClass)) {
	if (((KSimpleMenuWidget)XtParent(w))->ksimple_menu.sub_menu == (Widget)w) {
	    w = XtParent(w);
	    smw = (KSimpleMenuWidget)w;
	    smw->ksimple_menu.entry_set = NULL;
	}
	else
	    break;
    }

    smw->ksimple_menu.state |= SMW_UNMAPPING;
    if (smw->ksimple_menu.sub_menu)
	PopdownSubMenu(smw);
    XtCallActionProc(w, "XtMenuPopdown", event, params, *num_params);
}

static void
PopupSubMenu(KSimpleMenuWidget smw)
{
    Arg args[2];
    Cardinal num_args;
    Widget menu;
    KSmeBSBObject entry = (KSmeBSBObject)smw->ksimple_menu.entry_set;
    Position menu_x, menu_y;
    Bool popleft;

    if (entry->sme_bsb.menu_name == NULL)
	return;

    if ((menu = FindMenu((Widget)smw, entry->sme_bsb.menu_name)) == NULL)
	return;

    smw->ksimple_menu.sub_menu = menu;

    if (!XtIsRealized(menu))
	XtRealizeWidget(menu);

    popleft = (smw->ksimple_menu.state & SMW_POPLEFT) != 0;

    if (popleft)
	XtTranslateCoords((Widget)smw, -(int)XtWidth(menu),
			  XtY(entry) - XtBorderWidth(menu), &menu_x, &menu_y);
    else
	XtTranslateCoords((Widget)smw, XtWidth(smw), XtY(entry)
			  - XtBorderWidth(menu), &menu_x, &menu_y);

    if (!popleft && menu_x >= 0) {
	int scr_width = WidthOfScreen(XtScreen(menu));

	if (menu_x + XtWidth(menu) > scr_width) {
	    menu_x -= XtWidth(menu) + XtWidth(smw);
	    popleft = True;
	}
    }
    else if (popleft && menu_x < 0) {
	menu_x = 0;
	popleft = False;
    }
    if (menu_y >= 0) {
	int scr_height = HeightOfScreen(XtScreen(menu));

	if (menu_y + XtHeight(menu) > scr_height)
	    menu_y = scr_height - XtHeight(menu) - XtBorderWidth(menu);
    }
    if (menu_y < 0)
	menu_y = 0;

    num_args = 0;
    XtSetArg(args[num_args], XtNx, menu_x);	num_args++;
    XtSetArg(args[num_args], XtNy, menu_y);	num_args++;
    XtSetValues(menu, args, num_args);

    if (popleft)
	((KSimpleMenuWidget)menu)->ksimple_menu.state |= SMW_POPLEFT;
    else
	((KSimpleMenuWidget)menu)->ksimple_menu.state &= ~SMW_POPLEFT;

    XtPopup(menu, XtGrabNone);
}

static void
PopdownSubMenu(KSimpleMenuWidget smw)
{
    KSimpleMenuWidget menu = (KSimpleMenuWidget)smw->ksimple_menu.sub_menu;

    if (!menu)
	return;

    menu->ksimple_menu.state |= SMW_UNMAPPING;
    PopdownSubMenu(menu);

    XtPopdown((Widget)menu);

    smw->ksimple_menu.sub_menu = NULL;
}

/*ARGSUSED*/
static void
PopupCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    KSimpleMenuWidget smw = (KSimpleMenuWidget)w;

    smw->ksimple_menu.state &= ~(SMW_UNMAPPING | SMW_POPLEFT);
}
#endif /* OLDXAW */
