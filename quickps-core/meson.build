qps_so_link_args = []
if get_option('b_sanitize') == 'none'
    qps_so_link_args += ['-static-libstdc++', '-static-libgcc']
endif

# Native QuickJS C-Module (qps.so) for stock qjs interpreter
qps_so = shared_module(
    'qps',
    'module.cpp',
    name_prefix: '',
    cpp_args: ['-Wno-c99-extensions', '-Wno-unused-parameter'],
    link_args: qps_so_link_args,
    dependencies: [canvas_dep, quickjsmm_dep, paperjs_core_dep, quickjs_dep],
    install: true,
)

qjs = find_program('qjs')

test_script = configure_file(
    input: 'test_module.js',
    output: 'test_module.js',
    copy: true,
)

test(
    'qps native module test',
    qjs,
    args: ['-m', test_script],
    depends: [qps_so],
    workdir: meson.current_build_dir(),
)
