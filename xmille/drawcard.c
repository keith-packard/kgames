/*
 * drawcard.c
 *
 * display cards on the table
 */

# include	"mille.h"
# include	"uiXt.h"
# include	"card.h"
# include	"cards-svg.h"

static void
init_card (struct card *card)
{
	card->rsvg_handle = XkwRsvgCreate(card->svg);
}

static void
init_cards (void)
{
	int		i;
	init_card (&deck);
	init_card (&blank);

	for (i = 0; i < (NUM_CARDS - 1); i++) {
		init_card (&svg_cards[i]);
	}
}

void
init_color_cards (void)
{
	init_cards();
}

void
init_mono_cards(void)
{
	init_cards();
}
