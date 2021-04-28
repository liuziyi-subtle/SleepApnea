import numpy as np
import c2py
import heartpy as hp
import matplotlib.pyplot as plt

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

fig, axes = plt.subplots(2, 1, figsize=(15, 10))
axes[1].plot(y_interp)
axes[2].plot(y_interp_f)
plt.show()
