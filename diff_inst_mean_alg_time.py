def mean_from_arr(arr):
	mean = 0
 	for i in range (0, len(arr)):
		mean = mean + arr[i]
	return (mean / 3)

inst_output_times = open("10tasks_100to10inst.txt")

line_no = 0

f = open("mean_times_const_proc_diff_inst.txt", "w")

f.write("instances: acceleration:\n")

first_fetched = False

for line in inst_output_times:
	if(line_no == 0):
		n_inst = int(line)
		times_arr = []
		line_no = line_no + 1
	else:
		times_arr.append(float(line))
		line_no = line_no + 1
		if(line_no == 6):
			times_arr.sort()
			times_arr.pop()
			times_arr.pop(0)
			mean_time = mean_from_arr(times_arr)
			if(not first_fetched):
				first_fetched = True
				acceleration = 1.0
				basic_time = mean_time
			else:
				acceleration = basic_time / mean_time
			f.write(str(n_inst) + " " + str(acceleration) + "\n")
			line_no = 0		
f.close()
inst_output_times.close()
