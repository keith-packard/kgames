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

flex = find_program('flex')
bison = find_program('bison')

make_resource = find_program('make-resource')

res_gen = generator(make_resource,
		    output : ['@BASENAME@-res.h'],
		    arguments : [ '@INPUT@', '@OUTPUT@' ]
		   )

flex_gen = generator(flex,
		     output : ['@BASENAME@.c'],
		     arguments : [ '-o', '@OUTPUT@', '-P', 'LayYY', '@INPUT@'])

flex_files = flex_gen.process('laylex.l')

bison_gen = generator(bison,
		      output : ['@BASENAME@.c', '@BASENAME@.h'],
		      arguments : [ '-o', '@OUTPUT0@', '--defines=@OUTPUT1@', '-p', 'LayYY', '@INPUT@'])
			  
bison_files = bison_gen.process('laygram.y')

make_cards_svg = find_program('make-cards-svg')

cards_files = custom_target('svg-cards',
			    output : ['cards-svg.c', 'cards-svg.h'],
			    command : [make_cards_svg, '@OUTPUT0@', '@OUTPUT1@'])

srcs_xkw = [
  'Animate.c',
  'Cards.c',
  'CardsUtil.c',
  'Dialog.c',
  'Draw.c',
  'Events.c',
  'Hand.c',
  'Layout.c',
  'medium.c',
  'Message.c',
  'Pad.c',
  'padtest.c',
  'SuitCards.c',
  'Thermo.c',
  'Svg.c',
  'KCommand.c',
  'KLabel.c',
  'KMenuButton.c',
  'KPorthole.c',
  'KScrollbar.c',
  'KSimple.c',
  'KSimpleMenu.c',
  'KSme.c',
  'KSmeBSB.c',
  'KSmeLine.c',
  'KTextLine.c',
  'KToggle.c',
  'XkwInit.c',
]

lib_xkw = library('Xkw',
		  srcs_xkw, flex_files, bison_files, cards_files,
		  dependencies: x_libs,
		  install: true,
		  version: '2.0.0',
		  include_directories: inc)

executable('layout',
	   'layout.c',
	   dependencies: x_libs,
	   include_directories: inc,
	   link_with: [lib_xkw],
	   install: false)

executable('padtest',
	   'padtest.c',
	   dependencies: x_libs,
	   include_directories: inc,
	   link_with: [lib_xkw],
	   install: false)

#install_data('PadTest.ad',             rename: 'PadTest',              install_dir : app_defaults)
