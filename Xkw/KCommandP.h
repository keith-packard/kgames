/***********************************************************

Copyright 1987, 1988, 1998  The Open Group

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


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XkwKCommandP_h
#define _XkwKCommandP_h

/*
 * Command Widget Private Data
 */
#include <Xkw/KCommand.h>
#include <Xkw/KLabelP.h>

typedef enum {
    HighlightNone,		/* Do not highlight */
    HighlightWhenUnset,		/* Highlight only when unset, this is
				   to preserve current command widget
				   functionality */
    HighlightAlways		/* Always highlight, lets the toggle widget
				   and other subclasses do the right thing */
} XtCommandHighlight;

/* New fields for the KCommand widget class record */
typedef struct _KCommandClass {
    int		notused;
} KCommandClassPart;

/* Full class record declaration */
typedef struct _KCommandClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    KSimpleClassPart	ksimple_class;
    KLabelClassPart	klabel_class;
    KCommandClassPart   kcommand_class;
} KCommandClassRec;

extern KCommandClassRec kcommandClassRec;

/* New fields for the KCommand widget record */
typedef struct {
    /* resources */
    Dimension   	highlight_thickness;
    XtCallbackList	callbacks;
    int			shape_style;
    Dimension		corner_round;

    /* private state */
    Boolean	set;
    XtCommandHighlight	highlighted;

} KCommandPart;

/* Full widget declaration */
typedef struct _KCommandRec {
    CorePart         core;
    SimplePart	     simple;
    KSimplePart	     ksimple;
    KLabelPart	     klabel;
    KCommandPart     kcommand;
} KCommandRec;

#endif /* _XkwKCommandP_h */
