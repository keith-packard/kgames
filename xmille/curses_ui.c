/*
 * ui.c
 *
 * curses interface routines for mille
 */

# include	"mille.h"
#undef TRUE
#undef FALSE
# include	"ui.h"
#ifdef CTRL
# undef CTRL
#endif
#define CTRL(x)	(x - 'A' + 1)

WINDOW	*Board, *Miles, *Score;

char	*C_fmt = "%-18.18s";	/* format for printing cards		*/
char	Initstr[100];		/* initial string for error field	*/
char	*_cn[NUM_CARDS] = {	/* Card name buffer			*/
		"",
		"25",
		"50",
		"75",
		"100",
		"200",
		"Out of Gas",
		"Flat Tire",
		"Accident",
		"Stop",
		"Speed Limit", 
		"Gasoline",
		"Spare Tire",
		"Repairs",
		"Go",
		"End of Limit",
		"Extra Tank",
		"Puncture Proof",
		"Driving Ace",
		"Right of Way"
	},
	**C_name = &_cn[1];	/* Card names				*/

Message (string)
char	*string;
{
	wmove (Score, ERR_Y, ERR_X);
	waddstr (Score, string);
	wclrtoeol (Score);
}

debug (pos, string, a0, a1, a2)
{
	mvprintw (pos+6, 2, string, a0, a1, a2);
}

ComputerStatus (string)
{
	mvaddstr (MOVE_Y + 1, MOVE_X, string);
}

ComputerCard (type)
int	type;
{
	mvprintw (MOVE_Y + 2, MOVE_X, "%16s", C_name[type]);
}


Prompt (string)
char *string;
{
	mvaddstr (MOVE_Y, MOVE_X, string);
	clrtoeol ();
	refresh ();
}

char *
GetpromptedInput (string)
char	*string;
{
	static char	buf[1024];
	char		*sp;
	
	sp = buf;
	Prompt (string);
	leaveok (Board, FALSE);
	while ((*sp = readch()) != '\n') {
		if (*sp == _tty.sg_kill) {
			sp = buf;
			break;
		}
		else if (*sp == _tty.sg_erase) {
			if (--sp < buf)
				sp = buf;
			else {
				addch('\b');
				/*
				 * if the previous char was a control
				 * char, cover up two characters.
				 */
				if (*sp < ' ')
					addch('\b');
				clrtoeol();
			}
		}
		else
			addstr(unctrl(*sp++));
		refresh();
	}
	*sp = '\0';
	leaveok (Board, TRUE);
	return buf;
}

newboard()
{
	werase(Board);
	werase(Score);
	mvaddstr(5, 0, "--HAND--");
	mvaddch(6, 0, 'P');
	mvaddch(7, 0, '1');
	mvaddch(8, 0, '2');
	mvaddch(9, 0, '3');
	mvaddch(10, 0, '4');
	mvaddch(11, 0, '5');
	mvaddch(12, 0, '6');
	mvaddstr(13, 0, "--BATTLE--");
	mvaddstr(15, 0, "--SPEED--");
	mvaddstr(5, 20, "--DECK--");
	mvaddstr(7, 20, "--DISCARD--");
	mvaddstr(13, 20, "--BATTLE--");
	mvaddstr(15, 20, "--SPEED--");
	wmove(Miles, 0, 0);
	if (winch(Miles) != '-') {
		werase(Miles);
		mvwaddstr(Miles, 0, 0, "--MILEAGE--");
		mvwaddstr(Miles, 0, 41, "--MILEAGE--");
	}
	else {
		wmove(Miles, 1, 0);
		wclrtobot(Miles);
	}
	newscore();
	stdscr = Board;
}

