python_sources = [files('__init__.py')]
fs = import('fs')
fs.copyfile('__init__.py', '__init__.py')

pymod = import('python')
python3 = pymod.find_installation('python3')

pybind11_dep = dependency('pybind11')

if meson.get_compiler('cpp').get_id() == 'msvc'
    pylpp_boards_pp_cpp_args = ['/bigobj']
else
    pylpp_boards_pp_cpp_args = []
endif


_pylpp_boards_pp = python3.extension_module('_pylpp_boards_pp', 'pylpp_boards_pp.cpp',
                         dependencies: [lpp_boards_pp_dep, pybind11_dep,python3.dependency()],
                         link_args: link_args,
                         cpp_args: pylpp_boards_pp_cpp_args,
                         subdir: 'pylpp_boards_pp',
                         install: true,
                         extra_files: []
                        )

python3.install_sources(python_sources, pure: false, subdir: 'pylpp_boards_pp')
