/*
 *
Copyright 1989, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */

#ifndef _XkwKSmeBSBP_h
#define _XkwKSmeBSBP_h

/*
 * Sme Object Private Data
 */
#include <Xkw/KSmeP.h>
#include <Xkw/KSmeBSB.h>
#include <Xkw/Xkw.h>
#include <X11/extensions/Xrender.h>

typedef struct _KSmeBSBClassPart {
    XtPointer extension;
} KSmeBSBClassPart;

/* Full class record declaration */
typedef struct _KSmeBSBClassRec {
    RectObjClassPart	rect_class;
    KSmeClassPart	ksme_class;
    KSmeBSBClassPart	ksme_bsb_class;
} KSmeBSBClassRec;

extern KSmeBSBClassRec smeBSBClassRec;

/* New fields for the KSme Object record */
typedef struct {
    /* resources */
    String label;			/* The entry label */
    int vert_space;			/* extra vert space to leave, as a
					   percentage of the font height of
					   the label */
    Pixmap left_bitmap, right_bitmap;	/* bitmaps to show */
    Dimension left_margin, right_margin;/* left and right margins */
    XRenderColor foreground;
    XRenderColor background;
    double dpi;
    XkwFont font;
    XtJustify justify;			/* Justification for the label. */

    /* private */
    Dimension left_bitmap_width;	/* size of each bitmap */
    Dimension left_bitmap_height;
    Dimension right_bitmap_width;
    Dimension right_bitmap_height;
} KSmeBSBPart;

/*
 * Full instance record declaration
 */
typedef struct _KSmeBSBRec {
    ObjectPart	object;
    RectObjPart	rectangle;
    KSmePart	ksme;
    KSmeBSBPart	ksme_bsb;
} KSmeBSBRec;

#endif /* _XkwKSmeBSBP_h */
