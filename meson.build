project('WindowNavigator2', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

cc = meson.get_compiler('cpp')

executable(
  'windownavigator2',
  ['src/main.cpp', 'src/window_utils.cpp', 'src/gui.cpp', 'src/utils.cpp'],
  include_directories: ['include'],
  dependencies: [dependency('spdlog'), dependency('rapidfuzz')],
  cpp_args: ['/D _CRT_SECURE_NO_WARNINGS'],
  install : true
)