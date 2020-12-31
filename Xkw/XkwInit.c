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
#include <Xkw/KSimpleMenuP.h>
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

cairo_surface_t *
XkwGetSurface(Widget w)
{
    return cairo_xlib_surface_create(XtDisplay(w),
				     XtWindow(w),
				     XtScreen(w)->root_visual,
				     XtWidth(w),
				     XtHeight(w));
}

cairo_t *
XkwGetCairo(Widget w)
{
    Widget real = w;
    while (!XtIsWidget(real))
	real = XtParent(real);

    cairo_surface_t *s = XkwGetSurface(real);
    cairo_t *cr = cairo_create(s);
    cairo_translate(cr, XtX(w) - XtX(real), XtY(w) - XtY(real));
    cairo_surface_destroy (s);
    return cr;
}

static XRectangle
XkwDrawRect(Widget gw, Region region)
{
    XRectangle ret = { .x = 0, .y = 0, .width = XtWidth(gw), .height = XtHeight(gw) };

    if (region) {
	XRectangle clip;

	XClipBox(region, &clip);
	if (clip.x > 0) {
	    ret.x = clip.x;
	    ret.width -= clip.x;
	} else
	    clip.width += clip.x;
	if (clip.y > 0) {
	    ret.y = clip.y;
	    ret.height -= clip.y;
	} else
	    clip.height += clip.y;

	if (ret.width > clip.width)
	    ret.width = clip.width;
	if (ret.height > clip.height)
	    clip.height = clip.height;
    }
    return ret;
}

cairo_t *
XkwDrawBegin(Widget gw, Region region)
{
    Widget		greal = gw;

    while (!XtIsWidget(greal))
	greal = XtParent(greal);

    Boolean		surface_created = False;
    cairo_surface_t	*surface = NULL;
    XRenderColor	*bg = NULL;
    XRenderColor	*fg = NULL;

    if (XtIsSubclass(greal, ksimpleWidgetClass)) {
	KSimpleWidget 	real = (KSimpleWidget) greal;
	surface = real->ksimple.surface;
	fg = &real->ksimple.foreground;
	bg = &real->ksimple.background;
    } else if (XtIsSubclass(greal, ksimpleMenuWidgetClass)) {
	KSimpleMenuWidget real = (KSimpleMenuWidget) greal;
	surface = real->ksimple_menu.surface;
	fg = &real->ksimple_menu.foreground;
	bg = &real->ksimple_menu.background;
    } else {
	surface = XkwGetSurface(greal);
	surface_created = True;
    }

    XRectangle rect = XkwDrawRect(gw, region);

    cairo_surface_t *pix = cairo_surface_create_similar(surface,
							CAIRO_CONTENT_COLOR,
							rect.width, rect.height);
    if (surface_created)
	cairo_surface_destroy(surface);

    cairo_t *cr = cairo_create(pix);

    cairo_translate(cr, -rect.x, -rect.y);

    cairo_surface_destroy(pix);
    if (bg) {
	XkwSetSource(cr, bg);
	cairo_paint(cr);
    }
    if (fg)
	XkwSetSource(cr, fg);
    return cr;
}

void
XkwDrawEnd(Widget gw, Region region, cairo_t *cr)
{
    Widget		greal = gw;

    while (!XtIsWidget(greal))
	greal = XtParent(greal);

    Boolean		surface_created = False;
    cairo_surface_t	*surface = NULL;

    if (XtIsSubclass(greal, ksimpleWidgetClass)) {
	KSimpleWidget 	real = (KSimpleWidget) greal;
	surface = real->ksimple.surface;
    } else if (XtIsSubclass(greal, ksimpleMenuWidgetClass)) {
	KSimpleMenuWidget real = (KSimpleMenuWidget) greal;
	surface = real->ksimple_menu.surface;
    } else {
	surface = XkwGetSurface(greal);
	surface_created = True;
    }

    cairo_t 		*dest = cairo_create(surface);

    if (surface_created)
	cairo_surface_destroy(surface);

    XRectangle rect = XkwDrawRect(gw, region);

    double x = rect.x;
    double y = rect.y;
    if (gw != greal) {
	x += XtX(gw);
	y += XtY(gw);
    }
    cairo_set_source_surface(dest, cairo_get_target(cr), x, y);

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
	XtAddConverter(XtRString, XtRJustify, XmuCvtStringToJustify, NULL, 0);
	XtSetTypeConverter(XtRJustify, XtRString, XmuCvtJustifyToString,
			   NULL, 0, XtCacheNone, NULL);
    }
}
