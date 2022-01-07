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
#include <librsvg/rsvg.h>
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
	RsvgDimensionData 	dimension_data;
	rsvg_handle_get_dimensions(rsvg, &dimension_data);

	if (dimension_data.height == 0)
		return INFINITY;
	return (double) dimension_data.width / (double) dimension_data.height;
}

void
XkwRsvgDraw(cairo_t *cr, int surface_width, int surface_height, RsvgHandle *rsvg)
{
	RsvgDimensionData 	dimension_data;
	double			scale_x;
	double			scale_y;

	cairo_save(cr);
	rsvg_handle_get_dimensions(rsvg, &dimension_data);
	scale_x = (double) surface_width / (double) dimension_data.width;
	scale_y = (double) surface_height / (double) dimension_data.height;
	cairo_scale(cr, scale_x, scale_y);
	rsvg_handle_render_cairo(rsvg, cr);
	cairo_restore(cr);
}
