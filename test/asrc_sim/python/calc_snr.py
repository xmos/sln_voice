# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile
import argparse
import os

def rawFFT(data, sampling_rate, plot_fname, show_plot):
    data_len = len(data)
    print(f"data_len = {data_len}")

    fftLength = 128

    skip = int(15*60*sampling_rate) # Skip 15mins from the beginning
    Data = np.fft.rfft(data[skip + fftLength:skip + 2*fftLength])
    Data1 = np.abs(Data)
    m = np.argmax(Data1)
    tmpNoise=np.copy(Data1)
    tmpNoise[m] = 0
    noise_power = np.sum(tmpNoise)
    signal_power = Data1[m]
    snr = 20*(np.log10(signal_power/noise_power))
    print(f"SNR = {snr}")

    Data_abs = 20 * np.log10(Data1/np.max(Data1))
    max_index = np.argmax(Data_abs)


    x = np.linspace(0,sampling_rate/2, len(Data_abs))
    bin_resolution = (sampling_rate/2)/(len(Data_abs))

    ignoreSamples = 128
    #(S, f) = plt.psd(data[ignoreSamples:-ignoreSamples], NFFT=8192)
    #print(f)

    print(f"bin_resolution = {bin_resolution} Hz")
    print(f"Max found on bin {max_index}, max amplitude {Data_abs[max_index]}")
    print(f"Frequency at the max bin = {bin_resolution * max_index} Hz")

    plt.plot(Data_abs)
    plt.ylabel("Magnitude(dB)")
    plt.xlabel("bins")
    plt.savefig(plot_fname)
    if show_plot:
        plt.show()

    return snr


def get_args():
    parser = argparse.ArgumentParser("Script to plot FFT spectrum and calculate SNR")
    parser.add_argument("input_file", type=str, help="asrc input or output bin file, dumped when running usb_in_i2s_out or i2s_in_usb_out ASRC sim application")
    parser.add_argument("sampling_rate", type=int, help="sampling rate for the input file specified in the first argument")
    parser.add_argument("--plotfile", "-p", type=str, help="filename to save the FFT magnitude spectrum plot in", default="plot_spectrum.png")
    parser.add_argument("--show", "-s", action="store_true", help="Show the plot")
    return parser.parse_args()

# Usage python calc_snr.py <asrc_output.bin> <sampling rate>
if __name__ == "__main__":
    args = get_args()

    # Check if sampling rate is valid
    if args.sampling_rate not in [192000, 96000, 48000, 176400, 88200, 44100]:
        assert False, f"ERROR: Invalid sampling rate {args.sampling_rate} specified"

    #Check if file exists
    if os.path.isfile(args.input_file):
        dt = np.fromfile(args.input_file, dtype=np.int32)
        scipy.io.wavfile.write("test.wav", args.sampling_rate, dt.T)
        data = np.array(dt/(np.iinfo(np.int32).max), dtype=np.double)
        snr = rawFFT(data, args.sampling_rate, args.plotfile, args.show)
    else:
        assert False, f"Invalid input file {args.input_file}"
