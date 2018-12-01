#!/bin/bash

[[ $interleave ]] && numactl="numactl --interleave=$interleave --"

sysbench_cmd=(
	$numactl
	sysbench
	--time=$time
	memory
	--memory-block-size=$memory_block_size
	--memory-total-size=1024T
	--memory-scope=$memory_scope
	--memory-oper=$memory_oper
	--memory-access-mode=rnd
	--rand-type=$rand_type
	--rand-pareto-h=0.1
	--threads=$threads
	run
)

echo "${sysbench_cmd[@]}"
time "${sysbench_cmd[@]}" &

[[ $pid_file ]] && echo $! > $pid_file