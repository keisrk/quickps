subdir('quickjsmm')
subdir('canvas')

src = files()

quickps = library('quickps', src,
  dependencies : [paperjs_dep, quickjs_dep],
  link_with : [quickjsmm])

tests = files(
  'quickjsmm/context.test.cpp',
  'quickjsmm/esm.test.cpp',
  'quickjsmm/fixture.cpp',
  'quickjsmm/value.test.cpp',
  'paperjs.test.cpp',
)

unit_test = executable('unit_test', tests,
  include_directories : inc,
  dependencies : [gtest_dep, paperjs_dep, quickjs_dep, quickjsmm_dep],
)

test('unit test', unit_test,
  protocol : 'gtest',
  workdir : meson.current_build_dir(),
)
