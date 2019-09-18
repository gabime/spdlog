project('spdlog', ['cpp'],
  license         : 'MIT',
  version         : run_command(find_program('scripts/extract_version.py')).stdout().strip(),
  default_options : [
    'warning_level=3',
    'cpp_std=c++11',
    'buildtype=release',
    'b_colorout=always',
  ],
)

# ------------------------
# ---   Dependencies   ---
# ------------------------
dep_list = []
compile_args = []

# Threads
dep_list += dependency('threads')

# Check for FMT
if get_option('external_fmt')
  if not meson.version().version_compare('>=0.49.0')
    warning('Finding fmt can fail wit meson versions before 0.49.0')
  endif
  dep_list     += dependency('fmt')
  compile_args += '-DSPDLOG_FMT_EXTERNAL'
endif

if get_option('no_exceptions')
  compile_args += '-DSPDLOG_NO_EXCEPTIONS'
endif

# ------------------------------------
# ---   Compiled library version   ---
# ------------------------------------

spdlog_inc = include_directories('./include')

spdlog_srcs = files([
    'src/async.cpp',
    'src/color_sinks.cpp',
    'src/file_sinks.cpp',
    'src/fmt.cpp',
    'src/spdlog.cpp',
    'src/stdout_sinks.cpp'
])

if get_option('library_type') == 'static'
  spdlog = static_library(
    'spdlog',
    spdlog_srcs,
    cpp_args            : [compile_args] + ['-DSPDLOG_COMPILED_LIB'],
    include_directories : spdlog_inc,
    dependencies        : dep_list,
    install             : not meson.is_subproject()
    )
else
  spdlog = shared_library('spdlog',
    spdlog_srcs,
    cpp_args            : [compile_args] + ['-DSPDLOG_COMPILED_LIB'],
    include_directories : spdlog_inc,
    dependencies        : dep_list,
    install             : not meson.is_subproject(),
  )
endif

spdlog_dep = declare_dependency(
  link_with           : spdlog,
  include_directories : spdlog_inc,
  compile_args        : compile_args + ['-DSPDLOG_COMPILED_LIB'],
  dependencies        : dep_list,
  version             : meson.project_version(),
)

# ----------------------------------
# ---   Header only dependency   ---
# ----------------------------------

spdlog_headeronly_dep = declare_dependency(
  include_directories : spdlog_inc,
  compile_args        : compile_args,
  dependencies        : dep_list,
  version             : meson.project_version(),
)

# ------------------------
# ---   Installation   ---
# ------------------------

# Do not install when spdlog is used as a subproject
if not meson.is_subproject()
  install_subdir('include/spdlog', install_dir: get_option('includedir'))

  pkg = import('pkgconfig')
  pkg.generate(spdlog,
    name         : 'spdlog',
    description  : 'Fast C++ logging library',
    url          : 'https://github.com/gabime/spdlog',
    extra_cflags : ['-DSPDLOG_COMPILED_LIB']
  )
endif

# -------------------------------------
# ---   Conditionally add subdirs   ---
# -------------------------------------

if get_option('enable_tests')
  subdir('tests')
endif

if get_option('enable_examples')
  subdir('example')
endif

if get_option('enable_benchmarks')
  subdir('bench')
endif

# -------------------
# ---   Summary   ---
# -------------------

summary_str = '''spdlog build summary:

  - using external fmt:  @0@
  - building tests:      @1@
  - building examples:   @2@
  - building benchmarks: @3@
  - library type:        @4@
  - no exceptions:       @5@
'''.format(
  get_option('external_fmt'),
  get_option('enable_tests'),
  get_option('enable_examples'),
  get_option('enable_benchmarks'),
  get_option('library_type'),
  get_option('no_exceptions')
)

message(summary_str)
