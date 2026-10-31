compile_bc = subproject('quickjs').get_variable('compile_bc')

bundle = find_program('./bundle.sh')

paperjs_esm = custom_target('paperjs_esm',
  input : 'preamble.js',
  output : 'paper.js',
  command : [
    bundle, '@INPUT@', '@OUTPUT@'
  ]
)

paperjs_bc = custom_target('paperjs_bc',
  input : paperjs_esm,
  output : 'paper.bc',
  command : [
    compile_bc, '@INPUT@', '@OUTPUT@'
  ]
)

paperjs = library('paperjs',
  'paper.cpp',
  include_directories : inc,
  cpp_args : [
    '-Wno-c23-extensions',
    '--embed-dir=@0@'.format(meson.current_build_dir()),
  ],
  dependencies : declare_dependency(sources : paperjs_bc))

paperjs_dep = declare_dependency(
  include_directories : inc,
  link_with : paperjs)
