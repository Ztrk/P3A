import json
import sys
import os


def get_bap_dir_name(bap_entry):
	return bap_entry["algorithm"] + "-" + bap_entry["scheduling_policy"] + "-" + "future_arrivals" + str(bap_entry["future_arrivals"])

fslog = open("file_system.log", "w")

port_name = sys.argv[1]

with open('./p3a_config.json') as config_json:
  data = json.load(config_json)

bap_algorithms = data["bap_algorithms"]
bap_length = len(bap_algorithms)

instances = data["n_instances"]

ev_log = open("evaluator.log", "r")
Lines = ev_log.readlines() 

numbers = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]

bap_no = -1
inst_no = 0

division_info = ""

BLF_dir = ""

for line in Lines:
    if line[0] in numbers:
	bap_no = bap_no + 1
	info_tmp = "inst(" + str(inst_no) + "), bap(" + str(bap_no) + "): " +  line
	
	#file path - START
	original_umask = os.umask(0)

	BLF_path = "quay_divisions/" + port_name  + "/" + BLF_dir
	
	bap_dir = "/inst" + str(inst_no) + "/"
	
	whole_path = BLF_path + bap_dir
	
	try:
		os.makedirs(whole_path, 0755 )
	except:
		fslog.write("file already exists: (" + whole_path + ")\n")

	bap_file_name = get_bap_dir_name(bap_algorithms[bap_no]) + ".txt"
	
	bap_filepath = os.path.join(whole_path, bap_file_name)
	bap_lb_path = os.path.join(whole_path, "lb") + ".txt"

	bap_file_w_mode = open(bap_filepath, "w")
	bap_lb_file = open(bap_lb_path, "w")

	bap_file_w_mode.write("MWFT(norm) MWFT LB\n")
	bap_file_w_mode.write(line)

	bap_lb_file.write("Lower Bound: " + line.split()[2])

	bap_file_w_mode.close()

	#file path - END

	division_info = division_info + info_tmp + "\n"
	if bap_no == bap_length - 1:
		bap_no = -1
		inst_no = inst_no + 1
    else:
    	if line[0] == "E":
		inst_no = 0
		#if first BLF parsed
		if BLF_dir != "":
			MWFT_detailed = open("quay_divisions/" + port_name  + "/" + BLF_dir + "/MWFT_all.txt", "w")
			MWFT_detailed.write(division_info)
			MWFT_detailed.close()
		BLF = line.split(": ")[1].split()
		BLF_dir = "_".join(BLF)
		division_info = ""
		divisiion_info = division_info + BLF_dir + "\n"
fslog.close()
