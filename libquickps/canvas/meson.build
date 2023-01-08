src = files('color.cpp', 'context.cpp', 'facade.cpp', 'style.cpp')

canvas = library('canvas', src,
  include_directories : inc,
  dependencies : [cairo_dep, quickjsmm_dep])

canvas_dep = declare_dependency(
  include_directories : inc,
  link_with : [canvas],
  dependencies : [cairo_dep, quickjsmm_dep])
