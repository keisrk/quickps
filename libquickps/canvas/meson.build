src = files('color.cpp', 'context.cpp', 'facade.cpp', 'state.cpp', 'style.cpp')

canvas = library(
    'canvas',
    src,
    include_directories: inc,
    dependencies: [blend2d_dep, quickjsmm_dep],
)

canvas_dep = declare_dependency(
    include_directories: inc,
    link_with: [canvas],
    dependencies: [blend2d_dep, quickjsmm_dep],
)
