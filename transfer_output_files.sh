#!/bin/bash
#$1 - port abbreviation ("lh", "sh" , "si", etc.), $2 - number of processes, $3 - inst_times(s), $4 - std_output
declare -i proc=$2

#subfolder for number of processes
mkdir -p ./quay_divisions/$1/p${proc}
mv ./quay_divisions/?_* ./quay_divisions/$1/p${proc}

#move logs and execution times to process folder
mv evaluator.log ./quay_divisions/$1/p${proc}
mv local_search.log ./quay_divisions/$1/p${proc}
mv $3 ./quay_divisions/$1/p${proc}
mv $4 ./quay_divisions/$1/p${proc}
