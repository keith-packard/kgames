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

#ifndef _Xkw_h
#define _Xkw_h

#include <X11/Intrinsic.h>
#include <X11/Xfuncproto.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo.h>
#include <librsvg/rsvg.h>
//#include <librsvg/rsvg-cairo.h>

extern const char _XtRRenderColor[];
#define XtRRenderColor ((char *)_XtRRenderColor)
extern const char _XtRXkwFont[];
#define XtRXkwFont ((char *)_XtRXkwFont)
extern const char _XtRDpi[];
#define XtRDpi ((char *)_XtRDpi)
extern const char _XtRDouble[];
#define XtRDouble ((char *)_XtRDouble)

extern const char _XtNdpi[];
#define XtNdpi ((char *)_XtNdpi)
extern const char _XtCDpi[];
#define XtCDpi ((char *)_XtCDpi)

extern const char _XtNbackgroundColor[];
#define XtNbackgroundColor ((char *)_XtNbackgroundColor)
extern const char _XtNforegroundColor[];
#define XtNforegroundColor ((char *)_XtNforegroundColor)

typedef struct {
	cairo_font_face_t	*font_face;
	double			size;
} XkwFont;

typedef struct {
    int	*x;
    int *y;
    int *x_root;
    int *y_root;
    Window *window;
} XkwEventCoordPointers;

#ifndef XtX
#define XtX(w)            (((RectObj)w)->rectangle.x)
#endif
#ifndef XtY
#define XtY(w)            (((RectObj)w)->rectangle.y)
#endif
#ifndef XtWidth
#define XtWidth(w)        (((RectObj)w)->rectangle.width)
#endif
#ifndef XtHeight
#define XtHeight(w)       (((RectObj)w)->rectangle.height)
#endif
#ifndef XtBorderWidth
#define XtBorderWidth(w)  (((RectObj)w)->rectangle.border_width)
#endif

/*
 * Ok, Xt is surprisingly broken. Values that *can* fit in the arg
 * value are assumed to be there, even if they are passed by
 * address. We work around this by creating a macro which always uses
 * an address and then figuring out how to pass the resulting value
 * based on the size
 */
#define XkwSetArg(arg, n, d) do {			\
	(arg).name = (n);				\
	if (sizeof(*(d)) <= sizeof(XtArgVal))		\
	    memcpy(&(arg).value, (d), sizeof(*d));	\
	else						\
	    (arg).value = (XtArgVal) (d);		\
    } while(0)


_XFUNCPROTOBEGIN

static inline Boolean
XkwColorEqual(XRenderColor *a, XRenderColor *b)
{
    return memcmp(a, b, sizeof (XRenderColor)) == 0;
}

Widget
XkwInitialize(const char *class,
	      XrmOptionDescRec *options,
	      Cardinal num_options,
	      int *argc,
	      _XtString *argv,
	      Boolean input,
	      char const * const *fallback_resources);

void
XkwInitializeWidgetSet(void);

cairo_surface_t *
XkwGetSurface(Widget w);

cairo_t *
XkwGetCairo(Widget w);

void
XkwSetSource(cairo_t *cr, XRenderColor *color);

void
XkwSetSourceInterp(cairo_t *cr, XRenderColor *a, XRenderColor *b);

void
XkwDialogAddButton(Widget dialog, _Xconst char* name, XtCallbackProc function,
		   XtPointer param);

RsvgHandle *
XkwRsvgCreate(const char *str);

double
XkwRsvgAspect(RsvgHandle *rsvg);

void
XkwRsvgDraw(cairo_t *cr, int surface_width, int surface_height, RsvgHandle *rsvg);

cairo_t *
XkwDrawBegin(Widget gw, Region region);

void
XkwDrawEnd(Widget gw, Region region, cairo_t *cr);

void
XkwDrawRoundedRect(cairo_t *cr, double width, double height, double radius);

void
XkwDrawOval(cairo_t *cr, double width, double height);

void
XkwTranslateCoordsPosition(Widget to, Widget from, Position *x, Position *y);

void
XkwTranslateCoordsInt(Widget to, Widget from, int *x, int *y);

void
XkwGetEventCoordPointers(XEvent *e, XkwEventCoordPointers *cp);

void
XkwTranslateEvent(Widget to, Widget from, XEvent *e);

Bool
XkwGetEventCoords(XEvent *e, Position *x, Position *y);

Bool
XkwForwardEvent(Widget to, Widget from, XEvent *e);

_XFUNCPROTOEND

#endif /* _Xkw_h */
