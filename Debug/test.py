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
import c2py


def _get_c_debug_info():
    PLOT = c2py.GetPLOT()

    debug_info = {
        "x_interp": [PLOT.getitem_x_interp(i) for i in range(PLOT.x_interp_len)],
        "x_interp_len": PLOT.x_interp_len,

        "y_interp": [PLOT.getitem_y_interp(i) for i in range(PLOT.y_interp_len)],
        "y_interp_len": PLOT.y_interp_len,

        "y_interp_f": [PLOT.getitem_y_interp_f(i) for i in range(PLOT.y_interp_f_len)],
        "y_interp_f_len": PLOT.y_interp_f_len,

        "welch_density": [PLOT.getitem_welch_density(i) for i in range(PLOT.welch_density_len)],
        "welch_density_len": PLOT.welch_density_len,

        "peaks": [PLOT.getitem_peaks(i) for i in range(PLOT.peaks_len)],
        "peaks_len": PLOT.peaks_len,

        "peak_indices": [PLOT.getitem_peak_indices(i) for i in range(PLOT.peak_indices_len)],
        "peak_indices_len": PLOT.peak_indices_len,

        "ppg": [PLOT.getitem_ppg(i) for i in range(PLOT.ppg_len)],
        "ppg_len": PLOT.ppg_len,

        "ppg_f": [PLOT.getitem_ppg_f(i) for i in range(PLOT.ppg_f_len)],
        "ppg_f_len": PLOT.ppg_f_len,

        "rrs": [PLOT.getitem_rrs(i) for i in range(PLOT.rrs_len)],
        "rrs_len": PLOT.rrs_len,

        "breath_rates": [PLOT.getitem_breath_rates(i) for i in range(PLOT.breath_rates_len)],
        "breath_rates_len": PLOT.breath_rates_len
    }
    return debug_info


"""
Usage:
python3 analyze_breath.py --raw_dir=/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/Raw/NLM \
                          --golden_dir=/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/Raw/Golden \
                          --sample_rate=25 \
                          --detect_duration=32 \
                          --manual_infos_path=/Users/liuziyi/Documents/Lifesense/Data/SleepApnea/Raw/manual_infos.csv
"""
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

        # PLOT from C code.
        ppg_c = c2py.int_array(len(ppg))
        for i in range(len(ppg)):
            ppg_c[i] = int(ppg[i])
        c2py.BreathAnalysisInit()
        c2py.BreathAnalysis(ppg_c, len(ppg))
        debug_info = _get_c_debug_info()

        peaks = find_peaks(debug_info["ppg_f"], height=None, threshold=None, distance=6,
                           prominence=None, width=6, wlen=None, rel_height=0.5, plateau_size=None)
        peaks = peaks[0].astype(np.int32)

        fig, axes = plt.subplots(4, 1, figsize=(12, 7), sharex=False)
        axes[0].plot(debug_info["ppg"])
        axes[1].plot(debug_info["ppg_f"])
        axes[1].plot(debug_info["peak_indices"], debug_info["peaks"], "ro")
        axes[1].plot(peaks, [debug_info["ppg_f"][p] for p in peaks], "b*")
        axes[2].plot(debug_info["y_interp_f"])
        Pxx, freqs, bins, im = axes[3].specgram(
            debug_info["y_interp_f"], NFFT=600, Fs=25, noverlap=0)
        plt.show()

        a = input("-->")
