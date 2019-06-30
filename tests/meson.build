test_sources = files([
  'main.cpp',
  'test_async.cpp',
  'test_dup_filter.cpp',
  'test_errors.cpp',
  'test_file_helper.cpp',
  'test_file_logging.cpp',
  'test_fmt_helper.cpp',
  'test_macros.cpp',
  'test_misc.cpp',
  'test_mpmc_q.cpp',
  'test_pattern_formatter.cpp',
  'test_registry.cpp',
  'test_stdout_api.cpp',
  'utils.cpp',
])

global_test_deps = []

# -----------------------------------------------------
# ---   Add the systemd test if libsystemd is found ---
# -----------------------------------------------------

systemd_dep = dependency('libsystemd', required: false)

if systemd_dep.found()
  test_sources += files(['test_systemd.cpp'])
  global_test_deps += systemd_dep
endif

# --------------------------------------
# ---   Build the test executables   ---
# --------------------------------------

test_matrix =  [
  ['spdlog-utest',    spdlog_dep],
  ['spdlog-utest-ho', spdlog_headeronly_dep],
]

foreach i : test_matrix
  test_exe = executable(i[0], test_sources, dependencies: global_test_deps + [i[1]])
  test('test_' + i[0], test_exe)
endforeach
