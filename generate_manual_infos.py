"""
1. Generate manual_infos file.
2. Move golden directories to Golden dir.
3. Move raw files to NLM dir.

Note: New collected located in
/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/New.
"""

import pandas as pd
import numpy as np
from glob import glob
import os
import shutil
import utils

new_dir = "/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/New"
raw_dir = "/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/Raw"
golden_dir = os.path.join(raw_dir, "Golden")
nlm_dir = os.path.join(raw_dir, "NLM")

# Get (golden dir, raw file) pairs.
dict_golden2nlm = {}

philip_data_dirs = glob(os.path.join(new_dir, "SleepRespiratory/*Alice6"))
for pdd in philip_data_dirs:
    pdd_splits = pdd.split("/")[-1]  # Directory name
    person = pdd_splits.split("_")[1]
    date = pdd_splits.split("_")[2]
    fs = 100
    # start and end utc time
    record = pd.read_csv(pdd + "/TFlow.csv", header=None,
                         error_bad_lines=False, engine="python")
    start = "{}-{}-{} {}".format(date[:4], date[4:6], date[6:8],
                                 record.loc[0, 0])
    end = "{}-{}-{} {}".format(date[:4], date[4:6], date[6:8],
                               record.loc[0, 1])
    start = utils.local2utc(start, "%Y-%m-%d %H:%M:%S")
    end = utils.local2utc(end, "%Y-%m-%d %H:%M:%S")
    # New key
    key = person + date  # e.g. [伍远方20210330]
    dict_golden2nlm[key] = [pdd_splits, start, end, fs]
    # Move to Golden dir
    shutil.copytree(pdd, os.path.join(golden_dir, pdd_splits))

# 心率*.dat -- green / 血氧*.dat -- ir.
raw_file_paths = glob(os.path.join(new_dir, "**/内部格式/心率*.dat"))
for rfp in raw_file_paths:
    rfp_splits = rfp.split("/")[-1]
    date = rfp_splits.split("-")[-2][:8]  # YMD from YMDHMS
    person = rfp_splits.split("-")[-4]
    # start and end utc time
    record = pd.read_csv(rfp, header=None,
                         error_bad_lines=False, engine="python")
    record = record[record[0] != "APP_MSG"]
    record[0] = record[0].astype(np.int32)
    ts = record.loc[record[0] == 1, 2].astype(np.int32)
    fs = record.loc[record[0] == 112, 1].astype(np.int32).values[0]
    start = utils.nlmtime2utc(rfp_splits.split("-")[-2])
    end = start + round(ts.values[-1] / 1000)
    # Update
    dict_golden2nlm[person +
                    date].extend([rfp_splits, start, end, fs, date, person])
    # Move to NLM dir
    shutil.copyfile(rfp, os.path.join(nlm_dir, rfp_splits))

# Generate manual_infos
values = [value for _, value in dict_golden2nlm.items()]
manual_infos = pd.DataFrame(values,
                            columns=["golden_dir", "golden_start", "golden_end",
                                     "golden_fs", "file_name", "start", "end",
                                     "fs", "date", "person"])
# file_names = [value[1] for key, value in dict_golden2nlm.items()]
# golden_dirs = [value[0] for key, value in dict_golden2nlm.items()]
# persons = [value[0].split("_")[1] for key, value in dict_golden2nlm.items()]
# dates = [value[0].split("_")[2] for key, value in dict_golden2nlm.items()]

# manual_infos = pd.DataFrame({"file_name": file_names, "person": persons,
#                              "date": dates, "golden_dir": golden_dirs})
manual_infos.to_csv(raw_dir + "/manual_infos.csv",
                    index=False, encoding="utf_8_sig")
