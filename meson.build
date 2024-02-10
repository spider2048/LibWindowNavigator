project('WindowNavigator2', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

cc = meson.get_compiler('cpp')

shared_library(
  'WindowNavigator',
  ['src/window_utils.cpp', 'src/gui.cpp', 'src/utils.cpp', 'src/search.cpp'],
  include_directories: ['include'],
  dependencies: [dependency('spdlog'), dependency('rapidfuzz'), dependency('sdl2'), dependency('sdl2_ttf')],
  cpp_args: ['/D _CRT_SECURE_NO_WARNINGS'],
)