test_sources = files([
    'test_file_helper.cpp',
    'test_file_logging.cpp',
    'test_daily_logger.cpp',
    'test_misc.cpp',
    'test_pattern_formatter.cpp',
    'test_async.cpp',
    'test_registry.cpp',
    'test_macros.cpp',
    'utils.cpp',
    'main.cpp',
    'test_mpmc_q.cpp',
    'test_dup_filter.cpp',
    'test_fmt_helper.cpp',
    'test_stdout_api.cpp',
    'test_backtrace.cpp',
    'test_create_dir.cpp',
    'test_cfg.cpp',
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

# --------------------------------------
# ---   Build the test executables   ---
# --------------------------------------
if get_option('enable_tests')
    test_exe = executable('spdlog-utests', test_sources, dependencies: global_test_deps + [spdlog_dep])
    test('test_spdlog', test_exe, is_parallel : false)
endif

if get_option('enable_tests_ho')
    test_exe = executable('spdlog-utests-ho', test_sources, dependencies: global_test_deps + [spdlog_headeronly_dep])
    test('test_spdlog-ho', test_exe, is_parallel : false)
endif