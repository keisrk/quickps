src = files('rendering_context_2d.cpp')

quickjsmm = library('canvas', src,
  include_directories : inc,
  dependencies : [cairo_dep, quickjs_dep],
  link_with : [quickjsmm])
