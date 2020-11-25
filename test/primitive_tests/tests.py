import random
import math
import os
import shutil
import subprocess

class Ship:
    def __init__(self, no, ready, ln, proc_time, weight, own):
        self.no=no
        self.ready=ready
        self.length=ln
        self.proc_time=proc_time
        self.weight=weight
        self.owner=own

def write_reality(filename_ships, filename_berths, shipz, berthz):
    try:
        shutil.rmtree('../../instances')
    except:
        pass
    os.mkdir('../../instances')

    with open(filename_ships, 'w') as outer:
        outer.write(f'{len(shipz)}\n')
        for x in shipz:
            outer.write(f'{x.no} {x.ready} {x.length} {x.proc_time} {x.owner}\n')

    with open(filename_berths, 'w') as outer:
        outer.write(f'{len(berthz)}\n')
        for x in berthz:
            outer.write(f'{x}\n')

def get_res(correct_res):
    with open('result.txt', 'r') as outer:
        line=outer.readline()
        while line:
            old_line=line
            line=outer.readline()
    print(int(old_line), correct_res)

def procession(res, ships, berths):
    write_reality(f'../../instances/instance{i}.txt', f'test_b_no_{i}.txt', ships, berths)
    bashCommand = os.system("mpirun ../../build/p3a -i ./test_b_no_{i}.txt > result.txt")
    get_res(res)


for i in range(3):
    L=random.randint(1, 15) #berth len
    ships=[]
    berths=[L]

    m=random.randint(1, 20) #segments
    Q=L*m #full quayside
    k=random.randint(1, 30) #number of ship batches
    n=k*m #number of ships
    for j in range(n):
        x=Ship(j, 0, L, 1, 1, 1)
        ships.append(x)
    procession((k*(k-1))//2*m, ships, berths)


for i in range(3):
    berths=[]
    ships=[]
    for x in range(1, 20):
        L=random.randint((x-1)*20, x*20)
        berths.append(L)
    mx=L

    for j in range(2):
        x=Ship(j, 0, mx, 1, 1, 1)
        ships.append(x)
    procession(1, ships, berths)

for i in range(3):
    L=random.randint(1, 15) #berth len
    ships=[]
    berths=[L]
    m=random.randint(1, 20) #segments
    Q=L*m #full quayside

    k=random.randint(2, 30) #number of ship batches
    n=k*m #number of ships

    for j in range(n):
        x=Ship(j, j, L, m, 1, 1)
        ships.append(x)

    procession(0, ships, berths)
