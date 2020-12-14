with open("local_search.log") as f:
    content = f.readlines()
    result_index = len(content) - 1
    result = content[ result_index ].split()
    print(result[0])
