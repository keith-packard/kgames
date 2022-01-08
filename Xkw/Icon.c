/*
 * Copyright © 2021 Keith Packard
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

#include <stdint.h>
#include "Xkw.h"
#include "cards-svg.h"

static Dimension icon_sizes[] = {32, 64, 128};

#define NUM_SIZES (sizeof(icon_sizes)/sizeof(icon_sizes[0]))

static unsigned long *
XkwIconValue(unsigned long *total_sizep, const char *svg)
{
    unsigned long       *value = NULL;
    unsigned long       *pixels;
    unsigned long       total_size;
    int                 i;
    int                 x, y;
    RsvgHandle          *svg_handle = NULL;

    svg_handle = XkwRsvgCreate(svg);
    if (!svg_handle)
        return NULL;

    total_size = 0;
    for (i = 0; i < NUM_SIZES; i++) {
        total_size += 2;                                /* width/height */
        total_size += icon_sizes[i] * icon_sizes[i];    /* pixels */
    }
    value = calloc(total_size, sizeof(unsigned long));
    if (!value)
        return NULL;

    pixels = value;
    for (i = 0; i < NUM_SIZES; i++) {
        Dimension width = icon_sizes[i];
        Dimension height = icon_sizes[i];
        *pixels++ = width;
        *pixels++ = height;

        uint32_t stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
        unsigned char *surface_data = calloc(stride, height);

        if (!surface_data)
            goto bail;

        cairo_surface_t *surface = cairo_image_surface_create_for_data(surface_data,
                                                                       CAIRO_FORMAT_ARGB32,
                                                                       width, height,
                                                                       cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width));
        if (!surface) {
            free(surface_data);
            goto bail;
        }

        cairo_t *cr = cairo_create(surface);
        if (!cr) {
            free(surface_data);
            cairo_surface_destroy(surface);
            goto bail;
        }

        XkwRsvgDraw(cr, width, height, svg_handle);

        cairo_destroy(cr);

        cairo_surface_destroy(surface);

        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                uint8_t a, r, g, b;

                a = surface_data[y * stride + x * 4 + 3];
                r = surface_data[y * stride + x * 4 + 2];
                g = surface_data[y * stride + x * 4 + 1];
                b = surface_data[y * stride + x * 4 + 0];

                uint32_t pixel = (a << 24) | (r << 16) | (g << 8) | (b);

                pixels[y * width + x] = pixel;
            }
        }

        free(surface_data);

        pixels += width * height;
    }

    XkwRsvgDestroy(svg_handle);

    *total_sizep = total_size;
    return value;
bail:
    if (value)
        free(value);
    if (svg_handle)
        XkwRsvgDestroy(svg_handle);
    return NULL;
}

void
XkwSetIcon(Widget toplevel, const char *svg)
{
    unsigned long *value;
    unsigned long size;

    Atom _net_wm_icon = XInternAtom(XtDisplay(toplevel),
                                    "_NET_WM_ICON", FALSE);

    value = XkwIconValue(&size, svg);

    if (value) {
        XChangeProperty(XtDisplay(toplevel), XtWindow(toplevel),
                        _net_wm_icon, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *) value, (int) size);
        free(value);
    }
}

void
XkwSetCardIcon(Widget toplevel)
{
    XkwSetIcon(toplevel, svg_card_game);
}
