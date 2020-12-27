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

srcs_reversi = [
  'corners.c',
  'count.c',
  'fini.c',
  'game.c',
  'hasmove.c',
  'minmax.c',
  'move.c',
  'Reversi.c',
  'transform.c',
  'xreversi.c',
]

genedge = executable('genedge',
		     'genedge.c',
		     install: false)

bison_gen = generator(bison,
		      output : ['@BASENAME@.c', '@BASENAME@.h'],
		      arguments : [ '-o', '@OUTPUT0@', '--defines=@OUTPUT1@', '-p', 'LayYY', '@INPUT@'])
			  
makeedge_files = bison_gen.process('makeedge.y')

makeedge = executable('makeedge',
		      makeedge_files,
		      install: false)

edges_in = custom_target('edges.in',
			 output : ['edges.in'],
			 capture: true,
			 command : [genedge])

edges_c = custom_target('edges.c',
			input : edges_in,
			output : 'edges.c',
			capture: true,
			command : [makeedge, '@INPUT@'])

executable('kreversi',
	   srcs_reversi, edges_c,
	   include_directories: inc,
	   dependencies: [x_libs],
	   link_with: [lib_xkw],
	   install: true)

install_man('kreversi.6')
install_data('Kreversi.ad',
	     rename: 'Kreversi',
	     install_dir : app_defaults)
