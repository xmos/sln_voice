#!/bin/bash
cmake -S . -B ./build
cmake --build build --target usb_in_i2s_out -j8
cmake --build build --target i2s_in_usb_out -j8


dir_name=_plots
mkdir -p $dir_name

usbrate=48000

# Without SOF timestamps file
i2srate=88200
build/usb_in_i2s_out $i2srate 2>&1 > log
output=$(python calc_snr.py asrc_output.bin $i2srate -f $dir_name/plot_usb_in_i2s_out_$i2srate.png 2>&1)
snr=$(echo $output | sed -E 's/.*SNR = ([0-9]+).*/\1/g' | bc -l)
if [ $((snr)) -ge 120 ]; then
    echo "SNR $snr PASS"
else
    echo "SNR $snr FAIL"
    exit -1
fi

i2srate=192000
build/i2s_in_usb_out $i2srate 2>&1 > log
output=$(python calc_snr.py asrc_output.bin $usbrate -f $dir_name/plot_i2s_in_usb_out_$usbrate.png 2>&1)
snr=$(echo $output | sed -E 's/.*SNR = ([0-9]+).*/\1/g' | bc -l)
if [ $((snr)) -ge 120 ]; then
    echo "SNR $snr PASS"
else
    echo "SNR $snr FAIL"
    exit -1
fi

# With SOF timestamps file
i2srate=176400
build/usb_in_i2s_out $i2srate log_sofs_1hr 2>&1 > log
output=$(python calc_snr.py asrc_output.bin $i2srate -f $dir_name/plot_sof_usb_in_i2s_out_$i2srate.png 2>&1)
snr=$(echo $output | sed -E 's/.*SNR = ([0-9]+).*/\1/g' | bc -l)
if [ $((snr)) -ge 100 ]; then
    echo "SNR $snr PASS"
else
    echo "SNR $snr FAIL"
    exit -1
fi

i2srate=96000
build/i2s_in_usb_out $i2srate log_sofs_1hr 2>&1 >  log
output=$(python calc_snr.py asrc_output.bin $usbrate -f $dir_name/plot_sof_i2s_in_usb_out_$usbrate.png 2>&1)
snr=$(echo $output | sed -E 's/.*SNR = ([0-9]+).*/\1/g' | bc -l)
if [ $((snr)) -ge 100 ]; then
    echo "SNR $snr PASS"
else
    echo "SNR $snr FAIL"
    exit -1
fi

#python plot_csv.py log $dir_name/test_correct_$i.png 2
