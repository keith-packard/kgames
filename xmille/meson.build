#
# Copyright © 2020 Keith Packard
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting documentation, and
# that the name of the copyright holders not be used in advertising or
# publicity pertaining to distribution of the software without specific,
# written prior permission.  The copyright holders make no representations
# about the suitability of this software for any purpose.  It is provided "as
# is" without express or implied warranty.
#
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.
#
res_files = res_gen.process('Mille.ad')

srcs_xmille = [
  'animate.c',
  'comp.c',
  'drawcard.c',
  'end.c',
  'extern.c',
  'init.c',
  'mille.c',
  'misc.c',
  'move.c',
  'print.c',
  'roll.c',
  'save.c',
  'types.c',
  'uiXt.c',
  'varpush.c',
]

subdir('cards')

make_cards_svg = find_program('make-cards-svg')

cards_files = custom_target('xmille-svg-cards',
			    depend_files : cards_xmille + ['card-names'],
			    output : ['cards-svg.c', 'cards-svg.h'],
			    command : [make_cards_svg, '@OUTPUT0@', '@OUTPUT1@'])

executable('xmille',
	   srcs_xmille, cards_files, res_files,
	   include_directories: inc,
	   dependencies: x_libs,
	   link_with: [lib_xkw],
	   install: true)
install_man('xmille.6')
