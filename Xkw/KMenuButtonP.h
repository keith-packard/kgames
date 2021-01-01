/*
 *
Copyright 1989,1998  The Open Group

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
 */

/*
 * KMenuButtonP.h - Private Header file for KMenuButton widget.
 *
 * This is the private header file for the Athena KMenuButton widget.
 * It is intended to provide an easy method of activating pulldown menus.
 *
 * Date:    May 2, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium
 *          kit@expo.lcs.mit.edu
 */

#ifndef _XkwKMenuButtonP_h
#define _XkwKMenuButtonP_h

#include <Xkw/KMenuButton.h>
#include <Xkw/KCommandP.h>

/* New fields for the KMenuButton widget class */
typedef struct _KMenuButtonClass {
    XtPointer extension;
} KMenuButtonClassPart;

/* class record declaration */
typedef struct _KMenuButtonClassRec {
    CoreClassPart	    core_class;
    SimpleClassPart	    simple_class;
    KSimpleClassPart	    ksimple_class;
    KLabelClassPart	    klabel_class;
    KCommandClassPart	    kcommand_class;
    KMenuButtonClassPart    kmenuButton_class;
} KMenuButtonClassRec;

extern KMenuButtonClassRec kmenuButtonClassRec;

/* New fields for the KMenuButton widget */
typedef struct {
    /* resources */
    String menu_name;
} KMenuButtonPart;

/* widget declaration */
typedef struct _KMenuButtonRec {
    CorePart         core;
    SimplePart	     simple;
    KSimplePart	     ksimple;
    KLabelPart	     label;
    KCommandPart     kcommand;
    KMenuButtonPart  menu_button;
} KMenuButtonRec;

#endif /* _XkwKMenuButtonP_h */
