#!/bin/bash

#SBATCH --job-name=sd_ntask
#SBATCH --output=thread_increase_dir/sd_pr_nt_256.out
#SBATCH --error=thread_increase_dir/sd_pr_nt_256.err
#SBATCH --time=5-00:00:00
#SBATCH --partition=standard
#SBATCH --ntasks=256

module load mpich

Process_max=256

filename="thread_increase_dir/standard_nt_256.txt"

mkdir "thread_increase_dir"
touch $filename

echo "process time[s]"
echo "process time[s]" >> $filename
for i in $(seq 1 $Process_max);
do
    echo "process amount $i"
    if [[ $i -ge 64 ]]
    then
        divider=10
    elif [[ $i -ge 20 ]] && [[ $i -le 29 ]]
    then
        divider=5
    elif [[ $i -ge 10 ]] && [[ $i -le 19 ]]
    then
        divider=3
    else
        divider=1
    fi
    echo "divider: $divider"
    MiddleTime=0
    for d in $(seq 1 $divider);
    do
        echo "measure $d"
        STARTTIME=$(date +%s)
        mpirun -n $i ./p3a > change_threads_output.txt
        ENDTIME=$(date +%s)
        Time=$[$ENDTIME - $STARTTIME]
        echo "time: $Time"
        let "MiddleTime+=Time"
    done
    let "MiddleTime=MiddleTime/divider"
    echo "$i $MiddleTime"
    echo "$i $MiddleTime" >> $filename
    echo "ende all measures"
done

echo "end all process"
