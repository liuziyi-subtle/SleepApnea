"""
1. Generate intersect start and end utctime between golden and raw files.
2. Generate peak index of TFlow as golden.
3. Generate json file as annotation.
"""

import pandas as pd
import os
import numpy as np
import utils
import matplotlib.pyplot as plt
from scipy.signal import find_peaks


def align_timestamps(ts1, ts1_utc, ts2, ts2_utc):
    pass


def check_data_integrity(x):
    if not isinstance(x, np.ndarray):
        x = np.asarray(x)
    diff = x[1:] - x[:-1]
    return np.where(diff != 40)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--raw_dir", type=str, help="NLM directory.")
    parser.add_argument("--golden_dir", type=str, help="Golden direcory.")
    parser.add_argument("--manual_infos_path", type=str,
                        help="manual_infos path.")
    args = parser.parse_args()

    manual_infos = pd.read_csv(args.manual_infos_path)
    for index, series in manual_infos.iterrows():
        intersect_start = series[["golden_start", "start"]].max()
        intersect_end = series[["golden_end", "end"]].min()
        
        record_golden = pd.read_csv(
            os.path.join(args.golden_dir, series["golden_dir"] + "/TFlow.csv"),
            header=None, error_bad_lines=False, engine="python")
        tflow = record_golden.iloc[1:, 0].astype(np.int16).values
        start = int((intersect_start - series["golden_start"])
                    * series["golden_fs"])
        end = int((series["golden_end"] - intersect_end) * series["golden_fs"])
        tflow = tflow[start:(len(tflow) - end)][::4]
        peaks = find_peaks(tflow)[0]
        print("peaks: ", peaks)
        print("tflow[peaks]): ", tflow[peaks])

        fig, axes = plt.subplots(2, 1, figsize=(15, 8), sharex=True)
        axes[0].plot(ppg)
        axes[1].plot(tflow)
        axes[1].plot(peaks, tflow[peaks], "ro")
        plt.show()

        a = input("-->")

        # print(ts_golden_start, type(ts_golden_start))
