# Balanced_k-Means_Revisited

Original readme file: _readme.txt

```
Algorithm:
R. de Maeyer and P. Fränti: "Balanced K-means revisited
submitted (coming soon)
```

The program is written in C++ and is a Microsoft Visual Studio project, so, if this program 
is used, the “SoftBKmeans.sln” file can directly be opened by Visual Studio. Otherwise, the 
header and source files can be found in the folder “SoftBKmeans”.

## Parameters (defined in main.cpp)

### nameDataSet, dimension, size, numClusters
Choosing the data set is mostly self-explanatory. The variables nameDataSet, dimension
and size refer to the name, the dimension and the size of the data set and the variable 
numClusters to the desired number of clusters. These numbers can be found in the folder 
“datasets”. For example, the file name “A2_2_5250_35.txt” means that the data set A2 is 
two-dimensional, has a size of 5250 and consists of 35 clusters. The sources of the data sets 
are http://cs.uef.fi/sipu/datasets/ and http://archive.ics.uci.edu/ml .
The data sets the algorithm uses are in the folder “data”. They contain the same data points 
as the data sets saved in the folder “datasets”, but in a different order. This is because the 
resulting clustering also depends on the order in which the algorithm reads the data points 
and many of the original data sets contained in the folder “datasets” are ordered according 
to the labels of the data points. So, we want to avoid that the algorithm is biased by this 
“perfect” order by shuffling the data points.

### terminationCriterion, terminationCriterionValue
The variable terminationCriterion determines which balance requirement will be used. 
The balance requirements MaxDiffClusterSizes (maximum difference in cluster sizes), 
MinClusterSize (minimum cluster size), MaxSDCS (maximum standard deviation in cluster sizes) 
and MinNormEntro (minimum normalized entropy) are implemented. More balance requirements can 
be implemented in the file “kmeans.cpp”.
The variable terminationCriterionValue describes the value against which the termination 
criterion is checked. For example, if MaxDiffClusterSizes and 10 are chosen, the balance 
requirement is reached if the maximum difference in cluster sizes is less or equal 10. If 
MinNormEntro and 0.999 are chosen, the balance requirement is reached if the normalized 
entropy is greater or equal 0.999.
Note, that a completely balanced clustering corresponds to a maximum difference in 
cluster sizes of 1, not 0. If 0 is chosen, the algorithm only terminates if the number of data 
points is divisible by the number of clusters.

### stopWhenBalanced
The boolean variable stopWhenBalanced is important for the point of termination. If it is 
set to true, the algorithm will terminate as soon as a clustering is found that satisfies the 
chosen balance requirement. If the balance requirement is not so strict, it can happen that 
the first clustering that satisfies this requirement is still optimizable with respect to both 
the clustering and the balance quality. If stopWhenBalanced is set to false, the algorithm will 
not return the first clustering satisfying the balance requirement, but will continue and return 
the clustering with the highest clustering quality satisfying the balance requirement that it can 
find. So, the running time increases a little bit, but also the quality of the returned solution 
becomes higher.

### partlyRemainingFraction, increasingPenaltyFactor, useFunctionIter
The effects of the variables partlyRemainingFraction, increasingPenaltyFactor and useFunctionIter 
are more complicated to describe and are explained in the paper. The partly remaining fraction must 
be chosen larger than 0 and smaller than 1, and the increasing penalty factor has to be larger or 
equal 1 and is only used if useFunctionIter is set to false. Reasonable values for these variables 
are 0.15 for the partly remaining fraction and 1.01 for the increasing penalty factor. It is  
recommended to set useFunctionIter to true, since – in most cases – it speeds up the 
algorithm and does not lead to a worse quality.

### numRuns
The variable numRuns denotes the number of times the algorithm should be applied to the 
data set. Since the algorithm includes randomness, for testing and comparing it to other 
algorithms it is useful to run the algorithm several times.

### reproducible
The resulting clustering depends on the initialization of the centroids, which happens 
randomly. Sometimes it is useful to get reproducible results, for example to study the impact 
of one variable separately. To get reproducible results the boolean variable reproducible
can be set to true. Then the generator of the random numbers will always be initialized by 
the same number.

### graphicalRepresentation
If this boolean variable is set to true and the data set is two-dimensional, png.-files are 
created in the folder “convexHulls” that show the data points and the clusters.

The assignments returned by the algorithm can be found in the folder “results”. For example, if the 
algorithm was applied to the data set vowel and the number of runs was set to 10, the files 
“vowel_assignments_0.txt” to “vowel_assignments_9.txt” contain the assignments of the 10 runs. 
The file “vowel_summary.txt” contains the 10 running times in seconds. Note that the files are 
overwritten if the algorithm is applied again to the same data set.
