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
srcs_kcribbage = [
  'cards.c',
  'CribBoard.c',
  'crib.c',
  'extern.c',
  'io.c',
  'score.c',
  'support.c',
  'xt.c',
]

ad_file = configure_file(input: 'Cribbage.ad.in',
                         output: '@BASENAME@',
                         configuration: conf_data,
                         install: false)

res_files = res_gen.process(ad_file)

executable('kcribbage',
	   srcs_kcribbage, res_files,
	   include_directories: inc,
	   dependencies: x_libs,
	   link_with: [lib_xkw],
	   install: true)
install_man('kcribbage.6')

configure_file(input: 'kcribbage.desktop.in',
	       output: '@BASENAME@',
	       configuration: conf_data,
	       install_dir : desktop_dir)
