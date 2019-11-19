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
    warning('Finding fmt can fail with meson versions before 0.49.0')
    endif
    dep_list     += dependency('fmt')
    compile_args += '-DSPDLOG_FMT_EXTERNAL'
endif

if get_option('no_exceptions')
  compile_args += '-DSPDLOG_NO_EXCEPTIONS'
endif

if get_option('wchar_support')
    if build_machine.system() != 'windows'
        error('wchar_support only supported under windows')
    endif
    compile_args += '-DSPDLOG_WCHAR_TO_UTF8_SUPPORT'
endif

if get_option('wchar_filenames')
    if build_machine.system() != 'windows'
        error('wchar_filenames only supported under windows')
    endif
      compile_args += '-DSPDLOG_WCHAR_FILENAMES'
endif

if get_option('clock_coarse')
    if build_machine.system() != 'linux'
        error('clock_coarse only supported under linux')
    endif
    compile_args += '-DSPDLOG_CLOCK_COARSE'
endif

if get_option('prevent_child_fd')
    compile_args += '-DSPDLOG_PREVENT_CHILD_FD'
endif

if get_option('no_thread_id')
    compile_args += '-DSPDLOG_NO_THREAD_ID'
endif

if get_option('no_tls')
    compile_args += '-DSPDLOG_NO_TLS'
endif

if get_option('no_atomic_levels')
    compile_args += '-DSPDLOG_NO_ATOMIC_LEVELS'
endif

compile_args_compiled = compile_args + ['-DSPDLOG_COMPILED_LIB']
compile_args_ho = compile_args

# ------------------------------------
# ---   Compiled library version   ---
# ------------------------------------

spdlog_inc = include_directories('./include')

spdlog_srcs = files([
    'src/async.cpp',
    'src/color_sinks.cpp',
    'src/file_sinks.cpp',
    'src/spdlog.cpp',
    'src/stdout_sinks.cpp'
])

if not get_option('external_fmt')
    spdlog_srcs+= 'src/fmt.cpp'
endif

if get_option('library_type') == 'static'
    spdlog = static_library(
    'spdlog',
    spdlog_srcs,
    cpp_args            : compile_args_compiled,
    include_directories : spdlog_inc,
    dependencies        : dep_list,
    install             : not meson.is_subproject()
    )
else
    spdlog = shared_library('spdlog',
    spdlog_srcs,
    cpp_args            : compile_args_compiled,
    include_directories : spdlog_inc,
    dependencies        : dep_list,
    install             : not meson.is_subproject(),
    )
endif

spdlog_dep = declare_dependency(
      link_with           : spdlog,
      include_directories : spdlog_inc,
      compile_args        : compile_args_compiled,
      dependencies        : dep_list,
      version             : meson.project_version(),
)

# ----------------------------------
# ---   Header only dependency   ---
# ----------------------------------
spdlog_headeronly_dep = declare_dependency(
    include_directories : spdlog_inc,
    compile_args        : compile_args_ho,
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
    extra_cflags : compile_args_compiled
    )
endif

# -------------------------------------
# ---   Conditionally add subdirs   ---
# -------------------------------------

if get_option('enable_tests') or get_option('enable_tests-ho')
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
