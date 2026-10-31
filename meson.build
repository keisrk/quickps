project(
    'quickps',
    'cpp',
    version: '0.1.0',
    default_options: [
        'warning_level=3',
        'cpp_std=c++26',
        'python.install_env=venv',
        'default_library=static',
        'cpp_args=-stdlib=libc++',
        'cpp_link_args=-stdlib=libc++',
        'force_fallback_for=gtest',
    ],
)

add_project_arguments(['-stdlib=libc++'], language: 'cpp')
add_project_link_arguments(['-stdlib=libc++', '-static'], language: 'cpp')

blend2d_dep = dependency('blend2d')
gtest_dep = dependency('gtest_main', fallback: ['gtest', 'gtest_main_dep'])
quickjs_dep = dependency('quickjs')

inc = include_directories('.')
subdir('paperjs')
subdir('libquickps')
subdir('quickps')
subdir('examples')
