#!/bin/bash
#SBATCH --job-name=p3arun
#SBATCH --output=p3arun.out
#SBATCH --error=p3arun.err
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --partition=fast

mkdir ./quay_divisions

#-t(inst) -p(port)
for ((i = 50; i >= 10; i = i - 10))
do
	for((j = 1; j <= 3; j++))
	do
		mpiexec ./p3a -t $i -p $1
		python get_inst_time.py + "\n"
	done
done

rsync -av --progress ./quay_divisions/* ./quay_divisions/$1 --exclude ./quay_divisions/$1/*
#delete directories inside quay_divisions/.   dir_name = /^\d\w+$/
for((i = 0; i < 10; i = i + 1))
do
	rm -r ./quay_divisions/$i*
done
python file_system_v2.py

