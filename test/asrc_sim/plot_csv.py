import csv
import re
import sys
import matplotlib.pyplot as plt
import numpy as np
import os
import shutil

def calc_ema(current_avg, new_data, alpha):
    new_avg = (alpha*current_avg) + ((1-alpha)*new_data);
    return new_avg


def plot_3_cols(fname):
    data0 = []
    data1 = []
    data2 = []
    with open(fname) as fl:
        #reader = csv.reader(fl, delimiter=',')
        for row in fl.readlines():
            if m := re.match(r'([-0-9]+),([-0-9]+),([-0-9]+)', row):
                data0.append(int(m.group(1)))
                data1.append(int(m.group(2)))
                data2.append(int(m.group(3)))

    d = np.array(data1)
    x = [i for i in range(len(d))]
    A = np.vstack([x, np.ones(len(x))]).T
    m, c = np.linalg.lstsq(A, d, rcond=None)[0]
    print(f"m = {m}, c = {c}")

    fig, axs = plt.subplots(2, 1)
    axs[0].plot(data0[5:])
    axs[1].plot(data1[5:])
    axs[1].plot(data2)
    #axs[0].plot(data1)
    fig.tight_layout()
    plt.show()

def plot_2_cols(fname, plotname="test.png", showplot=True):
    data0 = []
    data1 = []
    with open(fname) as fl:
        #reader = csv.reader(fl, delimiter=',')
        for row in fl.readlines():
            if m := re.match(r'([-0-9]+),([-0-9]+)', row):
                data0.append(int(m.group(1)))
                data1.append(int(m.group(2)))

    d = np.array(data1)
    x = [i for i in range(len(d))]
    A = np.vstack([x, np.ones(len(x))]).T
    m, c = np.linalg.lstsq(A, d, rcond=None)[0]
    print(f"m = {m}, c = {c}")

    fig, axs = plt.subplots(2, 1)
    axs[0].plot(data0)
    axs[1].plot(data1)
    axs[0].set_ylabel("Current buf fill level")
    axs[0].set_xlabel("index")
    axs[1].set_ylabel("Avg buf fill level")
    axs[1].set_xlabel("index")

    fig.tight_layout()
    plt.savefig(plotname)
    if showplot == True:
        plt.show()




def plot_1_cols(fname, plotname="test.png", showplot=True):
    data0 = []
    with open(fname) as fl:
        #reader = csv.reader(fl, delimiter=',')
        for row in fl.readlines():
            if m := re.match(r'([-0-9]+)', row):
                data0.append(int(m.group(1)))


    d = np.array(data0)
    x = [i for i in range(len(d))]
    A = np.vstack([x, np.ones(len(x))]).T
    m, c = np.linalg.lstsq(A, d, rcond=None)[0]
    print(f"m = {m}, c = {c}")

    fig, axs = plt.subplots(1, 1)
    axs.plot(data0)
    axs.set_ylabel("Avg buffer fill level")
    axs.set_xlabel("index")
    fig.tight_layout()
    plt.savefig(plotname)
    if showplot == True:
        plt.show()


# Usage: python ../plot_csv.py <log> <plotname> <1 or 2>

if __name__ == "__main__":
    fname = sys.argv[1]
    plotname = sys.argv[2]
    num_cols = int(sys.argv[3])

    if num_cols == 1:
        plot_1_cols(fname, plotname)
    elif num_cols == 2:
        plot_2_cols(fname, plotname)
    elif num_cols == 3:
        plot_3_cols(fname)
