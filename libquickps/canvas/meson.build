src = files(
    'color.cpp',
    'context.cpp',
    'facade.cpp',
    'font.cpp',
    'state.cpp',
    'style.cpp',
)

canvas = library(
    'canvas',
    src,
    include_directories: inc,
    cpp_args: [
        '-Wno-c23-extensions',
        '--embed-dir=@0@'.format(
            meson.project_source_root()
            / 'subprojects/blend2d/blend2d-testing/resources',
        ),
    ],
    dependencies: [blend2d_dep, quickjsmm_dep],
)

canvas_dep = declare_dependency(
    include_directories: inc,
    link_with: [canvas],
    dependencies: [blend2d_dep, quickjsmm_dep],
)
