/*
* $XConsortium: Hand.h,v 1.5 91/07/25 21:33:53 keith Exp $
*/

#ifndef _XtHand_h
#define _XtHand_h
#include <Xkw/Xkw.h>
#include <Xkw/KSimple.h>

/***********************************************************************
 *
 * Hand Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		    Class		RepType		Default Value
 ----		    -----		-------		-------------
 background	    Background		pixel		XtDefaultForeground
 cardWidth	    CardWidth		Dimension	0
 cardHeight	    CardHeight		Dimension	0
 displayWidth	    DisplayWidth	Dimension	0
 displayHeight	    DisplayHeight	Dimension	0
 displayX	    DisplayX		Position	0
 displayY	    DisplayY		Position	0
 internalBorderWidthInternalBorderWidth	Dimension	0
 numRows	    NumRows		Dimension	0
 numCols	    NumCols		Dimension	0
 rowsHint	    RowsHint		Boolean		False
 colsHint	    ColsHint		Boolean		False
 rowOffset	    RowOffset		Dimension	0
 colOffset	    ColOffset		Dimension	0
 displayCallback    DisplayCallback	Callback	NULL
 inputCallback	    InputCallback	Callback	NULL
 rowInsert	    Insert		Boolean		False
 colInsert	    Insert		Boolean		False
 immediateUpdate    ImmediateUpdate	Boolean		True
*/

typedef struct _HandRec *HandWidget;  /* completely defined in HandPrivate.h */
typedef struct _HandClassRec *HandWidgetClass;    /* completely defined in HandPrivate.h */

extern WidgetClass handWidgetClass;

typedef struct _HandDisplay {
    Widget	    w;
    cairo_t	    *cr;
    XtPointer	    private;
} HandDisplayRec, *HandDisplayPtr;

typedef enum {
    HandActionStart,
    HandActionDrag,
    HandActionStop,
    HandActionExpand,
} HandAction;

typedef struct _HandLocation {
    Widget	w;
    Position	row, col;
    Position    x, y;
    XtPointer	private;
} HandLocation;

typedef struct _HandInput {
    Widget	    w;
    Position	    row, col;
    XtPointer       private;
    XEvent	    event;
    HandAction	    action;
    HandLocation    start;
    String	    *params;
    Cardinal	    *num_params;
} HandInputRec, *HandInputPtr;

void
HandRectangleForPos (Widget gw, int row, int col, XRectangle *r);

XtPointer
HandAddCard (Widget	gw,
	     XtPointer	private,
	     int	row,
	     int	col,
	     int	offset);

void
HandRemoveCard (Widget gw, XtPointer card);

void
HandReplaceCard (Widget gw, XtPointer card, XtPointer private, int offset);

void
HandRemoveAllCards (Widget gw);

void
HandUpdateDisplay (Widget gw);

#define InsertRow -1
#define InsertCol -1
#define XkwHandDefaultOffset	-32767

#define XtNcardWidth	"cardWidth"
#define XtCCardWidth	"CardWidth"
#define XtNcardHeight	"cardHeight"
#define XtCCardHeight	"CardHeight"
#define XtNdisplayWidth	"displayWidth"
#define XtCDisplayWidth	"DisplayWidth"
#define XtNdisplayHeight	"displayHeight"
#define XtCDisplayHeight	"DisplayHeight"
#define XtNdisplayX	"displayX"
#define XtCDisplayX	"DisplayX"
#define XtNdisplayY	"displayY"
#define XtCDisplayY	"DisplayY"
#define XtNinternalBorderWidth	"internalBorderWidth"
#define XtCInternalBorderWidth	"InternalBorderWidth"
#define XtNnumRows	"numRows"
#define XtCNumRows	"NumRows"
#define XtNnumCols	"numCols"
#define XtCNumCols	"NumCols"
#define XtNrowsHint	"rowsHint"
#define XtCRowsHint	"RowsHint"
#define XtNcolsHint	"colsHint"
#define XtCColsHint	"ColsHint"
#define XtNcolOffset	"colOffset"
#define XtCColOffset	"ColOffset"
#define XtNrowOffset	"rowOffset"
#define XtCRowOffset	"RowOffset"
#define XtNredisplay "redisplay"
#define XtCRedisplay "Redisplay"
#define XtNrowMajor "rowMajor"
#define XtCRowMajor "RowMajor"
#define XtNdisplayCallback  "displayCallback"
#define XtCDisplayCallback  "DisplayCallback"
#define XtNinputCallback  "inputCallback"
#define XtCInputCallback  "InputCallback"
#define XtNexpandCallback  "expandCallback"
#define XtCExpandCallback  "ExpandCallback"
#define XtNrowInsert	"rowInsert"
#define XtNcolInsert	"colInsert"
#define XtCInsert	"Insert"
#define XtNimmediateUpdate  "immediateUpdate"
#define XtCImmediateUpdate  "ImmediateUpdate"
#endif /* _XtHand_h */
/* DON'T ADD STUFF AFTER THIS #endif */
