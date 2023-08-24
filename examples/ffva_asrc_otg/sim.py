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

class Rate_monitor():
    def __init__(self, nominal_rate_ratio, window_size):
        self.nominal_rate_ratio = nominal_rate_ratio
        self.window_size = window_size
        self.accum = 0
        self.count = 0
        self.max = -1000
        self.min = 1000
        self.window_avg = 0
        self.window_max = 0
        self.window_min = 0
        self.window_max_min_avg = 0

        self.prev_window_avg = 0
        self.prev_window_max = 0
        self.prev_window_min = 0
        self.prev_window_max_min_avg = 0
        self.avg_valid = False
        self.first_value_calculated = False

        self.window_count = 0
        self.prev_window_count = 0
        self.avg = 0
        self.arr_window_max = []
        self.arr_window_min = []
        self.arr_avg = []
        self.arr_window_avg = []

    def calc_avg_rate(self, current_fill_level):

        self.accum = self.accum + current_fill_level
        if current_fill_level > self.max:
            self.max = current_fill_level
        if current_fill_level < self.min:
            self.min = current_fill_level
        self.count += 1
        if self.count == self.window_size:
            #if self.first_value_calculated:
            #    self.avg_valid = True

            self.window_avg = self.accum / self.window_size
            self.window_max = self.max
            self.window_min = self.min
            self.window_max_min_avg = (self.window_max + self.window_min) / 2
            self.window_count += 1

            if self.prev_window_max_min_avg != self.window_max_min_avg:
                if self.first_value_calculated: 
                    self.avg = (self.window_max_min_avg - self.prev_window_max_min_avg) / (self.window_count - self.prev_window_count)
                    self.avg_valid = True
                    print(f'wavg={self.window_max_min_avg}, pwavg={self.prev_window_max_min_avg}, wc={self.window_count}, pwc={self.prev_window_count}, avg={self.avg}')

                self.prev_window_count = self.window_count
                self.prev_window_avg = self.window_avg
                self.prev_window_max = self.window_max
                self.prev_window_min = self.window_min
                self.prev_window_max_min_avg = self.window_max_min_avg   
            self.count = 0
            self.accum = 0
            self.max = -1000
            self.min = 1000
            self.first_value_calculated = True
            self.arr_window_max.append(self.window_max)
            self.arr_window_min.append(self.window_min)
            self.arr_avg.append(self.avg)
            self.arr_window_avg.append(self.window_max_min_avg)

        return self.avg_valid, self.window_avg, self.avg

        
    
class Producer(sim.Component):
    def __init__(self, buffer, actual_rate_ratio, input_period, nominal_input_rate, nominal_output_rate, input_block_size):
        self.num_write_calls = 0
        self.buffer = buffer
        self.buf_fill_levels = []
        self.avg_buf_fill_levels = []
        self.diff_levels = []
        self.arr_error = []
        self.fs_in = nominal_input_rate
        self.fs_out = nominal_output_rate
        self.block_size = input_block_size
        self.asrc = Asrc(self.fs_in, self.fs_out, self.block_size)
        self.nominal_rate_ratio = self.fs_in / self.fs_out 

        self.rate_monitor = Rate_monitor(self.nominal_rate_ratio, 8192)
        self.actual_rate_ratio = actual_rate_ratio - 4e-7
        self.input_period = input_period
        self.rate_ratio = self.actual_rate_ratio
        super().__init__()
    
    def pi_controller(self, avg_level, diff):
        self.Kd = 270
        error = self.Kd*diff
        if error > 250:
            error = 250
        elif error < -250:
            error = -250
        return error

    def process(self):
        while True:
            num_samples = self.asrc.process_frame(self.rate_ratio)
            self.buffer.write(num_samples)
            self.num_write_calls = self.num_write_calls + 1
            if self.num_write_calls % 16 == 0:
                current_fill_level = self.buffer.fill_level()
                self.buf_fill_levels.append(current_fill_level)
                valid, avg_buf_level, diff_level = self.rate_monitor.calc_avg_rate(current_fill_level)

                if valid:
                    error = self.pi_controller(avg_buf_level, diff_level)
                    self.avg_buf_fill_levels.append(avg_buf_level)
                    self.diff_levels.append(diff_level)
                    self.arr_error.append(error)
                    self.rate_ratio = self.actual_rate_ratio + (error / (1<<28))                   
                    #print(self.rate_ratio)
                

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
test_time_s = 120 * 60
env.run(till=1000*test_time_s)

#plt.plot(p.buf_fill_levels)
#plt.show()

print(f"After SIM. written = {buf.num_samples_written}")
print(f"After SIM. read = {buf.num_samples_read}")
print(f"After SIM. num_write_calls = {p.num_write_calls}")
print(f"After SIM. num_read_calls = {c.num_read_calls}")
print(f"After SIM. fill_level = {buf.fill_level()}")

#plt.plot(p.diff_levels)
#plt.show()

fig, axs = plt.subplots(2, 2)
axs[0,0].plot(p.buf_fill_levels)
axs[1,0].plot(p.rate_monitor.arr_avg)
axs[0,1].plot(p.rate_monitor.arr_window_avg)
axs[0,1].plot(p.rate_monitor.arr_window_max )
axs[0,1].plot(p.rate_monitor.arr_window_min )
axs[1,1].plot(p.arr_error )
fig.tight_layout()
plt.show()


