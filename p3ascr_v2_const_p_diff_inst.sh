#!/bin/bash
#SBATCH --job-name=p3arun
#SBATCH --output=p3arun.out
#SBATCH --error=p3arun.err
#SBATCH --time=01:00:00
#SBATCH --ntasks=10
#SBATCH --partition=fast

module load mpich

for ((i = 50; i>=10; i = i - 10))
do
	echo "$i" >> 10tasks_50to10inst.txt
	for((j = 1; j <= 5; j++))
	do
		mpiexec ./p3a -t $i > evaluated_berths_info.txt
		python get_inst_time.py + "\n"  >> 10tasks_50to10inst.txt
	done
done
