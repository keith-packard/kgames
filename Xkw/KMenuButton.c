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
 *
 */

/*
 * MenuButton.c - Source code for MenuButton widget.
 *
 * This is the source code for the Athena MenuButton widget.
 * It is intended to provide an easy method of activating pulldown menus.
 *
 * Date:    May 2, 1989
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
#include <Xkw/KMenuButtonP.h>
#include <Xkw/Xkw.h>

/*
 * Class Methods
 */
static void XkwKMenuButtonClassInitialize(void);
static void XkwKMenuButtonInitialize(Widget, Widget, ArgList, Cardinal*);
static void XkwKMenuButtonDestroy(Widget);
static Boolean XkwKMenuButtonSetValues(Widget, Widget, Widget, ArgList, Cardinal*);

/*
 * Actions
 */
static void PopupMenu(Widget, XEvent*, String*, Cardinal*);

/*
 * Initialization
 */
#define superclass ((KCommandWidgetClass)&kcommandClassRec)

static char defaultTranslations[] =
"<Enter>:"	"highlight()\n"
"<Leave>:"	"reset()\n"
"Any<BtnDown>:"	"reset() PopupMenu()\n";

static char default_menu_name[] = "menu";

#define offset(field) XtOffsetOf(KMenuButtonRec, field)
static XtResource resources[] = {
  {
    XtNmenuName,
    XtCMenuName,
    XtRString,
    sizeof(String),
    offset(menu_button.menu_name),
    XtRString,
    (XtPointer)default_menu_name
  },
};
#undef offset

static XtActionsRec actionsList[] =
{
  {"PopupMenu",	PopupMenu},
};

KMenuButtonClassRec kmenuButtonClassRec = {
  /* core */
  {
    (WidgetClass)superclass,		/* superclass		  */
    "MenuButton",			/* class_name		  */
    sizeof(KMenuButtonRec),		/* size			  */
    XkwKMenuButtonClassInitialize,	/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    False,				/* class_inited		  */
    XkwKMenuButtonInitialize,		/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* num_resources	  */
    NULLQUARK,				/* xrm_class		  */
    False,				/* compress_motion	  */
    True,				/* compress_exposure	  */
    True,				/* compress_enterleave	  */
    False,				/* visible_interest	  */
    XkwKMenuButtonDestroy,		/* destroy		  */
    XtInheritResize,			/* resize		  */
    XtInheritExpose,			/* expose		  */
    XkwKMenuButtonSetValues,		/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
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
    XtInheritChangeSensitive,		/* change_sensitive	  */
#ifndef OLDXAW
    NULL,                               /* extension */
#endif
  },
  /* ksimple */
  {
      0,
  },
  /* klabel */
  {
    0,				/* extension */
  },
  /* kcommand */
  {
    0,				/* extension */
  },
  /* kmenu_button */
  {
    NULL,				/* extension */
  },
};

WidgetClass kmenuButtonWidgetClass = (WidgetClass)&kmenuButtonClassRec;

/*
 * Implementation
 */
static void
XkwKMenuButtonClassInitialize(void)
{
    XkwInitializeWidgetSet();
    XtRegisterGrabAction(PopupMenu, True,
			 ButtonPressMask | ButtonReleaseMask,
			 GrabModeAsync, GrabModeAsync);
}

/*ARGSUSED*/
static void
XkwKMenuButtonInitialize(Widget request, Widget cnew,
			ArgList args, Cardinal *num_args)
{
    KMenuButtonWidget mbw = (KMenuButtonWidget)cnew;

    (void) request;
    (void) args;
    (void) num_args;

    if (mbw->menu_button.menu_name != default_menu_name)
	mbw->menu_button.menu_name = XtNewString(mbw->menu_button.menu_name);
}

static void
XkwKMenuButtonDestroy(Widget w)
{
    KMenuButtonWidget mbw = (KMenuButtonWidget)w;

    if (mbw->menu_button.menu_name != default_menu_name)
	XtFree(mbw->menu_button.menu_name);
}

/*ARGSUSED*/
static Boolean
XkwKMenuButtonSetValues(Widget current, Widget request, Widget cnew,
		       ArgList args, Cardinal *num_args)
{
    KMenuButtonWidget mbw_old = (KMenuButtonWidget)current;
    KMenuButtonWidget mbw_new = (KMenuButtonWidget)cnew;

    (void) request;
    (void) args;
    (void) num_args;

    if (mbw_old->menu_button.menu_name != mbw_new->menu_button.menu_name) {
	if (mbw_old->menu_button.menu_name != default_menu_name)
	    XtFree(mbw_old->menu_button.menu_name);
	if (mbw_new->menu_button.menu_name != default_menu_name)
	    mbw_new->menu_button.menu_name =
		XtNewString(mbw_new->menu_button.menu_name);
    }

    return TRUE;
}

/*ARGSUSED*/
static void
PopupMenu(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    KMenuButtonWidget mbw = (KMenuButtonWidget)w;
    Widget menu = NULL, temp;
    Arg arglist[2];
    Cardinal num_args;
    int menu_x, menu_y, menu_width, menu_height, button_height;
    Position button_x, button_y;

    (void) event;
    (void) params;
    (void) num_params;
    temp = w;
    while(temp != NULL) {
	menu = XtNameToWidget(temp, mbw->menu_button.menu_name);
	if (menu == NULL)
	    temp = XtParent(temp);
	else
	    break;
    }

    if (menu == NULL) {
	char error_buf[BUFSIZ];

	snprintf(error_buf, sizeof(error_buf),
		 "KMenuButton:  Could not find menu widget named %s.",
		 mbw->menu_button.menu_name);
	XtAppWarning(XtWidgetToApplicationContext(w), error_buf);
	return;
    }

    if (!XtIsRealized(menu))
	XtRealizeWidget(menu);

    menu_width = XtWidth(menu) + (XtBorderWidth(menu) << 1);
    button_height = XtHeight(w) + (XtBorderWidth(w) << 1);
    menu_height = XtHeight(menu) + (XtBorderWidth(menu) << 1);

    XtTranslateCoords(w, 0, 0, &button_x, &button_y);
    menu_x = button_x;
    menu_y = button_y + button_height;

    if (menu_y >= 0) {
	int scr_height = HeightOfScreen(XtScreen(menu));

	if (menu_y + menu_height > scr_height)
	    menu_y = button_y - menu_height;
	if (menu_y < 0) {
	    menu_y = scr_height - menu_height;
	    menu_x = button_x + XtWidth(w) + (XtBorderWidth(w) << 1);
	    if (menu_x + menu_width > WidthOfScreen(XtScreen(menu)))
		menu_x = button_x - menu_width;
	}
    }
    if (menu_y < 0)
	menu_y = 0;

    if (menu_x >= 0) {
	int scr_width = WidthOfScreen(XtScreen(menu));

	if (menu_x + menu_width > scr_width)
	    menu_x = scr_width - menu_width;
    }
    if (menu_x < 0)
	menu_x = 0;

    num_args = 0;
    XtSetArg(arglist[num_args], XtNx, menu_x); num_args++;
    XtSetArg(arglist[num_args], XtNy, menu_y); num_args++;
    XtSetValues(menu, arglist, num_args);

    XtPopupSpringLoaded(menu);
}
