/*
 *
Copyright 1990, 1994, 1998  The Open Group

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
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifndef _XkwKPortholeP_h
#define _XkwKPortholeP_h

#include <Xkw/Xkw.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xkw/KPorthole.h>

/* new fields in widget class */
typedef struct {
    XtPointer extension;
} KPortholeClassPart;

/* widget class */
typedef struct _KPortholeClassRec {
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    KPortholeClassPart kporthole_class;
} KPortholeClassRec;

/* new fields in widget */
typedef struct {
    /* resources */
    XtCallbackList callbacks;

    /* private */
    Boolean dragging;
    Position start_x, start_y;
    Position child_start_x, child_start_y;
} KPortholePart;

typedef struct _KPortholeRec {
    CorePart core;
    CompositePart composite;
    KPortholePart kporthole;
} KPortholeRec;

extern KPortholeClassRec kportholeClassRec;

#endif /* _XkwKPortholeP_h */
