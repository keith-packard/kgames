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

#include <Xkw/KTextLineP.h>

#define superclass (&ksimpleClassRec)

static void
init_cairo(KTextLineWidget w, cairo_t *cr)
{
    cairo_set_font_face(cr, w->ktext_line.font.font_face);
    cairo_set_font_size(cr, w->ktext_line.font.size * w->ksimple.dpi / 72.0);
}

static cairo_t *
get_cairo(KTextLineWidget w)
{
    cairo_t *cr = XkwGetCairo((Widget) w);
    init_cairo(w, cr);
    return cr;
}

static cairo_t *
draw_begin(KTextLineWidget w, Region region)
{
    cairo_t *cr = XkwDrawBegin((Widget) w, region);
    init_cairo(w, cr);
    return cr;
}

static void
draw_end(KTextLineWidget w, Region region, cairo_t *cr)
{
    XkwDrawEnd((Widget) w, region, cr);
}

static double
pad(cairo_font_extents_t *font_extents)
{
    return font_extents->height / 4.0;
}

static double
cursor_width(cairo_font_extents_t *font_extents)
{
    return font_extents->height / 8.0;
}

static void
preferred_size(KTextLineWidget w, Dimension *width, Dimension *height)
{
    cairo_t *cr = get_cairo(w);
    char *string;
    cairo_text_extents_t text_extents;
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    if (w->ktext_line.string && w->ktext_line.string[0])
	string = w->ktext_line.string;
    else
	string = "hello world";
    cairo_text_extents(cr, string, &text_extents);
    cairo_destroy(cr);

    *width = text_extents.width + pad(&font_extents) * 2;
    *height = font_extents.height + pad(&font_extents) * 2;
}

static void
Initialize(Widget request, Widget cnew,
	   ArgList args, Cardinal *num_args)
{
    KTextLineWidget w = (KTextLineWidget) cnew;

    if (w->ktext_line.string == NULL)
	w->ktext_line.string = "";
    w->ktext_line.string = XtNewString(w->ktext_line.string);
    w->ktext_line.size = strlen (w->ktext_line.string);
    w->ktext_line.cursor = w->ktext_line.size;

    if (XtWidth(w) == 0 || XtHeight(w) == 0) {
	Dimension width, height;
	preferred_size(w, &width, &height);
	if (XtWidth(w) == 0)
	    XtWidth(w) = width;
	if (XtHeight(w) == 0)
	    XtHeight(w) = height;
    }
}

static void
Destroy(Widget gw)
{
    KTextLineWidget w = (KTextLineWidget)gw;

    XtFree(w->ktext_line.string);
}

static double
text_width(cairo_t *cr, char *text, int len)
{
    char save = text[len];
    if (save != '\0')
	text[len] = '\0';
    cairo_text_extents_t text_extents;
    cairo_text_extents(cr, text, &text_extents);
    if (save != '\0')
	text[len] = save;
   return text_extents.x_advance;
}

static void
Redisplay(Widget gw, XEvent *event, Region region)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    if (*superclass->core_class.expose != NULL)
	(*superclass->core_class.expose)(gw, event, region);

    cairo_t *cr = draw_begin(w, region);

    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);

    double padding = pad(&font_extents);
    double top = (w->core.height - (font_extents.ascent + font_extents.descent)) / 2;
    double x = padding;
    double y = top + font_extents.ascent;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, w->ktext_line.string);

    x = padding + text_width(cr, w->ktext_line.string, w->ktext_line.cursor);
    cairo_set_line_width(cr, cursor_width(&font_extents));
    cairo_move_to(cr, x, top);
    cairo_line_to(cr, x, top + font_extents.height);
    cairo_stroke(cr);
    draw_end(w, region, cr);
}

static Boolean
SetValues(Widget gcur, Widget greq, Widget gnew,
	  ArgList args, Cardinal *num_args)
{
    KTextLineWidget cur = (KTextLineWidget)gcur;
    KTextLineWidget req = (KTextLineWidget)greq;
    KTextLineWidget new = (KTextLineWidget)gnew;
    Boolean redisplay = False;

    (void) req;
    if (new->ktext_line.string != cur->ktext_line.string) {
	XtFree(cur->ktext_line.string);
	if (!new->ktext_line.string)
	    new->ktext_line.string = XtNewString("");
	else
	    new->ktext_line.string = XtNewString(new->ktext_line.string);
	redisplay = True;
    }
    if (new->ktext_line.font.font_face != cur->ktext_line.font.font_face ||
	new->ktext_line.font.size != cur->ktext_line.font.size)
	redisplay = True;

    return redisplay;
}

