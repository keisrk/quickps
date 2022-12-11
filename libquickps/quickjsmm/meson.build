src = files(
  'context.cpp',
  'esm.cpp',
  'value.cpp',
)

quickjsmm = library('quickjsmm', src,
  include_directories : inc,
  dependencies : quickjs_dep)

quickjsmm_dep = declare_dependency(
  include_directories : inc,
  link_with : quickjsmm,
  dependencies : quickjs_dep)
