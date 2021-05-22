from scipy import signal
import numpy as np
import c2py
import heartpy as hp
import matplotlib.pyplot as plt
from scipy.io import loadmat, savemat

example = [480.0, 800.0, 800.0, 800.0, 800.0, 680.0,
           760.0, 760.0, 480.0, 480.0, 880.0, 560.0, 920.0]
rrs = c2py.float_array(13)
for i in range(13):
    rrs[i] = float(example[i])

c2py.DebugInit()
c2py.CalcBreathRate(rrs, 13, 25)

a = c2py.GetPLOT()
x_interp = [a.getitem_x_interp(i) for i in range(a.x_interp_len)]
y_interp = [a.getitem_y_interp(i) for i in range(a.y_interp_len)]
y_interp_f = [a.getitem_y_interp_f(i) for i in range(a.y_interp_f_len)]

savemat("/Users/liuziyi/Desktop/y_interp.mat", {"y_interp": y_interp})

sos = signal.butter(2, [0.1, 0.4], 'bandpass', fs=1000, output='sos')
filtered = signal.sosfilt(sos, y_interp)
print(filtered[1294])

fig, axes = plt.subplots(3, 1, figsize=(15, 10))
axes[0].plot(y_interp)
axes[1].plot(y_interp_f)
axes[2].plot(filtered)

plt.show()
