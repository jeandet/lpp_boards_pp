project(
  'lpp_boards_pp',
  'cpp','c',
  meson_version: '>= 1.2.0',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++23', 'default_library=static', 'buildtype=release'],
  license : 'GPL3'
)

if build_machine.system() == 'windows'
    link_args = ['-static-libstdc++','-static-libgcc','-static']
elif build_machine.system() == 'darwin' and meson.get_compiler('cpp').get_id() != 'clang'
    link_args = ['-static-libstdc++','-static-libgcc']
else
    link_args = []
endif

header_files = files(
                    'include/auto_recycled_channel.hpp',
                    'include/concepts.hpp',
                    'include/simple_protocol.hpp',
                    'include/ftdi.hpp',
                    'include/pcb_lob.hpp',
                    'include/array2d.hpp'
                    )

source_files = files('src/ftdi.cpp')

libftdi = dependency('libftdi')
channels = dependency('channels', required : true)
cpp_utils = dependency('cpp_utils', required : true)
fmt = dependency('fmt', required : true)

lpp_boards_pp_dep = declare_dependency(
    include_directories : include_directories('include'),
    sources: source_files,
    link_args: link_args,
    dependencies : [libftdi, channels, cpp_utils, fmt],
    extra_files: [header_files, '.github/workflows/CI.yml', 'pyproject.toml', 'README.md'],
    )
subdir('tests')


if get_option('disable_python_wrapper')
     message('building without Python wrapper')
 else
     subdir('pylpp_boards_pp')
endif



summary({'C': meson.get_compiler('c').cmd_array(),
        'C++': meson.get_compiler('cpp').cmd_array()
        }, section: 'Compilers')

