/*
 * $NCDId$
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

#ifndef _DOMINOS_H_
#define _DOMINOS_H_

#include <stdio.h>
#include <X11/Xdefs.h>

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

typedef unsigned char	Pips;

typedef struct _domino	*DominoPtr;

typedef enum _Direction { North = 0, East = 1, South = 2, West = 3 } Direction;

typedef int Player;

typedef struct _domino {
    Pips	pips[2];
    Direction	orientation;
    Bool	hide;
    DominoPtr	peer[4];
} DominoRec;

#define LinkPeer    East

#include <stdlib.h>

#define New(type)   (type *) malloc (sizeof (type))
#define Dispose(p)  free ((char *) (p))
#define Some(type,n)	(type *) malloc ((n) * sizeof (type))
#define More(type,p,n)	((type *) ((p) ? realloc (p, (n) * sizeof (type)) : Some(type,n)))

#define MAX_DOMINO_PIP	9
#define PLAYER_START	7
#define MAX_PLAYERS	16

extern int	    NumPlayers;
extern DominoPtr    pile, player[], board;

typedef struct _Rect {
    int	    x1, y1, x2, y2;
} RectRec, *RectPtr;

typedef struct _Play {
    DominoPtr	*player;
    DominoPtr	source, target;
    int		dist;
    Direction	dir;
    Direction	orientation;
} PlayRec, *PlayPtr;

typedef struct _Undo *UndoPtr;

typedef struct _Undo {
    UndoPtr	next;
    DominoPtr	*player;
    DominoPtr	*dest;
    DominoPtr	*source;
    DominoPtr	domino;
    Direction	orientation;
} UndoRec;

extern UndoPtr	    undoList;

DominoPtr
MakeDomino(Pips a, Pips b);

DominoPtr
InitDominos (Pips max);

DominoPtr
MixDominos (DominoPtr dominos);

DominoPtr
PickDomino (DominoPtr *dominos);

void
DisposeDominos (DominoPtr domino);

DominoPtr
ReadDominos (FILE *file);

void
WriteDominos (FILE *file, DominoPtr d);

void
WriteScores (FILE *file, int *scores, int num);

int
ReadScores (FILE *file, int *scores, int num);

void
WriteInt (FILE *file, int i);

int
ReadInt (FILE *file, int *i);

void
FileError (char *s);

extern int	    DominoErrno;

int
TraverseDominos (DominoPtr d, int (*func)(DominoPtr, pointer), pointer data);

int
FindPlays (DominoPtr board,
	   DominoPtr domino,
	   int (*func)(DominoPtr, DominoPtr, Direction, Direction, pointer),
	   pointer data);

void
DisposeGame (void);

void
ResetGame (void);

int
PlayerDraw(DominoPtr *p, int remember);

DominoPtr *
PlayerExtract (DominoPtr *p, DominoPtr source);

int
PlayerMove (DominoPtr *p,
	    DominoPtr source,
	    DominoPtr target,
	    Direction dir,
	    Direction orientation);

void
PlayerFirstMove (DominoPtr *p, DominoPtr source);

int
PlayerUndo (void);

void
DisposeUndoList (void);

int
IsDouble(DominoPtr d);

Direction
OtherDir (Direction dir);

int
CanUseEdge (DominoPtr d, Direction dir, Direction orientation);

Pips
EdgePips (DominoPtr d, Direction dir, Direction orientation);

int
CanPlay (DominoPtr source, DominoPtr target, Direction dir, Direction orientation);

int
MakeFirstPlay (DominoPtr source,
	       DominoPtr target,
	       Direction dir,
	       Direction orientation,
	       pointer data);

int
FindPlay (DominoPtr *player, PlayPtr play);

#endif /* _DOMINOS_H_ */
