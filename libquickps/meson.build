subdir('quickjsmm')
subdir('canvas')

src = files()

quickps = library('quickps', src,
  dependencies : [paperjs_dep, quickjsmm_dep])

tests = files(
  'canvas/rendering_context_2d.test.cpp',
  'quickjsmm/context.test.cpp',
  'quickjsmm/esm.test.cpp',
  'quickjsmm/fixture.cpp',
  'quickjsmm/value.test.cpp',
  'paperjs.test.cpp',
)

unit_test = executable('unit_test', tests,
  include_directories : inc,
  dependencies : [canvas_dep, gtest_dep, paperjs_dep, quickjsmm_dep])

test('unit test', unit_test,
  protocol : 'gtest',
  workdir : meson.current_build_dir())
