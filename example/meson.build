example_matrix =  [
  ['spdlog-example',    spdlog_dep],
  ['spdlog-example-ho', spdlog_headeronly_dep],
]

foreach i : example_matrix
  test_exe = executable(i[0], ['example.cpp'], dependencies: i[1])
endforeach
