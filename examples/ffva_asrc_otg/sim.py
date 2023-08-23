import salabim as sim
sim.yieldless(False)
import matplotlib.pyplot as plt
import numpy as np

class Asrc():
    def __init__(self, fs_in, fs_out, block_size):
        self.fs_in = fs_in
        self.fs_out = fs_out
        self.block_size = block_size
        self.nominal_rate_ratio = self.fs_in / self.fs_out
        self.fractional_samples_accum = 0

    def process_frame(self, rate_ratio):
        num_samples_out_f = self.block_size/rate_ratio
        num_samples_out_i = int(num_samples_out_f)
        self.fractional_samples_accum = self.fractional_samples_accum + (num_samples_out_f - num_samples_out_i)
        if(self.fractional_samples_accum >= 1):
            self.fractional_samples_accum -= 1
            num_samples_out_i = num_samples_out_i + 1
        return num_samples_out_i
            
        

class Buffer():
    def __init__(self, total_size):
        self.num_samples_written = 0
        self.num_samples_read = 0
        self.total_size = total_size

    def size(self):
        return self.total_size

    def write(self, n):
        self.num_samples_written = self.num_samples_written + n

    def read(self, n):
        self.num_samples_read = self.num_samples_read + n

    def fill_level(self):
        unread = self.num_samples_written - self.num_samples_read
        return unread - (self.total_size/2)

class Producer(sim.Component):
    def __init__(self, buffer, actual_rate_ratio, input_period, nominal_input_rate, nominal_output_rate, input_block_size):
        self.num_write_calls = 0
        self.buffer = buffer
        self.buf_fill_levels = []
        self.fs_in = nominal_input_rate
        self.fs_out = nominal_output_rate
        self.block_size = input_block_size
        self.asrc = Asrc(self.fs_in, self.fs_out, self.block_size)
        self.nominal_rate_ratio = self.fs_in / self.fs_out 
        self.actual_rate_ratio = actual_rate_ratio - 5e-7
        self.input_period = input_period
        super().__init__()

    def process(self):
        while True:
            num_samples = self.asrc.process_frame(self.actual_rate_ratio)
            self.buffer.write(num_samples)
            self.num_write_calls = self.num_write_calls + 1
            if self.num_write_calls % 16 == 0:
                self.buf_fill_levels.append(self.buffer.fill_level())
            yield self.hold(self.input_period)

class Consumer(sim.Component):
    def __init__(self, buffer, sof_period):
        self.num_read_calls = 0
        self.buffer = buffer
        self.sof_period = sof_period
        super().__init__()
    def process(self):
        while True:
            self.buffer.read(48)
            self.num_read_calls = self.num_read_calls + 1 
            yield self.hold(self.sof_period)

env = sim.Environment(trace=False)
usb_freq_ppm = 8.34565224
nominal_usb_rate = 48000
actual_usb_rate = nominal_usb_rate + ((usb_freq_ppm * nominal_usb_rate)/1000000)
sof_period = 1/(actual_usb_rate / nominal_usb_rate)


input_block_size = 240
nominal_i2s_rate = 192000
actual_i2s_rate = nominal_i2s_rate 
input_period = ((input_block_size/nominal_i2s_rate)*1000) / (actual_i2s_rate /nominal_i2s_rate)
actual_rate_ratio_f = actual_i2s_rate / actual_usb_rate
actual_rate_ratio_fp_to_f =  ((int)(actual_rate_ratio_f * (1<<28)))/(1<<28)
print(f"actual_usb_rate = {actual_usb_rate}, actual_rate_ratio = {actual_rate_ratio_fp_to_f}, input_period = {input_period}, output_period = {sof_period}")


buf = Buffer(input_block_size*4)
buf.write(buf.size()/2)
print(f"At the start of sim, fill level = {buf.fill_level()}")
p = Producer(buf, actual_rate_ratio_fp_to_f, input_period, nominal_i2s_rate, nominal_usb_rate, input_block_size)
c = Consumer(buf, sof_period)
test_time_s = 60*60
env.run(till=1000*test_time_s)

plt.plot(p.buf_fill_levels)
plt.show()

print(f"After SIM. written = {buf.num_samples_written}")
print(f"After SIM. read = {buf.num_samples_read}")
print(f"After SIM. num_write_calls = {p.num_write_calls}")
print(f"After SIM. num_read_calls = {c.num_read_calls}")
print(f"After SIM. fill_level = {buf.fill_level()}")

data = np.array(p.buf_fill_levels, dtype=np.float64)
windowed_avgs = []
windowed_avg_snapshot = 0
max_val = -1000
max_val_snapshot = 0
max_over_a_window = []

min_val = 1000
min_val_snapshot = 0
min_over_a_window = []

accum = 0
count = 0
window_len = 4096
for d in data:
    count += 1
    accum = accum + d
    if d > max_val:
        max_val = d
    if d < min_val:
        min_val = d

    if(count == window_len):
        windowed_avg_snapshot = accum / window_len
        count = 0
        accum = 0
        max_val_snapshot = max_val
        max_val = -1000
        min_val_snapshot = min_val
        min_val = 1000
    windowed_avgs.append(windowed_avg_snapshot)
    max_over_a_window.append(max_val_snapshot) 
    min_over_a_window.append(min_val_snapshot) 

#plt.plot(windowed_avgs)
#plt.plot(max_over_a_window)
#plt.plot(min_over_a_window)
plt.plot((np.array(max_over_a_window) + np.array(min_over_a_window))/2)
plt.show()
