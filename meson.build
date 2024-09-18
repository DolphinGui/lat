project(
  'lat',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_LAT']

shlib = library(
  'lat',
  'src/lat.cpp',
  install: true,
  cpp_args: lib_args,
  gnu_symbol_visibility: 'hidden',
  include_directories: include_directories('include'),
)

lat_dep = declare_dependency(include_directories: include_directories('include'), link_with: shlib)

test_exe = executable('lat_test', 'tests/lat_test.cpp', dependencies: lat_dep)
test('lat', test_exe)


# Make this library usable from the system's
# package manager.
install_headers('include/lat.hpp', subdir: 'lat')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name: 'lat',
  filebase: 'lat',
  description: 'Meson sample project.',
  subdirs: 'lat',
  libraries: shlib,
  version: '0.1',
)