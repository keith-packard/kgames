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
srcs_dominos = [
  'computer.c',
  'connect.c',
  'domino.c',
  'Dominos.c',
  'file.c',
  'find.c',
  'game.c',
  'util.c',
]

ad_file = configure_file(input: 'Dominos.ad.in',
                         output: '@BASENAME@',
                         configuration: conf_data,
                         install: false)

res_files = res_gen.process(ad_file)

make_dominos_svg = find_program('make-dominos-svg')

dominos_files = custom_target('svg-dominos',
			      output : ['dominos-svg.c', 'dominos-svg.h'],
			      command : [make_dominos_svg, '@OUTPUT0@', '@OUTPUT1@'])

install_data('kdominos.svg', install_dir : svg_icon_dir)

executable('kdominos',
	   srcs_dominos, res_files, dominos_files,
	   include_directories: inc,
	   dependencies: x_libs,
	   link_with: [lib_xkw],
	   install: true)

install_man('kdominos.6')

configure_file(input: 'kdominos.desktop.in',
	       output: '@BASENAME@',
	       configuration: conf_data,
	       install_dir : desktop_dir)
