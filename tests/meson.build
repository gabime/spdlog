test_sources = files([
    'test_file_helper.cpp',
    'test_file_logging.cpp',
    'test_daily_logger.cpp',
    'test_misc.cpp',
    'test_pattern_formatter.cpp',
    'test_async.cpp',
    'includes.h',
    'test_registry.cpp',
    'test_macros.cpp',
    'utils.cpp',
    'main.cpp',
    'test_mpmc_q.cpp',
    'test_fmt_helper.cpp',
    'test_stdout_api.cpp',
    'test_dup_filter.cpp',
    'test_backtrace.cpp'
])


if  not get_option('no_exceptions')
    test_sources += 'test_errors.cpp'
endif

global_test_deps = []

# -----------------------------------------------------
# ---   Add the systemd test if libsystemd is found ---
# -----------------------------------------------------

systemd_dep = dependency('libsystemd', required: false)

if systemd_dep.found()
  test_sources += 'test_systemd.cpp'
  global_test_deps += systemd_dep
endif

run_command('mkdir', 'logs')
# --------------------------------------
# ---   Build the test executables   ---
# --------------------------------------

test_matrix =  [
  ['spdlog-utests',    spdlog_dep],
  ['spdlog-utests-ho', spdlog_headeronly_dep],
]

foreach i : test_matrix
  test_exe = executable(i[0], test_sources, dependencies: global_test_deps + [i[1]])
  test('test_' + i[0], test_exe)
endforeach

run_command(find_program('mkdir'), meson.current_build_dir() + '/logs')