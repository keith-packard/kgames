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

#include <math.h>
#include <Xkw/Xkw.h>

void
XkwDrawRoundedRect(cairo_t *cr, double width, double height, double radius)
{
    cairo_move_to(cr, radius, 0);
    /* top */
    cairo_line_to(cr, width - radius, 0);
    /* top right */
    cairo_arc(cr, width - radius, radius, radius, -M_PI/2, 0);
    /* right */
    cairo_line_to(cr, width, height - radius);
    /* bottom right */
    cairo_arc(cr, width - radius, height - radius, radius, 0, M_PI/2);
    /* bottom */
    cairo_line_to(cr, radius, height);
    /* bottom left */
    cairo_arc(cr, radius, height - radius, radius, M_PI/2, M_PI);
    /* left */
    cairo_line_to(cr, 0, radius);
    /* top left */
    cairo_arc(cr, radius, radius, radius, M_PI, M_PI * 3 / 2);
}

void
XkwDrawOval(cairo_t *cr, double width, double height)
{
    if (width >= height) {
	double	radius = height / 2.0;

	cairo_move_to(cr, radius, 0);
	/* top */
	cairo_line_to(cr, width - radius, 0);
	/* right */
	cairo_arc(cr, width - radius, radius, radius, -M_PI/2, M_PI/2);
	/* bottom */
	cairo_line_to(cr, radius, height);
	/* left */
	cairo_arc(cr, radius, radius, radius, M_PI/2, M_PI * 3/2);
    } else {
	double	radius = width / 2.0;

	cairo_move_to(cr, width, radius);
	/* right */
	cairo_line_to(cr, width, height - radius);
	/* bottom */
	cairo_arc(cr, radius, height - radius, radius, 0, M_PI);
	/* left */
	cairo_line_to(cr, 0, radius);
	/* top */
	cairo_arc(cr, radius, radius, radius, -M_PI, 0);
    }
}
