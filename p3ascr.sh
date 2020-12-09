#!/bin/bash
#SBATCH --job-name=p3arun
#SBATCH --output=p3arun.out
#SBATCH --error=p3arun.err
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=20
#SBATCH --partition=fast
#SBATCH --mem-per-cpu=1500MB

module load mpich
rm -r quay_divisions
mkdir quay_divisions
mpiexec ./p3a > evaluated_berths_info.txt

