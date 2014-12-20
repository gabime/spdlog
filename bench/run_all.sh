#~/bin/bash
#execute each bench 3 times and print the timing

exec 2>&1

#execute and time given exe 3 times 
bench_exe ()
{	
	echo "**************** $1 ****************"
	for i in {1..3}; do 
		time ./$1 $2; 
		rm  -f logs/*
		sleep 3
	done;
}

#execute  given async tests 3 times (timing is already builtin)
bench_async ()
{	
	echo "**************** $1 ****************"
	for i in {1..3}; do 
		./$1 $2; 
		echo		
		rm  -f logs/*
		sleep 3
	done;
}  


echo "----------------------------------------------------------"
echo "Single threaded benchmarks.. (1 thread,  1,000,000 lines)"
echo "----------------------------------------------------------"
for exe in boost-bench glog-bench easylogging-bench spdlog-bench;
do
	bench_exe $exe 1
done;

echo "----------------------------------------------------------"
echo "Multi threaded benchmarks.. (10 threads,  1,000,000 lines)"
echo "----------------------------------------------------------"
for exe in boost-bench-mt glog-bench-mt easylogging-bench-mt spdlog-bench-mt;
do
	bench_exe $exe 10
done;

echo "----------------------------------------------------------"
echo "Multi threaded benchmarks.. (100 threads,  1,000,000 lines)"
echo "----------------------------------------------------------"
for exe in boost-bench-mt glog-bench-mt easylogging-bench-mt spdlog-bench-mt;
do
	bench_exe $exe 100
done;
	

echo "---------------------------------------------------------------"
echo "Async, single threaded benchmark.. (1 thread,  1,000,000 lines)"
echo "---------------------------------------------------------------"
for exe in spdlog-async g2log-async
do
	bench_async $exe 1
done;

echo "---------------------------------------------------------------"
echo "Async, multi threaded benchmark.. (10 threads,  1,000,000 lines)"
echo "---------------------------------------------------------------"
for exe in spdlog-async g2log-async
do
	bench_async $exe 10
done;


echo "---------------------------------------------------------------"
echo "Async, multi threaded benchmark.. (100 threads,  1,000,000 lines)"
echo "---------------------------------------------------------------"
for exe in spdlog-async g2log-async
do
	bench_async $exe 100
done;