static XtGeometryResult
QueryGeometry(Widget gw, XtWidgetGeometry *intended,
	      XtWidgetGeometry *preferred)
{
    KTextLineWidget w = (KTextLineWidget)gw;

    preferred->request_mode = CWWidth | CWHeight;
    preferred_size(w, &preferred->width, &preferred->height);

    if (((intended->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	&& intended->width == preferred->width
	&& intended->height == preferred->height)
	return (XtGeometryYes);
      else if (preferred->width == XtWidth(w) && preferred->height == XtHeight(w))
	return (XtGeometryNo);

    return (XtGeometryAlmost);
}

static void
MoveForwardChar(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    if (w->ktext_line.cursor < strlen(w->ktext_line.string)) {
	w->ktext_line.cursor++;
	Redisplay(gw, NULL, NULL);
    }
}

static void
MoveBackwardChar(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    if (w->ktext_line.cursor > 0) {
	w->ktext_line.cursor--;
	Redisplay(gw, NULL, NULL);
    }
}

static void
DeleteBackwardChar(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    if (w->ktext_line.cursor > 0) {
	memmove(w->ktext_line.string + w->ktext_line.cursor - 1,
		w->ktext_line.string + w->ktext_line.cursor,
		strlen(w->ktext_line.string) - w->ktext_line.cursor + 1);
	w->ktext_line.cursor--;
	Redisplay(gw, NULL, NULL);
    }
}

static void
Delete(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    int len = strlen(w->ktext_line.string);
    if (len > w->ktext_line.cursor) {
	memmove(w->ktext_line.string + w->ktext_line.cursor,
		w->ktext_line.string + w->ktext_line.cursor + 1,
		strlen(w->ktext_line.string) - w->ktext_line.cursor);
	Redisplay(gw, NULL, NULL);
    }
}

static void
Accept(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    XtCallCallbackList(gw, w->ktext_line.callbacks, (XtPointer) NULL);
}

static void
InsertChar(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;
    char strbuf[128];
    KeySym keysym;

    int new = XLookupString((XKeyEvent*)ev, strbuf, sizeof(strbuf), &keysym, NULL);

    if (new == 0)
	return;

    int len = strlen(w->ktext_line.string);
    if (len + new + 1 > w->ktext_line.size)
	w->ktext_line.string = realloc(w->ktext_line.string, w->ktext_line.size += (new + 16));

    /* shift text right, including NUL */
    memmove(w->ktext_line.string + w->ktext_line.cursor + new,
	    w->ktext_line.string + w->ktext_line.cursor,
	    len + 1 - w->ktext_line.cursor);
    memcpy(w->ktext_line.string + w->ktext_line.cursor, strbuf, new);
    w->ktext_line.cursor += new;
    Redisplay(gw, NULL, NULL);
}

static void
SetCursor(Widget gw, XEvent *ev, String *args, Cardinal *num)
{
    KTextLineWidget w = (KTextLineWidget) gw;

    cairo_t *cr = get_cairo(w);
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    double padding = pad(&font_extents);
    int i;
    int len = strlen(w->ktext_line.string);

    for (i = 0; i < len; i++) {
	double x = padding + text_width(cr, w->ktext_line.string, i);
	if (x >= ev->xbutton.x)
	    break;
    }
    w->ktext_line.cursor = i;
    cairo_destroy(cr);
    Redisplay(gw, NULL, NULL);
}

static XtActionsRec actionsTable[] = {
  {"forward-character",		MoveForwardChar},
  {"backward-character",	MoveBackwardChar},
  {"delete-previous-character",	DeleteBackwardChar},
  {"delete",			Delete},
  {"accept",			Accept},
  {"insert-char",		InsertChar},
  {"set-cursor",		SetCursor},
};

static const char defaultTranslations[] =
"c<Key>H:"		"delete-previous-character()\n"
"<Key>Right:"		"forward-character()\n"
":<Key>KP_Right:"	"forward-character()\n"
"<Key>Left:"		"backward-character()\n"
":<Key>KP_Left:"	"backward-character()\n"
"<Key>Delete:"		"delete()\n"
":<Key>KP_Delete:"	"delete()\n"
"<Key>BackSpace:"	"delete-previous-character()\n"
"<Key>Return:"		"accept()\n"
":<Key>KP_Enter:"	"accept()\n"
"<Key>:"		"insert-char()\n"
"<Btn1Down>:"		"set-cursor()\n"
;

static XtResource resources[] = {
#define offset(field) XtOffsetOf(KTextLineRec, ktext_line.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNstring, XtCString, XtRString, sizeof(String),
      offset (string), XtRString, NULL },
    { XtNfont, XtCFont, XtRXkwFont, sizeof (XkwFont),
      offset (font), XtRString, XtDefaultFont },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      offset(callbacks), XtRCallback, NULL },
#undef offset
};

KTextLineClassRec ktextLineClassRec = {
  /* core */
  {
    (WidgetClass)superclass,		/* superclass */
    "KTextLine",			/* class_name */
    sizeof(KTextLineRec),		/* widget_size */
    NULL,				/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    Initialize,				/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    actionsTable,			/* actions */
    XtNumber(actionsTable),		/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    Destroy,				/* destroy */
    NULL,				/* resize */
    Redisplay,				/* expose */
    SetValues,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    (void *) defaultTranslations,	/* tm_table */
    QueryGeometry,			/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* simple */
  {
    XtInheritChangeSensitive,		/* change_sensitive */
  },
  {
    /* ksimple fields */
    /* empty			*/	0
  },
  /* ktextLine */
  {
    0,					/* extension */
  }
};

WidgetClass ktextLineWidgetClass = (WidgetClass)&ktextLineClassRec;
