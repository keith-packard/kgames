/*
 * Copyright © 2020 Keith Packard
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

#include <Xkw/KToggleP.h>

#define superclass (&kcommandClassRec)

static KToggleWidget
RadioGroupLeader(KToggleWidget w)
{
    KToggleWidget leader;

    for (leader = w; leader && leader->ktoggle.radio_prev; leader = leader->ktoggle.radio_prev)
	;
    return leader;
}

static void
AppendToRadioGroup(KToggleWidget w, KToggleWidget leader)
{
    KToggleWidget last;

    /* Find last member of radio group */
    for (last = leader; last->ktoggle.radio_next != NULL; last = (KToggleWidget) last->ktoggle.radio_next)
	;

    /* Insert into list */
    w->ktoggle.radio_prev = last;
    w->ktoggle.radio_next = NULL;

    last->ktoggle.radio_next = w;
}

static void
RemoveFromRadioGroup(KToggleWidget w)
{
    if (w->ktoggle.radio_prev)
	w->ktoggle.radio_prev->ktoggle.radio_next = w->ktoggle.radio_next;
    if (w->ktoggle.radio_next)
	w->ktoggle.radio_next->ktoggle.radio_prev = w->ktoggle.radio_prev;
}

static void
Initialize(Widget request, Widget cnew,
	   ArgList args, Cardinal *num_args)
{
    KToggleWidget w = (KToggleWidget) cnew;

    w->ktoggle.radio_prev = NULL;
    w->ktoggle.radio_next = NULL;

    if (w->ktoggle.radio_leader != NULL)
	AppendToRadioGroup(w, (KToggleWidget) w->ktoggle.radio_leader);
}

static void
Destroy(Widget gw)
{
    KToggleWidget w = (KToggleWidget) gw;

    RemoveFromRadioGroup(w);
}

static Boolean
SetValues(Widget current, Widget request, Widget cnew,
	  ArgList args, Cardinal *num_args)
{
    return False;
}

static void
TurnOffRadioSiblings(Widget gw)
{
    KToggleWidget w = (KToggleWidget) gw;

    KToggleWidget	radio;

    /* Find group leader */
    radio = RadioGroupLeader(w);

    for (; radio; radio = (KToggleWidget) (radio->ktoggle.radio_next))
	if (radio->kcommand.set && radio != w) {
	    KToggleWidgetClass cclass = (KToggleWidgetClass)radio->core.widget_class;

	    cclass->kcommand_class.unset((Widget) radio, NULL, NULL, NULL);
	    cclass->kcommand_class.notify((Widget) radio, NULL, NULL, NULL);
	}
}

static void
ToggleSet(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KToggleWidget w = (KToggleWidget) gw;
    KToggleWidgetClass cclass = (KToggleWidgetClass)w->core.widget_class;

    TurnOffRadioSiblings(gw);
    cclass->kcommand_class.set(gw, event, NULL, NULL);
}

static void
Toggle(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KToggleWidget w = (KToggleWidget) gw;
    KToggleWidgetClass cclass = (KToggleWidgetClass)gw->core.widget_class;

    if (w->kcommand.set)
	cclass->kcommand_class.unset(gw, event, NULL, NULL);
    else
	ToggleSet(gw, event, params, num_params);
}

static void
Notify(Widget gw, XEvent *event, String *params, Cardinal *num_params)
{
    KToggleWidgetClass cclass = (KToggleWidgetClass)gw->core.widget_class;

    cclass->kcommand_class.notify(gw, event, params, num_params);
}

XtPointer
XkwKToggleGetCurrent (Widget radio_group)
{
    KToggleWidget radio;

    for (radio = RadioGroupLeader((KToggleWidget) radio_group);
	 radio;
	 radio = radio->ktoggle.radio_next)
    {
	if (radio->kcommand.set)
	    return radio->ktoggle.radio_data;
    }
    return NULL;
}

void
XkwKToggleSetCurrent(Widget radio_group, XtPointer radio_data)
{
    KToggleWidget radio;

    for (radio = RadioGroupLeader((KToggleWidget) radio_group);
	 radio;
	 radio = radio->ktoggle.radio_next)
    {
	if (radio->ktoggle.radio_data == radio_data) {
	    ToggleSet((Widget) radio, NULL, NULL, NULL);
	    break;
	}
    }
}

static XtResource resources[] = {
#define offset(field) XtOffsetOf(KToggleRec, ktoggle.field)
  { XtNradioGroup, XtCWidget, XtRWidget, sizeof(Widget),
    offset(radio_leader), XtRWidget, NULL },
  { XtNradioData, XtCRadioData, XtRPointer, sizeof(XtPointer),
    offset(radio_data), XtRPointer, NULL },
};

static char defaultTranslations[] =
"<Enter>:"		"highlight(Always)\n"
"<Leave>:"		"unhighlight()\n"
"<Btn1Down>,<Btn1Up>:"	"toggle() notify()\n"
;

static XtActionsRec actionsList[] = {
    {"toggle",		Toggle},
    {"notify",		Notify},
    {"set",		ToggleSet},
};

KToggleClassRec ktoggleClassRec = {
  /* core */
  {
    (WidgetClass)superclass,		/* superclass */
    "Toggle",				/* class_name */
    sizeof(KToggleRec),			/* widget_size */
    NULL,				/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    Initialize,				/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    actionsList,			/* actions */
    XtNumber(actionsList),		/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    Destroy,				/* destroy */
    XtInheritResize,			/* resize */
    XtInheritExpose,			/* expose */
    SetValues,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    defaultTranslations,		/* tm_table */
    XtInheritQueryGeometry,		/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
  },
  /* ksimple */
  {
      /* empty */	0
  },
  /* klabel */
  {
      /* empty */	0
  },
  /* kcommand */
  {
      XtInheritSet,			/* set */
      XtInheritUnset,			/* unset */
      XtInheritNotify,			/* notify */
  },
  /* ktoggle */
  {
      /* empty */	0
  }
};

WidgetClass ktoggleWidgetClass = (WidgetClass)&ktoggleClassRec;
