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
from scipy.signal import find_peaks, butter
import heartpy as hp


def find_breath_peaks(sig):
    # TODO: Preprocess
    # Filter
    from scipy.signal import butter, sosfilt
    sos = butter(8, 0.15, 'hp', fs=25, output='sos')
    filtered = sosfilt(sos, sig)
    # find peaks
    peaks = find_peaks(filtered)[0]
    return peaks, filtered


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--raw_dir", type=str, help="NLM directory.")
    parser.add_argument("--sample_rate", type=int, help="NLM directory.")
    parser.add_argument("--detect_duration", type=int, help="NLM directory.")
    parser.add_argument("--manual_infos_path", type=str, help="")
    parser.add_argument("--golden_dir", type=str, help="")
    parser.add_argument("--result_dir", type=str, help="")
    args = parser.parse_args()

    manual_infos = pd.read_csv(args.manual_infos_path).iloc[3:]
    window_length = args.detect_duration * args.sample_rate

    for index, series in manual_infos.iterrows():
        intersect_start = series[["golden_start", "start"]].max()
        intersect_end = series[["golden_end", "end"]].min()

        # Read NLM format files.
        record_path = os.path.join(args.raw_dir, series["file_name"])
        start = int((intersect_start - series["start"]) * series["fs"])
        end = int((series["end"] - intersect_end) * series["fs"])
        ppg = utils.read_nlmfile(record_path, 1, np.uint16, 1, start, end)

        # Calculate breath parameters.
        ppg_peaks, ppg_f = find_breath_peaks(ppg)
        breathingrates = []
        ppg_peaks = []
        for i in range(0, len(ppg_f), window_length):
            segment = ppg_f[i:i + window_length]
            measurements = hp.process(segment, 25, clean_rr=True)
            br = measurements[-1]["breathingrate"]
            breathingrates.append(br)
            pl = measurements[0]["peaklist"]
            pl = [p + i for p in pl]
            ppg_peaks.extend(pl)

        # Read Philips format files.
        record_path = os.path.join(
            args.golden_dir, series["golden_dir"] + "/TFlow.csv")
        start = int((intersect_start - series["golden_start"])
                    * series["golden_fs"])
        end = int((series["golden_end"] - intersect_end) * series["golden_fs"])
        golden = utils.read_golden_philips(record_path, np.int16, start, end)

        # Calculate golden breath.
        peaks_golden = find_peaks(golden, distance=50, height=1)[0]
        print("golden: ", golden)
        breathingrates_golden = []
        for i in range(0, len(ppg_f), window_length):
            peaks_segment = peaks_golden[(peaks_golden > i) & (
                peaks_golden < i + window_length)]
            peaks_mean = np.mean(peaks_segment[1:] - peaks_segment[:-1])
            br = args.sample_rate / peaks_mean
            breathingrates_golden.append(br)

        fig, axes = plt.subplots(3, 1, figsize=(15, 8), sharex=True)
        axes[0].plot(ppg_f)
        axes[0].plot(ppg_peaks, ppg_f[ppg_peaks], "bo")
        axes[1].plot(golden)
        axes[1].plot(peaks_golden, golden[peaks_golden], "ro")
        axes[2].plot(range(0, len(ppg_f), window_length), breathingrates, "ro")
        axes[2].plot(range(0, len(ppg_f), window_length),
                     breathingrates_golden, "b*")
        # axes[2].set_ylim(0, 0.5)
        plt.show()

        a = input("-->")
