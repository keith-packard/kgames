/*
 *
Copyright 1989, 1998  The Open Group

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

#ifndef _XkwKSmeLineP_h
#define _XkwKSmeLineP_h

/*
 * KSmeLine Widget Private Data
 */
#include <Xkw/KSmeP.h>
#include <Xkw/KSmeLine.h>
#include <X11/extensions/Xrender.h>

/* New fields for the KSmeLine widget class */
typedef struct _KSmeLineClassPart {
    XtPointer extension;
} KSmeLineClassPart;

/* Full class record */
typedef struct _KSmeLineClassRec {
    RectObjClassPart    rect_class;
    KSmeClassPart	ksme_class;
    KSmeLineClassPart	ksme_line_class;
} KSmeLineClassRec;

extern KSmeLineClassRec smeLineClassRec;

/* New fields for the KSmeLine widget */
typedef struct {
    /* resources */
    XRenderColor foreground;
    XRenderColor background;
    Dimension line_width;	/* Width of the line */
} KSmeLinePart;

/* Full instance record */
typedef struct _KSmeLineRec {
    ObjectPart	object;
    RectObjPart	rectangle;
    KSmePart	ksme;
    KSmeLinePart	ksme_line;
} KSmeLineRec;

#endif /* _XkwKSmeLineP_h */
