project(
  'lpp_boards_pp',
  'cpp','c',
  meson_version: '>= 1.2.0',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++23', 'default_library=static', 'buildtype=release'],
  license : 'GPL3'
)

header_files = files('include/simple_protocol.hpp', 'include/ftdi.hpp')
source_files = files('src/ftdi.cpp')

libftdi = meson.get_compiler('cpp').find_library('ftdi1')

lpp_boards_pp_dep = declare_dependency(
    include_directories : include_directories('include'),
    sources: source_files,
    dependencies : libftdi,
    extra_files: header_files,
    )
subdir('tests')


summary({'C': meson.get_compiler('c').cmd_array(),
        'C++': meson.get_compiler('cpp').cmd_array()
        }, section: 'Compilers')

