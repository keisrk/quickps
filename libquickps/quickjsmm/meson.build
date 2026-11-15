src = files(
    'context.cpp',
    'esm.cpp',
    'value.cpp',
)

quickjs_headers_dep = quickjs_dep.partial_dependency(
    includes: true,
    compile_args: true,
)

quickjsmm = library(
    'quickjsmm',
    src,
    include_directories: inc,
    dependencies: quickjs_headers_dep,
)

quickjsmm_dep = declare_dependency(
    include_directories: inc,
    link_with: quickjsmm,
    dependencies: quickjs_headers_dep,
)
