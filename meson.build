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
project('kgames', 'c',
	license : 'MIT',
	version: '1.0')

c_compiler = meson.get_compiler('c')

lib_m = c_compiler.find_library('m')
lib_x11 = c_compiler.find_library('X11')
lib_xmu = c_compiler.find_library('Xmu')
lib_xpm = c_compiler.find_library('Xpm')
lib_xt = c_compiler.find_library('Xt')
lib_xaw = c_compiler.find_library('Xaw')

x_libs = [lib_xaw, lib_xt, lib_xmu, lib_xpm, lib_x11, lib_m]

prefix = get_option('prefix')
datadir = get_option('datadir')

app_defaults = datadir / 'app-defaults'

add_global_arguments('-D', 'APPDEFAULTS="' + prefix / app_defaults + '"', language: 'c')

inc_dirs = ['.']

inc = include_directories(inc_dirs)

subdir('Xkw')
subdir('dominos')
subdir('kaces')
subdir('kcanfield')
subdir('kcribbage')
subdir('kklondike')
subdir('kmontana')
subdir('kspider')
subdir('ktabby')
subdir('kthieves')
subdir('ktowers')
subdir('reversi')
subdir('xmille')
