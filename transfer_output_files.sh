#!/bin/bash
#$1 - port abbreviation ("lh", "sh" , "si", etc.), $2 - proces number, $3 - inst_times(s)
declare -i proc=$2

#subfolder for proc number of processes
mkdir -p ./quay_divisions/$1/p${proc}
#rsync -av --progress ./quay_divisions/* ./quay_divisions/$1/p${proc} --exclude ./quay_divisions/$1
mv ./quay_divisions/?_* ./quay_divisions/$1/p${proc}

#move logs and execution times to process folder
mv evaluator.log ./quay_divisions/$1/p${proc}
mv local_search.log ./quay_divisions/$1/p${proc}
mv $3 ./quay_divisions/$1/p${proc}

