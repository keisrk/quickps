qpsc_link_args = []
if get_option('b_sanitize') == 'none'
    qpsc_link_args += ['-static']
endif

qpsc_exe = executable(
    'qpsc',
    'qpsc.cpp',
    dependencies: [quickps_dep],
    link_args: qpsc_link_args,
    install: false,
)
