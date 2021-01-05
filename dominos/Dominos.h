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

#ifndef _XtDominos_h
#define _XtDominos_h
#include "dominos.h"
#include <Xkw/Xkw.h>

/* define exposed functions */

void
DominosSetDominos(Widget, DominoPtr *);

DominoPtr
DominosXYToDomino (Widget gw,
		   int x,
		   int y,
		   int *distp,
		   Direction *dirp);

typedef struct _DominosRec *DominosWidget;
typedef struct _DominosClassRec *DominosWidgetClass;

extern WidgetClass  dominosWidgetClass;

#define XtNfaceColor	"faceColor"
#define XtCFaceColor	"FaceColor"
#define XtNpipsColor	"pipsColor"
#define XtCPipsColor	"PipsColor"
#define XtNsize		"size"
#define XtCSize		"Size"
#define XtNinputCallback  "inputCallback"
#define XtCInputCallback  "InputCallback"

typedef enum {
    DominosActionStart,
    DominosActionDrag,
    DominosActionStop,
} DominosAction;

typedef struct _DominosInput {
    Widget	    w;
    DominosAction   action;
    DominoPtr	    domino;
    Direction	    direction;
    int		    distance;
    XEvent	    event;
    String	    *params;
    Cardinal	    *num_params;
} DominosInputRec, *DominosInputPtr;

#endif /* _XtDominos_h */
