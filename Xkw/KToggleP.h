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

#ifndef _XkwKToggleP_h_
#define _XkwKToggleP_h_

#include <Xkw/Xkw.h>
#include <Xkw/KToggle.h>
#include <Xkw/KCommandP.h>

typedef struct _KToggleClass {
    int unused;
} KToggleClassPart;

typedef struct _KToggleClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    KSimpleClassPart	ksimple_class;
    KLabelClassPart	klabel_class;
    KCommandClassPart	kcommand_class;
    KToggleClassPart	ktoggle_class;
} KToggleClassRec;

extern KToggleClassRec ktoggleClassRec;

typedef struct {
    /* resources */
    Widget radio_leader;	/* leader of radio group */
    XtPointer radio_data;	/* application data for radio group */

    /* private */
    KToggleWidget radio_prev;		/* previous radio button */
    KToggleWidget radio_next;		/* next radio button */
} KTogglePart;

typedef struct _KToggleRec {
    CorePart		core;
    SimplePart		simple;
    KSimplePart		ksimple;
    KLabelPart		klabel;
    KCommandPart	kcommand;
    KTogglePart		ktoggle;
} KToggleRec;

#endif /* _XkwKToggleP_h_ */
