# Balanced k-Means Revisited

Algorithm:

R. de Maeyer, S. Sieranoja and P. Fränti, Balanced k-means revisited, Applied Computing and Intelligence, 2023, Volume 3, Issue 2: 145-179. doi: 10.3934/aci.2023008

https://aimspress.com/article/doi/10.3934/aci.2023008


# Compile and use on Linux

Compile:
```
cd SoftBKmeans
make
```

Run example (hard balance):
```
./bkmeans -k 15 --seed 7059488 -i datasets/s1.txt -o labels.pa -c centroids.txt --switch 30
```

Run example (soft balance), limiting maximum difference between sizes of any two partitions to 50:
```
./bkmeans -k 15 --seed 7059488 -i data/S1.txt -o labels.pa -c centroids.txt --switch 30 --maxdiff 50
```

Command line parameters:
```
Usage: bkmeans -i FILENAME [-k <n>] -o FILENAME -c FILENAME [--seed=INT] [--visualize] [--switch=INT] [--help] [--vizpref=PREFIX] [--iter=INT] [--maxdiff=DBL]
  -i FILENAME                    Input filename
  -k, --numclu=<n>               Number of clusters
  -o FILENAME                    Output partition filename
  -c FILENAME                    Output centroids filename
  --seed=INT                     Random number seed
  --visualize                    Visualize clustering results of 2D set.
  --switch=INT                   Delta switch postprocess iterations (default: 10)
  --help                         Help
  --vizpref=PREFIX               Prefix to use for visualization filename
  --iter=INT                     Limit maximum iterations
  --maxdiff=DBL                  Max difference between partition sizes (default: 1)
```


# Python interface

```
git clone https://github.com/uef-machine-learning/Balanced_k-Means_Revisited
cd Balanced_k-Means_Revisited/SoftBKmeans/
pip install .
./py_example.py
```



