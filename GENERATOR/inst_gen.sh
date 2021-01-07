#!/bin/bash
#$1 - lh/rt/sh/si/hb - jaki port
#S2 - ile instancji do wygenerowania
string="instance"
mkdir instances_$1
 for i in $(seq 0 $[$2 - 1])
do
   ./$1_gen -o "instances_$1/$string$i.txt"
done
