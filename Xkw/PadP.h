/*
 * $NCD$
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */
#ifndef _PadP_h
#define _PadP_h

#include "Pad.h"
/* include superclass private header file */
#include <Xkw/KSimpleP.h>
#include <Xkw/Xkw.h>
#include <stdlib.h>

typedef struct {
    int empty;
} PadClassPart;

typedef struct _PadClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    KSimpleClassPart	ksimple_class;
    PadClassPart	pad_class;
} PadClassRec;

extern PadClassRec padClassRec;

typedef struct _PadLine {
    char    *text;
    char    *attr;
} PadLineRec, *PadLinePtr;

typedef struct {
    /* resources */
    XkwFont	    font;
    XRenderColor    bold;
    Dimension	    rows, cols;
    Dimension	    internal_border;
    XtCallbackList  resize_callbacks;
    /* private state */
    PadLinePtr	    lines;
    int		    underline_pos;
    int		    underline_thickness;
    int		    char_width;
    int		    char_height;
    int		    char_vAdjust;
    int		    char_hAdjust;
} PadPart;

#define New(t) (t *) malloc(sizeof (t))
#define Dispose(p)  free((char *) p)
#define Some(t,n)   (t*) malloc(sizeof(t) * n)
#define More(p,t,n) ((p)? (t *) realloc((char *) p, sizeof(t)*n):Some(t,n)

#define AllocText(b, cols) (\
    (b)->text = Some(char, (cols)+1),\
    (b)->attr = Some(char, (cols)+1),\
    Clear (b, cols))

#define DisposeText(b) (Dispose ((b)->text), Dispose ((b)->attr))

#define CopyText(fromb, tob, col, num) (\
    memcpy((tob)->text + col, (fromb)->text + col, num), \
    memcpy((tob)->attr + col, (fromb)->attr + col, num))

#define ColPos(w,x)	(((x)-(int)(w)->pad.internal_border) /\
			 (int)(w)->pad.char_width)
#define RowPos(w,y)	(((y)-(int)(w)->pad.internal_border) /\
			 (int)(w)->pad.char_height)

typedef struct _PadRec {
    CorePart		core;
    SimplePart		simple;
    KSimplePart		ksimple;
    PadPart		pad;
} PadRec;

/* declare specific PadWidget class and instance datatypes */

typedef struct _PadClassRec*	    PadWidgetClass;
typedef struct _PadRec*		    PadWidget;

#endif /* _PadP_h */
