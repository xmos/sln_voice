
import csv
import re
import sys
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

if __name__ == "__main__":
    fname = sys.argv[1]

    data0 = []
    data1 = []
    timestamps_dir0 = []
    bytes_dir0 = []

    timestamps_dir1 = []
    bytes_dir1 = []

    with open(fname) as fl:
        for line_num, row in enumerate(fl.readlines()):
            if 'beagle' in fname:
                if m := re.match(r'\d+,HS,\d+,(\d+):(\d+)\.(\d+)\.(\d+),.*,([0-9]+) B,.*,.*,.*,(\w+) txn', row):
                    if m.group(6) == 'OUT':
                        timestamps_dir0.append(int(m.group(1))*60e8 + int(m.group(2))*1e8 + int(m.group(3))*1e5 + int(m.group(4))*1e2)
                        bytes_dir0.append(int(m.group(5)))
                    else:
                        timestamps_dir1.append(int(m.group(1))*60e8 + int(m.group(2))*1e8 + int(m.group(3))*1e5 + int(m.group(4))*1e2)
                        bytes_dir1.append(int(m.group(5)))
            else:
                if m := re.match(r'([-0-9]+),([-0-9]+),([-0-9]+)', row):
                    if(int(m.group(1)) == 0):
                        timestamps_dir0.append(int(m.group(2)))
                        bytes_dir0.append(int(m.group(3)))
                    else:
                        timestamps_dir1.append(int(m.group(2)))
                        bytes_dir1.append(int(m.group(3)))

    span_dir0 = [int(t - s)&0xffffffff for s, t in zip(timestamps_dir0, timestamps_dir0[1:])]
    span_dir1 = [int(t - s)&0xffffffff for s, t in zip(timestamps_dir1, timestamps_dir1[1:])]


    span_dir0_np = np.array(span_dir0)
    span_dir1_np = np.array(span_dir1)
    
    print(f"dir0 avg = {np.average(span_dir0_np)}")
    print(f"dir1 avg = {np.average(span_dir1_np)}")
    
    window_len = 64000
    window_avg_0 = pd.Series(span_dir0_np).rolling(window=window_len).mean().iloc[window_len-1:].values
    std_dev = np.std(window_avg_0[:-window_len])
    mean = np.mean(window_avg_0[:-window_len])
    full_mean = np.mean(span_dir0_np)
    print(f"Dir0: window_len = {window_len}, mean {mean}, std_dev {std_dev}, avg_rate = {(48/mean)*100000}, full_avg_rate = {(48/full_mean)*100000}")

    window_avg_1 = pd.Series(span_dir1_np).rolling(window=window_len).mean().iloc[window_len-1:].values
    std_dev = np.std(window_avg_1[:-window_len])
    mean = np.mean(window_avg_1[:-window_len])
    full_mean = np.mean(span_dir1_np)
    print(f"Dir1: window_len = {window_len}, mean {mean}, std_dev {std_dev}, avg_rate = {(48/mean)*100000}, full_avg_rate = {(48/full_mean)*100000}")

    #d = np.array(data0)
    #x = [i for i in range(len(d))]
    #A = np.vstack([x, np.ones(len(x))]).T
    #m, c = np.linalg.lstsq(A, d, rcond=None)[0]
    #print(f"m = {m}, c = {c}")

    fig, axs = plt.subplots(2, 2)
    axs[0,0].plot(span_dir0)
    axs[0,1].plot(window_avg_0)

    axs[1,0].plot(span_dir1)
    axs[1,1].plot(window_avg_1)
    fig.tight_layout()
    plt.show()
