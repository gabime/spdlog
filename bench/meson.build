benchmark = dependency('benchmark')

bench_matrix = [
  ['bench',           [spdlog_dep],            []],
  ['async_bench',     [spdlog_dep],            []],
  ['formatter-bench', [spdlog_dep, benchmark], ['all']],
  ['latency',         [spdlog_dep, benchmark], []],
]

foreach i : bench_matrix
  bench_exe = executable(i[0], i[0] + '.cpp', dependencies: i[1])
  benchmark('bench_' + i[0], bench_exe, args: i[2])
endforeach

