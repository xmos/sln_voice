#!/bin/bash
cmake -S . -B ./build
cmake --build build --target usb_in_i2s_out
cmake --build build --target i2s_in_usb_out


dir_name=_plots
mkdir -p $dir_name

# Without SOF timestamps file
build/usb_in_i2s_out 88200 2>&1 > log
python calc_snr.py asrc_output.bin 88200
build/i2s_in_usb_out 192000 2>&1 > log
python calc_snr.py asrc_output.bin 48000

# With SOF timestamps file
build/usb_in_i2s_out 176400 log_sofs_1hr 2>&1 > log
python calc_snr.py asrc_output.bin 176400
build/i2s_in_usb_out 96000 log_sofs_1hr 2>&1 >  log
python calc_snr.py asrc_output.bin 48000

#python plot_csv.py log $dir_name/test_correct_$i.png 2
