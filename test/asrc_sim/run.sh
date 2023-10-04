#!/bin/bash
dir_name=_plots_usb_in_i2s_out_with_correction
mkdir -p $dir_name

for i in 192000 176400 96000 88200 48000 44100
    do
    build/usb_in_i2s_out $i log_sofs_1hr 2>&1 > log
    python plot_csv.py log $dir_name/test_correct_$i.png 2
    done