newscore() {

	reg int	i;

	stdscr = Score;
	move(0, 22);
	if (inch() != 'Y') {
		erase();
		mvaddstr(0, 22,  "You   Comp   Value");
		mvaddstr(1, 2, "Milestones Played");
		mvaddstr(2, 8, "Each Safety");
		mvaddstr(3, 5, "All 4 Safeties");
		mvaddstr(4, 3, "Each Coup Fourre");
		mvaddstr(2, 37, "100");
		mvaddstr(3, 37, "300");
		mvaddstr(4, 37, "300");
	}
	else {
		move(5, 1);
		clrtobot();
	}
	for (i = 0; i < SCORE_Y; i++)
		mvaddch(i, 0, '|');
	move(SCORE_Y - 1, 1);
	while (addch('_') != ERR)
		continue;
	if (WIndow == W_FULL || Finished) {
		mvaddstr(5, 5, "Trip Completed");
		mvaddstr(6, 10, "Safe Trip");
		mvaddstr(7, 5, "Delayed Action");
		mvaddstr(8, 10, "Extension");
		mvaddstr(9, 11, "Shut-Out");
		mvaddstr(10, 21, "----   ----   -----");
		mvaddstr(11, 9, "Hand Total");
		mvaddstr(12, 20, "-----  -----");
		mvaddstr(13, 6, "Overall Total");
		mvaddstr(14, 15, "Games");
		mvaddstr(5, 37, "400");
		mvaddstr(6, 37, "300");
		mvaddstr(7, 37, "300");
		mvaddstr(8, 37, "200");
		mvaddstr(9, 37, "500");
	}
	else {
		mvaddstr(5, 21, "----   ----   -----");
		mvaddstr(6, 9, "Hand Total");
		mvaddstr(7, 20, "-----  -----");
		mvaddstr(8, 6, "Overall Total");
		mvaddstr(9, 15, "Games");
		mvaddstr(11, 2, "p: pick");
		mvaddstr(12, 2, "u: use #");
		mvaddstr(13, 2, "d: discard #");
		mvaddstr(14, 2, "w: toggle window");
		mvaddstr(11, 21, "q: quit");
		mvaddstr(12, 21, "o: order hand");
		mvaddstr(13, 21, "s: save");
		mvaddstr(14, 21, "r: reprint");
	}
	stdscr = Board;
}

init_ui ()
{
	initscr();
# ifdef attron
#	define	CA	cursor_address
# endif
	if (!CA) {
		printf("Sorry.  Need cursor addressing to play mille\n");
		exit(-1);
	}
	delwin(stdscr);
	stdscr = Board = newwin(BOARD_Y, BOARD_X, 0, 0);
	Score = newwin(SCORE_Y, SCORE_X, 0, 40);
	Miles = newwin(MILES_Y, MILES_X, 17, 0);
#ifdef attron
	idlok(Board, TRUE);
	idlok(Score, TRUE);
	idlok(Miles, TRUE);
#endif
	leaveok(Score, TRUE);
	leaveok(Miles, TRUE);
	clearok(curscr, TRUE);
	crmode();
	noecho();
}

Error (string, arg)
char *string;
{
	stdscr = Score;
	mvprintw (Score, ERR_Y, ERR_X, string, arg);
	clrtoeol ();
	stdscr = Board;
}

finish_ui ()
{
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
}

update_ui ()
{
	refresh ();
}

Beep ()
{
	putchar ('\007');
}

CARD
getcard()
{
	reg char	c, c1;

	for (;;) {
		while ((c = readch()) == '\n' || c == '\r' || c == ' ')
			continue;
		if (islower(c))
			c = toupper(c);
		if (c == _tty.sg_kill || c == _tty.sg_erase)
			return -1;
		addstr(unctrl(c));
		clrtoeol();
		switch (c) {
		  case '1':	case '2':	case '3':
		  case '4':	case '5':	case '6':
			c -= '0';
			break;
		  case '0':	case 'P':	case 'p':
			c = 0;
			break;
		  default:
			putchar('');
			addch('\b');
			if (!isprint(c))
				addch('\b');
			c = -1;
			break;
		}
		refresh();
		if (c >= 0) {
			while ((c1=readch()) != '\r' && c1 != '\n' && c1 != ' ')
				if (c1 == _tty.sg_kill)
					return -1;
				else if (c1 == _tty.sg_erase) {
					addch('\b');
					clrtoeol();
					refresh();
					goto cont;
				}
				else
					write(0, "", 1);
			return c;
		}
cont:		;
	}
}


