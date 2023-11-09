#!/usr/bin/env python
import numpy as np

x=np.loadtxt('data/S1.txt').tolist()

from balkmeans import balkmeans
print(balkmeans(x,15))
# balkmeans(x,15)

