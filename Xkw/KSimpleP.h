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

#ifndef _XkwKSimpleP_h_
#define _XkwKSimpleP_h_

#include <Xkw/Xkw.h>
#include <X11/IntrinsicP.h>
#include <Xkw/KSimple.h>
#include <X11/Xaw/SimpleP.h>
#include <cairo/cairo-xlib.h>

_XFUNCPROTOBEGIN

typedef struct {
    int	unused;
} KSimpleClassPart;

typedef struct _KSimpleClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    KSimpleClassPart	ksimple_class;
} KSimpleClassRec;

extern KSimpleClassRec ksimpleClassRec;

typedef struct {
    /* resources */
    XRenderColor	background;
    XRenderColor	foreground;
    double		dpi;

    /* private */
    cairo_surface_t	*surface;
} KSimplePart;

typedef struct _KSimpleRec {
    CorePart	core;
    SimplePart	simple;
    KSimplePart	ksimple;
} KSimpleRec;

_XFUNCPROTOEND

#endif /* _XkwKSimple_h_ */
