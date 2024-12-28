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
#include <math.h>
//#include <librsvg/rsvg-cairo.h>

RsvgHandle *
XkwRsvgCreate(const char *str)
{
	GError		*error;

	error = NULL;
	return rsvg_handle_new_from_data((const guint8 *) str, strlen(str), &error);
}

void
XkwRsvgDestroy(RsvgHandle *handle)
{
        g_object_unref(handle);
}

double
XkwRsvgAspect(RsvgHandle *rsvg)
{
        double  width, height;
	rsvg_handle_get_intrinsic_size_in_pixels(rsvg, &width, &height);

	if (height == 0)
		return INFINITY;
	return width / height;
}

void
XkwRsvgDraw(cairo_t *cr, int surface_width, int surface_height, RsvgHandle *rsvg)
{
        RsvgRectangle   viewport = {
                .x = 0, .y = 0, .width = surface_width, .height = surface_height
        };
        rsvg_handle_render_document(rsvg, cr, &viewport, NULL);
}
