# include	<ctype.h>
# include	<stdio.h>
# include	<stdarg.h>
# include	<string.h>
# include	<unistd.h>
# include	<stdbool.h>
# include	<stdlib.h>

# define TRUE	1
# define FALSE	0

/*
 * @(#)mille.h	1.1 (Berkeley) 4/1/82
 */

typedef short CARD;

/*
 * Miscellaneous constants
 */

# define	HAND_SZ		7	/* number of cards in a hand	*/
# define	DECK_SZ		101	/* number of cards in decks	*/
# define	NUM_SAFE	4	/* number of saftey cards	*/
# define	NUM_MILES	5	/* number of milestones types	*/
# define	NUM_CARDS	20	/* number of types of cards	*/

# define	PLAYER		0
# define	COMP		1

# define	W_SMALL		0	/* Small (initial) window	*/
# define	W_FULL		1	/* Full (final) window		*/

/*
 * Move types
 */

# define	M_DISCARD	0
# define	M_DRAW		1
# define	M_PLAY		2
# define	M_ORDER		3
# define	M_REASONABLE	4

/*
 * Scores
 */

# define	SC_SAFETY	100
# define	SC_ALL_SAFE	300
# define	SC_COUP		300
# define	SC_TRIP		400
# define	SC_SAFE		300
# define	SC_DELAY	300
# define	SC_EXTENSION	200
# define	SC_SHUT_OUT	500

/*
 * safety descriptions
 */

# define	S_UNKNOWN	0	/* location of safety unknown	*/
# define	S_IN_HAND	1	/* safety in player's hand	*/
# define	S_PLAYED	2	/* safety has been played	*/
# define	S_GAS_SAFE	0	/* Gas safety card index	*/
# define	S_SPARE_SAFE	1	/* Tire safety card index	*/
# define	S_DRIVE_SAFE	2	/* Driveing safety card index	*/
# define	S_RIGHT_WAY	3	/* Right-of-Way card index	*/
# define	S_CONV		15	/* conversion from C_ to S_	*/

/*
 * card numbers
 */

# define	C_INIT		-1
# define	C_25		0
# define	C_50		1
# define	C_75		2
# define	C_100		3
# define	C_200		4
# define	C_EMPTY		5
# define	C_FLAT		6
# define	C_CRASH		7
# define	C_STOP		8
# define	C_LIMIT		9
# define	C_GAS		10
# define	C_SPARE		11
# define	C_REPAIRS	12
# define	C_GO		13
# define	C_END_LIMIT	14
# define	C_GAS_SAFE	15
# define	C_SPARE_SAFE	16
# define	C_DRIVE_SAFE	17
# define	C_RIGHT_WAY	18

typedef struct {
	bool	coups[NUM_SAFE];
	bool	can_go;
	bool	new_battle;
	bool	new_speed;
	short	safety[NUM_SAFE];
	short	nummiles[NUM_MILES];
	CARD	hand[HAND_SZ];
	CARD	battle;
	CARD	speed;
	int	mileage;
	int	hand_tot;
	int	safescore;
	int	coupscore;
	int	total;
	int	games;
} PLAY;

/*
 * animation constants
 */

# define ANIMATE
# define ANIMATE_HAND		0
# define ANIMATE_DECK		1
# define ANIMATE_DISC		2
# define ANIMATE_MILES		3
# define ANIMATE_BATTLE		4
# define ANIMATE_SPEED		5
# define ANIMATE_OBATTLE	6
# define ANIMATE_OSPEED		7
# define ANIMATE_SAFETY		8

/*
 * macros
 */

# define	other(x)	(1 - x)
# define	nextplay()	(Play = other(Play))
# define	nextwin(x)	(1 - x)
# define	opposite(x)	(Opposite[x])
# define	is_safety(x)	(x >= C_GAS_SAFE)


/*
 * externals
 */

extern bool	Debug, Finished, Next, On_exit, Order, Saved;

extern const char *const C_fmt;
extern const char	*Fromfile;
extern const char *const *C_name;
extern char	Initstr[];

extern const int Value[], Numcards[];
extern const CARD	Opposite[NUM_CARDS];

extern int	Card_no, End, Handstart, Movetype, Numgos,
		Numneed[], Numseen[NUM_CARDS], Play, WIndow;

extern CARD	Deck[DECK_SZ], Discard, *Topcard;

extern FILE	*outf;

extern PLAY	Player[2];

/*
 * functions
 */

CARD	getcard();

void
VError(const char *string, va_list ap);

void
Error (const char *string, ...);

void
Prompt (char *string);

void
debug (int pos, char *string, int a0, int a1, int a2);

bool
error (const char *string, ...);

char *
GetpromptedInput (char *string);

void
MilleMessage (char *string);

void
stand(int y, int x, char *str);

void
InScore (int line, int player, char *text);

void
prscore(bool for_real);

/* animate.c */
void
animate_move (int player, int orig_type, int orig_arg, int dest_type, int dest_arg);

/* comp.c */
void
calcmove(void);

int
onecard(const PLAY *pp);

int
canplay(const PLAY *pp, const PLAY *op, CARD card);

/* drawcard.c */

/* end.c */
void
finalscore(PLAY *pp);

void
extrapolate(PLAY *pp);

void
undoex(void);

/* init.c */
void
init(void);

void
shuffle(void);

/* mille.c */
void
rub(int sig);

void
die(void);

/* misc.c */
bool
error(const char *str, ...);

bool
check_ext(bool forcomp);

void
check_more(void);

/* move.c */
void
domove(void);

void
check_go(void);

char *
sprint (char * string, ...);

int
haspicked(const PLAY *pp);

char *
playcard(PLAY *pp);

void
account(CARD card);

void
sort(CARD *hand);

/* print.c */

/* roll.c */
int
roll(int ndie, int nsides);

/* save.c */
bool
save(void);

bool
rest_f(const char *file);

bool
rest(void);

/* types.c */
bool
is_repair(CARD card);

int
safety(CARD card);

/* uiXt.c */
void
prboard(void);

void
do_save (void);

void
do_quit (void);

void
getmove(void);

void
ComputerCard (int type);

void
ComputerStatus (char *string);

void
ComputerDistance (int distance);

void
ComputerSpeed (int type);

void
ComputerBattle (int type);

void
ComputerMiles (int type, int ind, int count);

void
EraseComputer (void);

void
ComputerSafety (int type, int ind);

void
DisplayDiscard (int type);

void
DisplayDeck (int numberLeft);

void
HumanDistance (int distance);

void
HumanSpeed (int type);

void
HumanBattle (int type);

void
HumanMiles (int type, int ind, int count);

void
EraseHuman (void);

void
HumanSafety (int type, int ind);

void
HumanHand (int type, int ind);

void
newboard(void);

void
newscore(void);

void
draw_board (void);

void
redraw_board (void);

void
init_ui (int *argc, char **argv);

void
finish_ui (void);

void
update_ui (void);

void
Beep (void);

int
getyn(char *prompt);

void
FlushInput (void);

/* varpush.c */
int
varpush(int file, int (*func)(int, void *, size_t));
