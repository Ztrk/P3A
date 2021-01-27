import json
import sys
import os


def get_bap_dir_name(bap_entry):
    algo = bap_entry["algorithm"] + "-" + bap_entry["scheduling_policy"]
    f_arr = "-" + "future_arrivals" + str(bap_entry["future_arrivals"])
    return algo + f_arr


fs_log = open("file_system.log", "w")
port_name = sys.argv[1]
proc_exec = sys.argv[2]
port_path = "quay_divisions/" + port_name + "/" + "p" + str(proc_exec) + "/"

with open('./p3a_config.json') as config_json:
    data = json.load(config_json)

bap_algorithms = data["bap_algorithms"]
bap_length = len(bap_algorithms)
berths = data["berths"]

evaluator_location = port_path + "evaluator.log"
ev_log = open(evaluator_location, "r")
Lines = ev_log.readlines()
ev_log.close()

berth_lengths = open(port_path + "berth_lengths.txt", "w")
berth_str = "berth lengths:\n["
for berth in berths:
	berth_str = berth_str + str(berth) + ", "

berth_str = berth_str[:-2]
berth_str = berth_str + "]"

berth_lengths.write(berth_str)
berth_lengths.close()


numbers = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
bap_no = -1

for line in Lines:
    if line[0] in numbers:
        bap_no = bap_no + 1
        # file path - START
        original_umask = os.umask(0)
	BLF_path = port_path + BLF_dir
        bap_dir = "/inst" + str(inst_no) + "/"
        whole_path = BLF_path + bap_dir
        try:
            os.makedirs(whole_path, 0o755)
        except:
            fs_log.write("file already exists: (" + whole_path + ")\n")
        
	bap_file_name = get_bap_dir_name(bap_algorithms[bap_no]) + ".txt"
        bap_filepath = os.path.join(whole_path, bap_file_name)
        
	bap_lb_path = os.path.join(whole_path, "lb") + ".txt"
        bap_lb_file = open(bap_lb_path, "w")
	bap_lb_file.write("Lower Bound: " + line.split()[2])
	bap_lb_file.close()

	bap_file_w_mode = open(bap_filepath, "w")
	bap_file_w_mode.write("MWFT(norm) MWFT LB\n")
        bap_file_w_mode.write(line)
	bap_file_w_mode.close()

        # file path - END
        if bap_no == bap_length - 1:
            bap_no = -1
            inst_no = inst_no + 1
    else:
        if line[0] == "E":
            inst_no = 0
            BLF = line.split(": ")[1].split()
            BLF_dir = "_".join(BLF)
fs_log.close()
