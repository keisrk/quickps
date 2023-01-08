subdir('quickjsmm')
subdir('canvas')

src = files('quickps.cpp')

quickps = library('quickps', src,
  dependencies : [canvas_dep, paperjs_dep, quickjsmm_dep])

quickps_dep = declare_dependency(
  include_directories : inc,
  link_with : quickps,
  dependencies : [canvas_dep, paperjs_dep, quickjsmm_dep])

tests = files(
  'cairomm.test.cpp',
  'canvas/color.test.cpp',
  'canvas/context.test.cpp',
  'e2e.test.cpp',
  'isinstanceof.test.cpp',
  'paperjs.test.cpp',
  'quickjsmm/context.test.cpp',
  'quickjsmm/esm.test.cpp',
  'quickjsmm/facade.cpp',
  'quickjsmm/fixture.cpp',
  'quickjsmm/value.test.cpp',
  'quickps.test.cpp',
)

unit_test = executable('unit_test', tests,
  include_directories : inc,
  dependencies : [gtest_dep, quickps_dep])

test('unit test', unit_test,
  protocol : 'gtest',
  workdir : meson.current_build_dir())
