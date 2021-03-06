import random
import math
import os
import shutil
import subprocess

passed=0
all_tests=0
class Ship:
    def __init__(self, no, ready, ln, proc_time, weight, own):
        self.no=no
        self.ready=ready
        self.length=ln
        self.proc_time=proc_time
        self.weight=weight
        self.owner=own

def write_reality(filename_ships, filename_berths, shipz, berthz, Q, n_instances):
    try:
        shutil.rmtree('./instances')
    except:
        pass
    os.mkdir('./instances')

    for i in range(0, n_instances):
        with open(f'{filename_ships}{i}.txt', 'w') as outer:
            outer.write(f'{len(shipz)}\n')
            for x in shipz:
                outer.write(f'{x.no} {x.ready} {x.length} {x.proc_time} {x.weight} {x.owner}\n')

    with open(filename_berths, 'w') as outer:
        outer.write(f'{Q}\n')
        for x in berthz:
            outer.write(f'{x}\n')

def get_res(correct_res):
    with open('result.txt', 'r') as outer:
        line=outer.readline()
        while line:
            old_line=line
            line=outer.readline()
    art=1 if float(old_line)-correct_res<0.000001 else 0
    global all_tests
    global passed
    all_tests+=1
    passed=passed+art
    print(f'{"PASSED" if art==1 else "NOT PASSED"} {float(old_line)} {correct_res}')

def procession(res, ships, berths, Q):
    write_reality(f'./instances/instance', f'test_b_no_{i}.txt', ships, berths, Q, 4)
    bashCommand = os.system(f"mpirun ./build/p3a -i ./test_b_no_{i}.txt > result.txt")
    normal=sum([x.proc_time for x in ships])/len(ships)
    get_res((res+sum([x.proc_time for x in ships]))/(len(ships)*normal))


pathway='./test/primitive_tests/jsons/'
reality='p3a_config.json'
falsity='test_config.json'
dot='./'
try:
    os.rename(dot+reality, pathway+reality)
    os.rename(pathway+falsity, dot+reality)
    print('Basic case')
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
        procession((k*(k-1))//2*m, ships, berths, Q)

    print('Test Max')
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
        procession(1, ships, berths, Q)

    print('Cyclic')
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
        procession(0, ships, berths, Q)

    print('Trivial case - 2 ships')
    berths=[5]
    ships=[]
    Pt=2
    x=Ship(0, 0, 5, Pt, 1, 1)
    ships.append(x)

    x=Ship(1, 1, 5, Pt, 1, 1)
    ships.append(x)

    procession(1, ships, berths, 5)
    print(f'PASSED: {passed} out of {all_tests}')
except:
    pass
finally:
    os.rename(dot+reality, pathway+falsity)
    os.rename(pathway+reality, dot+reality)
    
