import sys

def mean_from_arr(arr):
	mean = 0
	arr_len = len(arr)
	for i in range (0,arr_len):
		mean = mean + arr[i]
	return mean / arr_len

inst_output_times = open(sys.argv[1], "r")

line_no = 0

inst_mean_times = open(sys.argv[2], "w")

inst_mean_times.write("instances: mean_time:\n")

for line in inst_output_times:
	if(line_no == 0):
		n_inst = int(line)
		times_arr = []
		line_no = line_no + 1
	else:
		times_arr.append(float(line))
		line_no = line_no + 1
		if(line_no == 3 + 1):
			times_arr.sort()
			mean_time = mean_from_arr(times_arr)
			inst_mean_times.write(str(n_inst) + " " + str(mean_time) + "\n")
			line_no = 0		
inst_mean_times.close()
inst_output_times.close()
