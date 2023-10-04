import sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile

def rawFFT(data, sampling_rate):
    data_len = len(data)
    print(f"data_len = {data_len}")

    ignoreSamples = 128

    f = open("test.dat", "w")
    for d in data[-ignoreSamples:]:
        f.write(f"{str(d)},\n")
    f.close()
    #print(data[-ignoreSamples:])



    #Data = np.fft.rfft(data[-ignoreSamples:])
    skip = int(15*60*sampling_rate)
    Data = np.fft.rfft(data[skip + ignoreSamples:skip + 2*ignoreSamples])
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

    #(S, f) = plt.psd(data[ignoreSamples:-ignoreSamples], NFFT=8192)

    #print(f)

    print(f"bin_resolution = {bin_resolution}")
    print(f"Max found on bin {max_index}, max amplitude {Data_abs[max_index]}")
    print(f"Frequency at the max bin = {bin_resolution * max_index}")

    plt.plot(Data_abs)
    plt.show()
    # fft assuming signal is already an integer fraction of the true output rate
    #min = np.argmin(np.abs(data[ignoreSamples:-(fftPoints)])) + ignoreSamples #investigating if picking a data set that looks like it is nearly zero crossing at the ends is better
    #samples = data[min:min+self.fftPoints]
    #l=self.fftPoints
    #fftData = self.mpAbs(np.fft.fft(samples))
    #fftDataDB = 20 * np.log10((fftData/np.max(fftData)) + self.fudge)
    #realRate = self.sampleRates[self.opRate] * float(self.fDev)
    #x = np.linspace(0,realRate/2, num=int(l/2) ) / 1000
    #return [x, fftDataDB[0:int(l/2)], fftData[0:int(l/2)]/np.max(fftData[0:int(l/2)])]
    return [0]

def doFFT(data, sampling_rate, window=False):
    # convinient way to select between fft styles.  Note that the periodic one will need a lot more samples, so
    # use window=True for debuging.
    return rawFFT(data, sampling_rate)

if __name__ == "__main__":
    if(len(sys.argv) != 3):
        assert False, "Wrong number of arguments given!!"
    dt = np.fromfile(sys.argv[1], dtype=np.int32)

    scipy.io.wavfile.write("test.wav", int(sys.argv[2]), dt.T)

    print(dt.shape)
    data = np.array(dt/(np.iinfo(np.int32).max), dtype=np.double)
    x = np.linspace(0, len(data[-1000:]), len(data[-1000:]))
    #plt.scatter(x, data[0:1000])
    #plt.plot(data)
    #plt.show()
    doFFT(data, int(sys.argv[2]))
