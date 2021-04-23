import numpy as np
import c2py
import heartpy as hp

example = [480.0, 800.0, 800.0, 800.0, 800.0, 680.0,
           760.0, 760.0, 480.0, 480.0, 880.0, 560.0, 920.0]
rrs = c2py.float_array(13)
for i in range(13):
    rrs[i] = float(example[i])

c2py.CalcBreathRate(rrs, 13, 25)
