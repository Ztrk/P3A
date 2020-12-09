import json
import os

def extract_division_from_line(line):
    extracted = line.split(" ", 1)[1].replace(" ", "_")[0:-2]
    return extracted

with open('p3a_config.json') as json_file:
    data = json.load(json_file)
    num_instances = data['n_instances']

    bap_algorithms = data['bap_algorithms']

    bap_algorithms_count = len(bap_algorithms)

    bap_alg_names = []

    for bap_a in bap_algorithms:
	alg_name = str(bap_a['algorithm']) + "-" + str(bap_a['scheduling_policy']) + "-" + str(bap_a['future_arrivals'])
	bap_alg_names.append(alg_name)

    rows_for_instance = num_instances * bap_algorithms_count
    print("instances no: " + str(num_instances))
    print("bap algoithms no: " + str(bap_algorithms_count))
    print("rows for instance: " + str(rows_for_instance))


    evaluator_log = open('evaluator.log', 'r')

    instance_no = 0
    
    folder_division = "quay_divisions"
    subfolder_division = ""
    merged_division_folder = ""
    
    instance_folder = ""
    instance_whole_folder = ""

    line_no = 0
    stats_line = ""

    for position, line in enumerate(evaluator_log):
	#print(str(position) + " " + line)	
	if position % (rows_for_instance + 2) == 0:
		instance_no = 0
		subfolder_division = "/" + extract_division_from_line(line)
		merged_division_folder = folder_division + subfolder_division
	elif position % (rows_for_instance + 2) == 1:
		stats_line = line
		continue
	else:
		if line_no == 0:
			original_umask = os.umask(0)
			instance_folder = "/instance" + str(instance_no) + "/"
			instance_whole_folder = merged_division_folder + instance_folder
			try:
				os.makedirs(instance_whole_folder, 0755 )
			except:
				continue
		
		try:
			file_name = bap_alg_names[ line_no ] + ".txt"
			filepath = os.path.join(instance_whole_folder, file_name)
			tmp_file = open(filepath, "w")
			tmp_file.write(stats_line)
			tmp_file.write(line)
			tmp_file.close()
		except:
			print("file already exists.")
		line_no += 1
		if line_no == bap_algorithms_count:
			instance_no += 1
			line_no = 0
		if(instance_no == 100):
			instance_no = 0
		


	   
