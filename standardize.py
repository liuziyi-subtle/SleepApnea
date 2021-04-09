from glob import glob


golden_dirs = glob(
    "/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/SleepRespiratory/*Alice6")
raw_paths = glob(
    "/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/**/内部格式/心率*.dat")

golden_dirs = list(golden_dirs)
file_name = list(raw_paths)

print(list(raw_paths))
print(len(golden_dirs), len(raw_paths))

# dirs = glob(dir)

# for d in dirs:
#     print(d.split("/")[-1])
