/*
 * $XConsortium: HandP.h,v 1.9 91/07/26 15:21:49 keith Exp $
 */

/*
 * HandP.h - Private definitions for Hand widget
 */

#ifndef _XtHandP_h
#define _XtHandP_h

#include <stdlib.h>
#include "Hand.h"
#include <Xkw/KSimpleP.h>

/***********************************************************************
 *
 * Hand Widget Private Data
 *
 ***********************************************************************/

/************************************
 *
 *  Class structure
 *
 ***********************************/

/*
 * New fields for the Hand widget class record
 */

typedef struct _HandClass {
	int		makes_compiler_happy;  /* not used */
} HandClassPart;

/*
 * Full class record declaration
 */

typedef struct _HandClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    KSimpleClassPart	ksimple_class;
    HandClassPart	hand_class;
} HandClassRec;

extern HandClassRec handClassRec;

/***************************************
 *
 *  Instance (widget) structure
 *
 **************************************/

/*
 * New fields for the Hand widget record
 */

#define New(t) (t *) malloc(sizeof (t))
#define Dispose(p)  free((char *) p)
#define Some(t,n)   (t*) malloc(sizeof(t) * n)
#define More(p,t,n) ((p)? (t *) realloc((char *) p, sizeof(t)*n):Some(t,n)

typedef enum { ClipUnclipped, ClipPartclipped, ClipAllclipped } HandClip;

typedef struct _Card {
    struct _Card    *next, *prev;
    XtPointer	    private;
    int		    x, y;
    int		    row, col;
    int		    offset;
    Boolean	    shown;
} CardRec, *CardPtr;

typedef struct {
    /*
     * Resource specifiable values
     */
    Dimension	    card_width;		/* area occupied by card */
    Dimension	    card_height;	/*  */
    Dimension	    internal_border;	/* space around entire hand */
    Dimension	    num_cols;		/* number of columns */
    Dimension	    num_rows;		/* number of rows */
    Dimension	    col_offset;		/* distance to space columns apart */
    Dimension	    row_offset;		/* distance to space rows apart */
    Position	    display_x;		/* inside display rectangle, */
    Position	    display_y;		/*  the display func will fill */
    Dimension	    display_width;	/*  the entire space.  Used to */
    Dimension	    display_height;	/*  optimize redisplay */
    Boolean	    cols_hint;		/* number of columns is only a hint */
    Boolean	    rows_hint;		/* number of rows is only a hint */
    Boolean	    row_insert;		/* when inserting cards, those on */
    Boolean	    col_insert;		/*  top get moved down/right */
    Boolean	    immediate_update;	/* redisplay after every edit */
    Boolean	    row_major;		/* stack cards in cols */
    Boolean	    force_erase;	/* erase stack even if card replaces */
    XtCallbackList  display_callback;	/* func to display cards */
    XtCallbackList  input_callback;	/* func called on button press */

    /* List of cards could be changed by resource, but easier by func */
    CardPtr	    topCard, bottomCard;/* list of cards */
    Region	    damage;		/* Damage caused by card changes */
    Dimension	    real_col_offset;	/* when widget gets reshaped, */
    Dimension	    real_row_offset;	/*  the offset values are adjusted */
} HandPart;

/*
 * Full widget declaration
 */

typedef struct _HandRec {
    CorePart	core;
    SimplePart	simple;
    KSimplePart ksimple;
    HandPart	hand;
} HandRec;

#endif /* _XtHandP_h */
