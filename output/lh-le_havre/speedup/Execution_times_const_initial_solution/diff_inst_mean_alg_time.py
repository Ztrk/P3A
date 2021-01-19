import sys

# plik 1 (argv[1] to zbior wartosci: 'liczba instancji' - 'czasy dla tej liczby instancji (3 do 5)'
# plik 2 (argv[2] to plik wynikowy ze srednimi czasami wykonania algorytmu dla kazdej instancji)

def mean_from_arr(arr, inst_to_consider):
	mean = 0
 	for i in range (0, len(arr)):
		mean = mean + arr[i]
	return (mean / inst_to_consider)

inst_output_times = open(sys.argv[1])

results_per_instance_no = 3

line_no = 0

f = open(sys.argv[2], "w")

f.write("instances: mean_time:\n")

first_fetched = False

for line in inst_output_times:
	if(line_no == 0):
		n_inst = int(line)
		times_arr = []
		line_no = line_no + 1
	else:
		times_arr.append(float(line))
		line_no = line_no + 1
		if(line_no == results_per_instance_no + 1):
			if len(times_arr) > 3:
				times_arr.sort()
				times_arr.pop()
				times_arr.pop(0)
			mean_time = mean_from_arr(times_arr, len(times_arr))
			f.write(str(n_inst) + " " + str(mean_time) + "\n")
			line_no = 0		
f.close()
inst_output_times.close()
