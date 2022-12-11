project('paperjs', 'cpp')

curl = find_program('curl')
py_mod = import('python')
py = py_mod.find_installation()
prepro = files('scripts/prepro.py')

quickjs = subproject('quickjs').get_variable('quickjs')

cdn = generator(curl,
  output : '@BASENAME@.js',
  arguments : ['--config', '@INPUT@', '--output', '@OUTPUT@'],
)

acornjs = cdn.process(
  'node_modules/acorn/acorn.config',
  preserve_path_from : meson.current_source_dir(),
)

strapsjs = cdn.process(
  'node_modules/straps/straps.config',
  preserve_path_from : meson.current_source_dir(),
)

nodomjs = cdn.process(
  'node_modules/nodom/nodom.config',
  preserve_path_from : meson.current_source_dir(),
)

src = files(
  'src/constants.js',
  'src/options.js',
  'src/paper.q.js',
)

paperjs_pp = custom_target('paperjs_pp',
  input : [quickjs, acornjs, strapsjs, nodomjs, src],
  output : 'paper.js',
  command : [
    py, prepro, 'bundle',
    '--dll', '@INPUT0@',
    '--node-module', '@INPUT1@',
    '--node-module', '@INPUT2@',
    '--node-module', '@INPUT3@',
    '--evaluate', '@INPUT4@',
    '--evaluate', '@INPUT5@',
    '@INPUT6@'
  ],
  capture : true,
)

paperjs_c = custom_target('paperjs_c',
  input : [quickjs, paperjs_pp],
  output : ['paper.cpp', 'paper.h'],
  command : [
    py, prepro, 'compile',
    '--dll', '@INPUT0@',
    '--ns', 'paper',
    '@INPUT1@'
  ]
)

paperjs = library('paperjs',
  paperjs_c)

paperjs_dep = declare_dependency(
  sources : paperjs_c[1],
  link_with : paperjs)

meson.override_dependency('paperjs', paperjs_dep)
