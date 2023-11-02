# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re
import matplotlib.pyplot as plt
import numpy as np
import os
import argparse


def plot_3_cols(fname, plotname, showplot):
    data0 = []
    data1 = []
    data2 = []
    with open(fname) as fl:
        #reader = csv.reader(fl, delimiter=',')
        for row in fl.readlines():
            if m := re.match(r'(-?[0-9]+),(-?[0-9]+),(-?[0-9]+)', row):
                data0.append(int(m.group(1)))
                data1.append(int(m.group(2)))
                data2.append(int(m.group(3)))

    d = np.array(data1)
    x = [i for i in range(len(d))]
    A = np.vstack([x, np.ones(len(x))]).T
    m, c = np.linalg.lstsq(A, d, rcond=None)[0]
    print(f"m = {m}, c = {c}")

    fig, axs = plt.subplots(3, 1)
    axs[0].plot(data0)
    axs[1].plot(data1)
    axs[2].plot(data2)
    fig.tight_layout()
    plt.savefig(plotname)
    if showplot == True:
        plt.show()


def plot_2_cols(fname, plotname, showplot):
    data0 = []
    data1 = []
    with open(fname) as fl:
        #reader = csv.reader(fl, delimiter=',')
        for row in fl.readlines():
            if m := re.match(r'(-?[0-9]+),(-?[0-9]+)', row):
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



def plot_1_cols(fname, plotname, showplot):
    data0 = []
    with open(fname) as fl:
        #reader = csv.reader(fl, delimiter=',')
        for row in fl.readlines():
            if m := re.match(r'(-?[0-9]+)', row):
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

def get_args():
    parser = argparse.ArgumentParser("Script to plot the stdout when running the usb_in_i2s_out or i2s_in_usb_out application")
    parser.add_argument("input_file", type=str, help="stdout when running the usb_in_i2s_out or i2s_in_usb_out application dumped into a log file")
    parser.add_argument("num_cols", type=int, help="No. of comma separated entries per line in the stdout of the usb_in_i2s_out or i2s_in_usb_out application run")
    parser.add_argument("--plotfile", "-p", type=str, help="filename to save the output column plot in", default="plot_csv_cols.png")
    parser.add_argument("--show", "-s", action="store_true", help="Show the plot")
    return parser.parse_args()

if __name__ == "__main__":
    args = get_args()
    if os.path.isfile(args.input_file):
        if args.num_cols == 1:
            plot_1_cols(args.input_file, args.plotfile, args.show)
        elif args.num_cols == 2:
            plot_2_cols(args.input_file, args.plotfile, args.show)
        elif args.num_cols == 3:
            plot_3_cols(args.input_file, args.plotfile, args.show)
        else:
            assert False, f"Invalid number of colums {args.num_cols} provided. Valid values: 1, 2 or 3"
    else:
        assert False, f"Invalid input file {args.input_file}"
