#!/bin/bash
#SBATCH --job-name=p3arun
#SBATCH --output=p3arun.out
#SBATCH --error=p3arun.err
#SBATCH --time=00:05:00
#SBATCH --ntasks=20
#SBATCH --partition=fast

mpirun ./p3a -e "thisPositionIsForBerthFile.txt" -t 100 > evaluated_berths_info.txt

