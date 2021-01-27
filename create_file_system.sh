#!/bin/bash
#SBATCH --job-name=sys_p3arun
#SBATCH --output=sys_p3arun.out
#SBATCH --error=sys_p3arun.err
#SBATCH --time=00:05:00
#SBATCH --partition=fast
#SBATCH --ntasks=8

mkdir ./quay_divisions
#S1 port abbreviation
#$2 -t(inst)
mkdir -p ./quay_divisions/$1


declare -i mp=4

for ((i = 1 ; i <= mp; i = i + 1))
do
	if (($2%$i==0)); then
		#$2 - number of instances
		mpirun -n $i ./p3a "-t" $2 >> output_stream_${i}tasks$2inst.txt
		#execution time
		python get_inst_time.py + "\n" >> ${i}tasks$2inst.txt

		./transfer_output_files.sh $1 $i ${i}tasks$2inst.txt output_stream_${i}tasks$2inst.txt

		python file_system_v2.py $1 $i
	fi
done

mv *.err ./quay_divisions/$1/
