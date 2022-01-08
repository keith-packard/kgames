# define	DEBUG

/*
 * @(#)table.c	1.1 (Berkeley) 4/1/82
 */

# include	"mille.h"

void
main(void)
{
	printf("   %16s -> %5s %5s %4s %s\n", "Card", "cards", "count", "need", "opposite");
	for (int i = 0; i < NUM_CARDS - 1; i++) {
		int count = 0;
		for (int j = 0; j < DECK_SZ; j++)
			if (Deck[j] == i)
				count++;
		printf("%2d %16s -> %5d %5d %4d %s\n", i, C_name[i], Numcards[i], count, Numneed[i], C_name[opposite(i)]);
	}
}
