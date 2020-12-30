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

#include <Xkw/Xkw.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <Xkw/KSimpleP.h>
#include <cairo/cairo-ft.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xft/Xft.h>

const char _XtRRenderColor[] = "RenderColor";
const char _XtRXkwFont[] = "XkwFont";
const char _XtRDpi[] = "Dpi";

const char _XtNdpi[] = "dpi";
const char _XtCDpi[] = "Dpi";

const char _XtNbackgroundColor[] = "backgroundColor";
const char _XtNforegroundColor[] = "foregroundColor";

#define	donestr(type, value, tstr)					\
    {									\
	if (toVal->addr != NULL) {					\
	    if (toVal->size < sizeof(type)) {				\
		toVal->size = sizeof(type);				\
		XtDisplayStringConversionWarning(dpy,			\
						 (char*) fromVal->addr, tstr); \
		return False;						\
	    }								\
	    *(type*)(toVal->addr) = (value);				\
	}								\
	else {								\
	    static type static_val;					\
	    static_val = (value);					\
	    toVal->addr = (XPointer)&static_val;			\
	}								\
	toVal->size = sizeof(type);					\
	return True;							\
    }

static Boolean
XkwCvtStringToRenderColor(Display *dpy,
			  XrmValue *args, Cardinal *num_args,
			  XrmValue *fromVal, XrmValue *toVal,
			  XtPointer *converter_data)
{
    char	    *spec;
    XRenderColor    renderColor;

    spec = (char *) fromVal->addr;
    if (strcasecmp (spec, XtDefaultForeground) == 0)
    {
	renderColor.red = 0;
	renderColor.green = 0;
	renderColor.blue = 0;
	renderColor.alpha = 0xffff;
    }
    else if (strcasecmp (spec, XtDefaultBackground) == 0)
    {
	renderColor.red = 0xffff;
	renderColor.green = 0xffff;
	renderColor.blue = 0xffff;
	renderColor.alpha = 0xffff;
    }
    else if (!XRenderParseColor (dpy, spec, &renderColor))
	return False;

    donestr (XRenderColor, renderColor, XtRRenderColor);
}

static Boolean
XkwCvtStringToXkwFont(Display *dpy,
		      XrmValue *args, Cardinal *num_args,
		      XrmValue *fromVal, XrmValue *toVal,
		      XtPointer *converter_data)
{
    char		*string_name = (char *) fromVal->addr;
    FcPattern		*pat, *match;
    FcResult		result;
    XkwFont	    	xkwFont;

    if (strncmp(string_name, "fixed", 5) == 0)
	string_name = "monospace-12";
    if (*string_name == '-') {
	pat = XftXlfdParse(string_name, False, False);
    } else {
	pat = FcNameParse ((FcChar8 *) string_name);
    }
    if (!pat)
	return False;

    FcConfigSubstitute (0, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);

    match = FcFontMatch (0, pat, &result);
    FcPatternDestroy (pat);
    if (!match)
	return False;

    FcPatternGetDouble (match, FC_SIZE, 0, &xkwFont.size);
    xkwFont.font_face = cairo_ft_font_face_create_for_pattern (match);

    FcPatternDestroy (match);
    if (!xkwFont.font_face)
	return False;

    donestr (XkwFont, xkwFont, XtRXkwFont);
}

static void
XkwFreeXkwFont(XtAppContext app_context,
	       XrmValue *val,
	       XtPointer converter_data,
	       XrmValue *args,
	       Cardinal *num_args)
{
    XkwFont	*xkwFont = (XkwFont *) val;

    cairo_font_face_destroy(xkwFont->font_face);
}

static Boolean
xkw_str_to_double(char *string, double *result)
{
    char *end;
    double d;

    if (!string)
	return False;
    d = strtod(string, &end);
    if (end == string)
	return False;
    *result = d;
    return True;
}

static Boolean
XkwCvtStringToDpi(Display *dpy,
		  XrmValue *args, Cardinal *num_args,
		  XrmValue *fromVal, XrmValue *toVal,
		  XtPointer *converter_data)
{
    double	dpi = 0.0;

    if (!xkw_str_to_double((char *) fromVal->addr, &dpi)) {
	if (!xkw_str_to_double(XGetDefault(dpy, "Xft", "dpi"), &dpi)) {
	    int screen = DefaultScreen (dpy);
	    int	height_pix = DisplayHeight(dpy, screen);
	    int	height_mm = DisplayHeightMM(dpy, screen);

	    dpi = (double) height_pix / ((double) height_mm / 25.4);
	}
    }

    donestr (double, dpi, XtRDpi);
}

cairo_t *
XkwGetCairo(Widget w)
{
    while (!XtIsWidget(w))
	w = XtParent(w);
    cairo_surface_t	*s = cairo_xlib_surface_create(XtDisplay(w),
						       XtWindow(w),
						       XtScreen(w)->root_visual,
						       XtWidth(w),
						       XtHeight(w));
    cairo_t *cr = cairo_create(s);
    cairo_surface_destroy (s);
    return cr;
}

cairo_t *
XkwDrawBegin(Widget gw)
{
    KSimpleWidget 	w = (KSimpleWidget) gw;
    cairo_surface_t	*pix;
    cairo_t		*cr;

    pix = cairo_surface_create_similar(w->ksimple.surface,
				       CAIRO_CONTENT_COLOR,
				       XtWidth(w),
				       XtHeight(w));
    cr = cairo_create(pix);
    cairo_surface_destroy(pix);
    XkwSetSource(cr, &w->ksimple.background);
    cairo_paint(cr);
    XkwSetSource(cr, &w->ksimple.foreground);
    return cr;
}

void
XkwDrawEnd(Widget gw, cairo_t *cr)
{
    KSimpleWidget 	w = (KSimpleWidget) gw;
    cairo_t 		*dest = cairo_create(w->ksimple.surface);

    cairo_set_source_surface(dest, cairo_get_target(cr), 0, 0);
    cairo_paint(dest);
    cairo_destroy(cr);
    cairo_destroy(dest);
}

void
XkwSetSource(cairo_t *cr, XRenderColor *color)
{
    cairo_set_source_rgba(cr,
			  color->red / 65535.0,
			  color->green / 65535.0,
			  color->blue / 65535.0,
			  color->alpha / 65535.0);
}

void
XkwSetSourceInterp(cairo_t *cr, XRenderColor *a, XRenderColor *b)
{
    cairo_set_source_rgba(cr,
			  a->red / (2.0 * 65535.0) + b->red / (2.0 * 65535.0),
			  a->green / (2.0 * 65535.0) + b->green / (2.0 * 65535.0),
			  a->blue / (2.0 * 65535.0) + b->blue / (2.0 * 65535.0),
			  a->alpha / (2.0 * 65535.0) + b->alpha / (2.0 * 65535.0));
}

void
XkwInitializeWidgetSet(void)
{
    static Boolean init_done;

    if (!init_done) {
	init_done = True;
	XtSetTypeConverter(XtRString, XtRRenderColor,
			   XkwCvtStringToRenderColor,
			   NULL, 0,
			   XtCacheByDisplay, NULL);
	XtSetTypeConverter(XtRString, XtRXkwFont,
			   XkwCvtStringToXkwFont,
			   NULL, 0,
			   XtCacheAll, XkwFreeXkwFont);
	XtSetTypeConverter(XtRString, XtRDpi,
			   XkwCvtStringToDpi,
			   NULL, 0,
			   XtCacheByDisplay, NULL);
    }
}
