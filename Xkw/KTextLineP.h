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

#ifndef _XkwKTextLineP_h_
#define _XkwKTextLineP_h_

#include <Xkw/Xkw.h>
#include <Xkw/KTextLine.h>
#include <Xkw/KSimpleP.h>

typedef struct _KTextLineClass {
    int	notused;
} KTextLineClassPart;

/* Full class record declaration */
typedef struct _KCommandClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    KSimpleClassPart	ksimple_class;
    KTextLineClassPart  ktext_line_class;
} KTextLineClassRec;

extern KTextLineClassRec ktextLineClassRec;

typedef struct {
    /* resources */
    XkwFont		font;
    String		string;
    XtCallbackList 	callbacks;

    /* private */
    int			size;
    int			cursor;
} KTextLinePart;

/*
 * Full instance record declaration
 */
typedef struct _KTextLineRec {
    CorePart		core;
    SimplePart		simple;
    KSimplePart		ksimple;
    KTextLinePart	ktext_line;
} KTextLineRec;

#endif /* _XkwKTextLineP_h_ */
