#!/usr/bin/env python
import numpy as np
from sklearn import metrics
from balkmeans import balkmeans

x=np.loadtxt('datasets/s1.txt').tolist()
gt_labels = np.loadtxt('datasets/s1-gt.pa',dtype=np.int32,skiprows=4) 

labels = balkmeans(x,num_clusters=15,maxdiff=500,partly_remaining_factor=0.15,increasing_penalty_factor=1.01,seed=3393,postprocess_iterations=10)

print("NMI: %f" % (metrics.normalized_mutual_info_score(labels, gt_labels)))