/*
 *	Get a yes or no answer to the given question.  Saves are
 * also allowed.  Return TRUE if the answer was yes, FALSE if no.
 */
getyn(prompt)
reg char	*prompt; {

	reg char	c;

	Saved = FALSE;
	for (;;) {
		leaveok(Board, FALSE);
		mvaddstr(MOVE_Y, MOVE_X, prompt);
		clrtoeol();
		refresh();
		switch (c = readch()) {
		  case 'n':	case 'N':
			addch('N');
			refresh();
			leaveok(Board, TRUE);
			return FALSE;
		  case 'y':	case 'Y':
			addch('Y');
			refresh();
			leaveok(Board, TRUE);
			return TRUE;
		  case 's':	case 'S':
			addch('S');
			refresh();
			Saved = save();
			continue;
		  default:
			addstr(unctrl(c));
			refresh();
			putchar('');
			break;
		}
	}
}

readch()
{
	reg int		cnt;
	static char	c;

	for (cnt = 0; read(0, &c, 1) <= 0; cnt++)
		if (cnt > 100)
			exit(1);
	return c;
}


getmove()
{
	reg char	c, *sp;
	static char	moveprompt[] = ">>:Move:";
#ifdef EXTRAP
	static bool	last_ex = FALSE;	/* set if last command was E */

	if (last_ex) {
		undoex();
		prboard();
		last_ex = FALSE;
	}
#endif
	for (;;) {
		stand(MOVE_Y, MOVE_X, moveprompt);
		clrtoeol();
		move(MOVE_Y, MOVE_X + sizeof moveprompt);
		leaveok(Board, FALSE);
		refresh();
		while ((c = readch()) == _tty.sg_kill || c == _tty.sg_erase)
			continue;
		if (islower(c))
			c = toupper(c);
		if (isprint(c) && !isspace(c)) {
			addch(c);
			refresh();
		}
		switch (c)
		{
		  case 'P':		/* Pick */
			Movetype = M_DRAW;
			goto ret;
		  case 'U':		/* Use Card */
		  case 'D':		/* Discard Card */
			if ((Card_no = getcard()) < 0)
				break;
			Movetype = (c == 'U' ? M_PLAY : M_DISCARD);
			goto ret;
		  case 'O':		/* Order */
			Order = !Order;
			Movetype = M_ORDER;
			goto ret;
		  case 'Q':		/* Quit */
			rub();		/* Same as a rubout */
			break;
		  case 'W':		/* WIndow toggle */
			WIndow = nextwin(WIndow);
			newscore();
			prscore(TRUE);
			wrefresh(Score);
			break;
		  case 'R':		/* Redraw screen */
		  case CTRL('L'):
			clearok(curscr, TRUE);
			newboard();
			prboard();
			break;
		  case 'S':		/* Save game */
			On_exit = FALSE;
			save();
			break;
		  case 'E':		/* Extrapolate */
#ifdef EXTRAP
			if (last_ex)
				break;
			Finished = TRUE;
			if (WIndow != W_FULL)
				newscore();
			prscore(FALSE);
			wrefresh(Score);
			last_ex = TRUE;
			Finished = FALSE;
#else
			error("%c: command not implemented", c);
#endif
			break;
		  case '\r':		/* Ignore RETURNs and	*/
		  case '\n':		/* Line Feeds		*/
		  case ' ':		/* Spaces		*/
		  case '\0':		/* and nulls		*/
			break;
		  case 'Z':		/* Debug code */
			if (geteuid() == ARNOLD) {
				if (!Debug && outf == NULL) {
					char	buf[40];
over:
					mvaddstr(MOVE_Y, MOVE_X, "file: ");
					clrtoeol();
					leaveok(Board, FALSE);
					refresh();
					sp = buf;
					while ((*sp = readch()) != '\n') {
						if (*sp == _tty.sg_kill)
							goto over;
						else if (*sp == _tty.sg_erase) {
							if (--sp < buf)
								sp = buf;
							else {
								addch('\b');
								if (*sp < ' ')
								    addch('\b');
								clrtoeol();
							}
						}
						else
							addstr(unctrl(*sp++));
						refresh();
					}
					*sp = '\0';
					leaveok(Board, TRUE);
					if ((outf = fopen(buf, "w")) == NULL)
						perror(buf);
					setbuf(outf, 0);
				}
				Debug = !Debug;
				break;
			}
			/* FALLTHROUGH */
		  default:
			error("unknown command: %s", unctrl(c));
			break;
		}
	}
ret:
	leaveok(Board, TRUE);
}


