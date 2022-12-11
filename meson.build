project('quickps', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3',
                     'cpp_std=c++17'])

subproject('quickjs')
subproject('paperjs')

boost_dep = dependency('boost', modules : ['program_options'])
cairo_dep = dependency('cairomm-1.0')
gtest_dep = dependency('gtest', main : true)
paperjs_dep = dependency('paperjs')
quickjs_dep = dependency('quickjs')

inc = include_directories('.')
subdir('libquickps')
subdir('quickps')
