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
	version: '2.1',
	default_options: ['bindir=games', 'datadir=share', 'warning_level=3'])

c_compiler = meson.get_compiler('c')

lib_m = c_compiler.find_library('m')
lib_x11 = dependency('x11')
lib_xmu = dependency('xmu')
lib_xpm = dependency('xpm')
lib_xt = dependency('xt')
lib_xaw = dependency('xaw7')
lib_render = dependency('xrender')
lib_freetype = dependency('freetype2')
lib_fontconfig = dependency('fontconfig')
lib_cairo = dependency('cairo')
lib_cairo_ft = dependency('cairo-ft')
lib_xft = dependency('xft')
lib_rsvg2 = dependency('librsvg-2.0')

x_libs = [lib_xft, lib_cairo_ft, lib_rsvg2, lib_cairo, lib_fontconfig,
	  lib_freetype, lib_render, lib_xaw, lib_xt, lib_xmu,
	  lib_xpm, lib_x11, lib_m]

prefix = get_option('prefix')
datadir = get_option('datadir')
bindir = get_option('bindir')

svg_icon_dir = datadir / 'icons/hicolor/scalable/apps'

desktop_dir = datadir / 'applications'

menudir = get_option('menudir')
if menudir == ''
  if get_option('user-menu')
    fs = import('fs')
    xdg_config_dir = fs.expanduser('~/.config')
    message('user-menu config_dir is ' + xdg_config_dir)
  else
    xdg_config_dir= sysconfdir / 'xdg'
  endif
  menudir = xdg_config_dir / 'menus/applications-merged'
endif

message('menudir is ' + menudir)

conf_data = configuration_data()
conf_data.set('KGAMES_VERSION', meson.project_version(),
	      description: 'KGames version in unquoted string form.')
conf_data.set('KGAMES_VERSION_STRING',
	      '"@0@"'.format(meson.project_version()),
	      description: 'KGames version in quoted string form.')
conf_data.set('BINDIR', prefix / bindir)
conf_data.set('ICONDIR', prefix / svg_icon_dir)
conf_data.set('MENUDIR', menudir)

inc_dirs = ['.']

inc = include_directories(inc_dirs)

c_warnings = ['-Werror=implicit-function-declaration',
	      '-Werror=vla',
	      '-Warray-bounds',
	      '-Wold-style-definition',
	      '-Wno-overlength-strings',
	     ]

add_project_arguments(c_compiler.get_supported_arguments(c_warnings), language : 'c')

configure_file(output : 'kgames.h',
	       configuration: conf_data,
	       install: false)

configure_file(input: 'kgames.directory.in',
	       output: '@BASENAME@',
	       configuration: conf_data,
	       install_dir : datadir / 'desktop-directories'
	       )

configure_file(input: 'kgames.menu.in',
	       output: '@BASENAME@',
	       configuration: conf_data,
	       install_dir : menudir
	       )

subdir('Xkw')
subdir('kaces')
subdir('kcanfield')
subdir('kcribbage')
subdir('kdominos')
subdir('kklondike')
subdir('kmcarlo')
subdir('kmontana')
subdir('kslyfox')
subdir('kspider')
subdir('ktabby')
subdir('kthieves')
subdir('ktowers')
subdir('xmille')
subdir('xreversi')

meson.add_install_script('update-menus.sh')
