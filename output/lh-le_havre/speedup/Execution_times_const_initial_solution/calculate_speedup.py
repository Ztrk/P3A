import sys

#argv[1] - 10/5 proc , argv[2] - 1 proc
with open(sys.argv[1]) as proc_10:
	content_10 = proc_10.readlines()
with open(sys.argv[2]) as proc_1:
	content_1 = proc_1.readlines()
speedups = open("different_inst_speedups.txt", "w")
speedups.write(content_10[0].split()[0] + " speedup:\n")
content_10.pop(0)
content_1.pop(0)
for t10,t1 in zip (content_10, content_1):
	t10l = t10.split()
	t1l = t1.split()
	speedups.write( str(t10l[0]) + " " + str( float(t1l[1]) / float(t10l[1]) ) + "\n")
speedups.close()
