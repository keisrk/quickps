fs = import('fs')
py_mod = import('python')

py = py_mod.find_installation(
  required: false,
  disabler: true,
  modules: ['pytest', 'pytest_html']
)

curl = find_program('curl')
cdn = generator(curl,
  output : '@BASENAME@.js',
  arguments : ['--config', '@INPUT@', '--output', '@OUTPUT@'],
)
twgl = cdn.process('src/twgl.config')

esbuild_version = '0.15.18'
esbuild_py = configure_file(
  input: files('scripts/esbuild.py'),
  output: 'esbuild.py',
  configuration: {
    'SYSTEM': build_machine.system(),
    'CPU_FAMILY': build_machine.cpu_family(),
    'FALLBACK_VERSION': esbuild_version,
  }
)

esbuild = find_program(esbuild_py, version: esbuild_version)

sources = files(
  'src/crane.js',
  'src/cube.js',
  'src/logo.js',
)

paperscripts = []

foreach src : sources
  paperscripts += custom_target(
    input: [src, twgl],
    output: fs.name(src),
    command: [esbuild, '@INPUT0@', '--outfile=@OUTPUT@',
      '--depfile=@DEPFILE@',
      '--nodepath=@PRIVATE_DIR@',
      '--bundle', '--format=esm'
    ],
    depfile: fs.stem(src) + '.d'
  )
endforeach

pytest_args = [
  '-m', 'pytest',
  '@0@/tests'.format(meson.current_source_dir()),
  '--html', 'examples.html',
  '--qpsc=@0@'.format(qpsc_exe.path()),
]

foreach js : paperscripts
  pytest_args += '--js'
  pytest_args += js
endforeach

test(
  'integration test',
  py,
  args: pytest_args,
  depends: [qpsc_exe, paperscripts],
  timeout: 240,
)
