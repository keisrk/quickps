project(
    'quickps',
    'cpp',
    version: '0.1.0',
    default_options: [
        'warning_level=3',
        'cpp_std=c++26',
        'python.install_env=venv',
        'default_library=static',
    ],
)

add_project_link_arguments(['-static-libstdc++', '-static-libgcc', '-lstdc++fs'], language: 'cpp')

blend2d_dep = dependency('blend2d')
gtest_dep = dependency('gtest_main')
paperjs_dep = dependency('paperjs')
quickjs_dep = dependency('quickjs')

inc = include_directories('.')
subdir('libquickps')
subdir('quickps')
subdir('examples')
