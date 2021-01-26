#!/bin/bash
#SBATCH --job-name=sys_p3arun
#SBATCH --output=sys_p3arun.out
#SBATCH --error=sys_p3arun.err
#SBATCH --time=00:30:00
#SBATCH --partition=fast
#SBATCH --ntasks=6

mkdir ./quay_divisions
#S1 port abbreviation
#$2 -t(inst)
mkdir -p ./quay_divisions/$1


declare -i mp=8

for ((i = 1 ; i <= mp; i = i + 1))
do
	if (($2%$i==0)); then
		#$2 - instances amount
		mpirun -n $i ./p3a "-t" $2
		#execution time
		python get_inst_time.py + "\n" >> ${i}tasks$2inst.txt

		#tmp comment	
		./transfer_output_files.sh $1 $i ${i}tasks$2inst.txt
		
		#remove BLF folders from main folder - quay_divisions
		for((j = 0; j < 10; j = j + 1))
		do
			rm -r ./quay_divisions/$j*
		done

		python file_system_v2.py $1 $i
	fi
done

#python file_system_v2.py $1 ${mp}

mv *.err ./quay_divisions/$1/
mv *.out ./quay_divisions/$1/

