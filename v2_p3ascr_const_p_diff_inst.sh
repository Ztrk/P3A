#!/bin/bash
#SBATCH --job-name=p3arun
#SBATCH --output=p3arun.out
#SBATCH --error=p3arun.err
#SBATCH --time=00:02:00
#SBATCH --nodes=1
#SBATCH --ntasks=5
#SBATCH --partition=fast


module load mpich
mkdir ./quay_divisions


for ((i = 5; i>=5; i = i - 5))
do
	for((j = 1; j <= 1; j++))
	do
		mpiexec ./p3a -t $i -p $1
		python get_inst_time.py + "\n"
		#python file_system.py
	done
done

rsync -av --progress ./quay_divisions/* ./quay_divisions/$1 --exclude ./quay_divisions/$1/*
#delete directories inside quay_divisions/.
for((i = 0; i <= 10; i = i + 1))
do
	rm -r ./quay_divisions/$i*
done
python file_system_v2.py

