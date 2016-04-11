#!/usr/bin/python

import math
import numpy as np


x = np.array([10, 20, 30, 40, 50, 60, 70, 80])
y = np.array([68, 163, 258, 358, 454, 554, 633, 757])

A = np.vstack([x, np.ones(len(x))]).T
m, c = np.linalg.lstsq(A, y)[0]

print "y = " + str(m) + "x + " + str(c)
