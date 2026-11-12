compile_bc = subproject('quickjs').get_variable('compile_bc')

bundle_full = find_program('./bundle-full.sh')
bundle_core = find_program('./bundle-core.sh')

# --- Flavor 1: Full (PaperScript + Acorn) ---
paperjs_full_esm = custom_target('paperjs_full_esm',
  input : 'preamble-full.js',
  output : 'paper-full.js',
  command : [
    bundle_full, '@INPUT@', '@OUTPUT@'
  ]
)

paperjs_full_bc = custom_target('paperjs_full_bc',
  input : paperjs_full_esm,
  output : 'paper-full.bc',
  command : [
    compile_bc, '@INPUT@', '@OUTPUT@'
  ]
)

paperjs_full = library('paperjs_full',
  'paper-full.cpp',
  include_directories : inc,
  cpp_args : [
    '-Wno-c23-extensions',
    '--embed-dir=@0@'.format(meson.current_build_dir()),
  ],
  dependencies : declare_dependency(sources : paperjs_full_bc))

paperjs_full_dep = declare_dependency(
  include_directories : inc,
  link_with : paperjs_full)

# --- Flavor 2: Core (Clean JS + defineScope, 0 Acorn) ---
paperjs_core_esm = custom_target('paperjs_core_esm',
  input : 'preamble-core.js',
  output : 'paper-core.js',
  depend_files : ['paper-core-entry.js'],
  command : [
    bundle_core, '@INPUT@', '@OUTPUT@'
  ]
)

paperjs_core_bc = custom_target('paperjs_core_bc',
  input : paperjs_core_esm,
  output : 'paper-core.bc',
  command : [
    compile_bc, '@INPUT@', '@OUTPUT@'
  ]
)

paperjs_core = library('paperjs_core',
  'paper-core.cpp',
  include_directories : inc,
  cpp_args : [
    '-Wno-c23-extensions',
    '--embed-dir=@0@'.format(meson.current_build_dir()),
  ],
  dependencies : declare_dependency(sources : paperjs_core_bc))

paperjs_core_dep = declare_dependency(
  include_directories : inc,
  link_with : paperjs_core)

# Compatibility alias
paperjs_dep = paperjs_full_dep
