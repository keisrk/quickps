project('quickjs', 'c',  version : '2021-03-27')
inc = include_directories('.')
cc = meson.get_compiler('c')
m_dep = cc.find_library('m')
dl_dep = dependency('dl')
thread_dep = dependency('threads')
src = files(
  'cutils.c',
  'libbf.c',
  'libunicode.c',
  'libregexp.c',
  'quickjs.c',
)
extra_args = [
  '-D_GNU_SOURCE',
  '-DCONFIG_BIGNUM',
  '-DCONFIG_VERSION="@0@"'.format(meson.project_version()),
  '-w',
]

quickjs = both_libraries('quickjs', src,
  c_args : extra_args,
  dependencies: [m_dep, dl_dep, thread_dep])

quickjs_dep = declare_dependency(
  include_directories : inc,
  link_with : quickjs.get_static_lib())
