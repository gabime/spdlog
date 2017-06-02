prebuilt_cxx_library(
  name = 'spdlog',
  header_namespace = 'spdlog',
  header_only = True,
  exported_headers = subdir_glob([
    ('include/spdlog', '**/*.h'),
    ('include/spdlog', '**/*.cc'),
  ]),
  visibility = [
    'PUBLIC',
  ],
)
