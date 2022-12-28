project('quickjs', 'c')
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
  'quickjs-libc.c',
)
extra_args = [
  '-D_GNU_SOURCE',
  '-DCONFIG_BIGNUM',
  '-DCONFIG_VERSION="2021-03-27"',
  '-w',
]

quickjs = shared_library('quickjs', src,
  c_args : extra_args,
  include_directories : inc,
  dependencies: [m_dep, dl_dep, thread_dep])

quickjs_dep = declare_dependency(
  include_directories : inc,
  link_with : quickjs)

meson.override_dependency('quickjs', quickjs_dep)