# define	COMP_STRT	20
# define	CARD_STRT	2

prboard() {

	reg PLAY	*pp;
	reg int		i, j, k, temp;

	for (k = 0; k < 2; k++) {
		pp = &Player[k];
		temp = k * COMP_STRT + CARD_STRT;
		for (i = 0; i < NUM_SAFE; i++)
			if (pp->safety[i] == S_PLAYED) {
				mvaddstr(i, temp, C_name[i + S_CONV]);
				if (pp->coups[i])
					mvaddch(i, temp - CARD_STRT, '*');
			}
		mvprintw(14, temp, C_fmt, C_name[pp->battle]);
		mvprintw(16, temp, C_fmt, C_name[pp->speed]);
		for (i = C_25; i <= C_200; ) {
			reg char	*name;
			reg int		end;

			name = C_name[i];
			temp = k * 40;
			end = pp->nummiles[i++];
			for (j = 0; j < end; j++)
				mvwaddstr(Miles, i, (j << 2) + temp, name);
		}
	}
	prscore(TRUE);
	temp = CARD_STRT;
	pp = &Player[PLAYER];
	for (i = 0; i < HAND_SZ; i++)
		mvprintw(i + 6, temp, C_fmt, C_name[pp->hand[i]]);
	mvprintw(6, COMP_STRT + CARD_STRT, "%2d", Topcard - Deck);
	mvprintw(8, COMP_STRT + CARD_STRT, C_fmt, C_name[Discard]);
	if (End == 1000) {
		static char	ext[] = "Extension";

		stand(EXT_Y, EXT_X, ext);
	}
	wrefresh(Board);
	wrefresh(Miles);
	wrefresh(Score);
}

/*
 *	Put str at (y,x) in standout mode
 */
stand(y, x, str)
reg int		y, x;
reg char	*str; {

	standout();
	mvaddstr(y, x, str);
	standend();
	return TRUE;
}

prscore(for_real)
reg bool	for_real; {

	reg PLAY	*pp;
	reg int		x;
	reg char	*Score_fmt = "%4d";

	stdscr = Score;
	for (pp = Player; pp < &Player[2]; pp++) {
		x = (pp - Player) * 6 + 21;
		mvprintw(1, x, Score_fmt, pp->mileage);
		mvprintw(2, x, Score_fmt, pp->safescore);
		if (pp->safescore == 400)
			mvaddstr(3, x + 1, "300");
		else
			mvaddch(3, x + 3, '0');
		mvprintw(4, x, Score_fmt, pp->coupscore);
		if (WIndow == W_FULL || Finished) {
#ifdef EXTRAP
			if (for_real)
				finalscore(pp);
			else
				extrapolate(pp);
#else
			finalscore(pp);
#endif
			mvprintw(11, x, Score_fmt, pp->hand_tot);
			mvprintw(13, x, Score_fmt, pp->total);
			mvprintw(14, x, Score_fmt, pp->games);
		}
		else {
			mvprintw(6, x, Score_fmt, pp->hand_tot);
			mvprintw(8, x, Score_fmt, pp->total);
			mvprintw(9, x, Score_fmt, pp->games);
		}
	}
	stdscr = Board;
}

FlushInput ()
{
	raw();	/* Flush input */
	noraw();

}
