project('paperjs', 'cpp',  version : '0.16.0',
  default_options : ['cpp_std=c++26']
)

curl = find_program('curl')
py_mod = import('python')
py = py_mod.find_installation()
prepro = files('scripts/prepro.py')

quickjs = subproject('quickjs').get_variable('quickjs').get_shared_lib()

cdn = generator(curl,
  output : '@BASENAME@.js',
  arguments : ['--fail', '--proto', '=https', '--tlsv1.2', '--config', '@INPUT@', '--output', '@OUTPUT@'],
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

paperjs_bc = custom_target('paperjs_bc',
  input : [quickjs, paperjs_pp],
  output : 'paper.bc',
  command : [
    py, prepro, 'compile',
    '--dll', '@INPUT0@',
    '--output', '@OUTPUT@',
    '@INPUT1@'
  ]
)

inc = include_directories('src')

paperjs = library('paperjs',
  'src/paper.cpp',
  include_directories : inc,
  cpp_args : [
    '-Wno-c23-extensions',
    '--embed-dir=@0@'.format(meson.current_build_dir()),
  ],
  dependencies : declare_dependency(sources : paperjs_bc))

paperjs_dep = declare_dependency(
  include_directories : inc,
  link_with : paperjs)
