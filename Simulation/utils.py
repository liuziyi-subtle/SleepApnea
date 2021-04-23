import time
import datetime
import pandas as pd
import numpy as np


def nlmtime2utc(nlmtime):
    """
    transfer nlmtime from file_name to utc time.
    e.g. 
    """
    localtime_splits = [int(nlmtime[:4])]
    for i in range(4, len(nlmtime), 2):
        localtime_splits.append(int(nlmtime[i:i+2]))

    year, month, day, hour, minute, second = localtime_splits
    localtime = datetime.datetime(year, month, day, hour, minute, second)
    utctime = time.mktime(localtime.timetuple())

    return utctime


def local2utc(localtime_str, localtime_str_format):
    utctime = datetime.datetime.strptime(localtime_str, localtime_str_format)
    utctime = time.mktime(utctime.timetuple())
    return utctime


def read_nlmfile(path, signal_type, d_type, columns, start, end):
    record = pd.read_csv(
        path, header=None, error_bad_lines=False, engine="python")
    record = record[record[0] != "APP_MSG"]
    record[0] = record[0].astype(np.int32)
    signal = record.loc[record[0] == signal_type,
                        columns].astype(d_type).values
    signal = signal[start:(len(signal) - end)]
    return signal


def read_golden_philips(path, d_type, start, end):
    record = pd.read_csv(
        path, header=None, error_bad_lines=False, engine="python")
    signal = record.iloc[1:, 0].astype(d_type).values
    signal = signal[start:(len(signal) - end)][::4]
    return signal
