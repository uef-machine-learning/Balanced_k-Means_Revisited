

#include <Python.h>

#include <stdio.h>
#include <limits.h>
#include <cstring>
#include <pthread.h>

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cfloat>
#include <cmath>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <errno.h>

using std::ios;
using std::sort;
using std::string;
using std::vector;

#include "lib_api.hpp"

#include <Python.h>
#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include <math.h>
#include <stdio.h>

PyObject *array_to_py(int *arr, int N) {
  // Convert c array to python format
  // printf("array_to_py size=%d\n", N);
  PyObject *pyarr = PyList_New(N);
  for (int i = 0; i < N; i++) {
    PyList_SetItem(pyarr, i, Py_BuildValue("i", arr[i]));
  }
  return pyarr;
}

PyObject *py_balkmeans(PyListObject *py_v, int o_num_clusters, float o_partly_remaining_factor,
                       float o_increasing_penalty_factor, int o_iter, float o_maxdiff,
                       int o_verbose, int o_seed, int switchPostp, int o_postprocess_iterations) {

  PyObject *py_labels;

  int lsize = PyList_Size((PyObject *)py_v);
  PyObject *x = PyList_GetItem((PyObject *)py_v, 0);
  int dim = PyList_Size(x);
  printf("size=%d dim=%d\n", lsize, dim);
  srand(o_seed);

  vector<vector<double>> vec;

  for (int i = 0; i < lsize; i++) {
    vector<double> v2;
    // printf("i=%d\n",i);
    PyObject *x = PyList_GetItem((PyObject *)py_v, i);
    if (PyList_Size(x) != dim) {
      std::cout << "Input dimensions not uniform";
      return 0;
    }
    for (int i_dim = 0; i_dim < dim; i_dim++) {
      PyObject *y = PyList_GetItem(x, i_dim);
      double item = PyFloat_AsDouble(y);
      // cout << item << " ";
      v2.push_back(item);
    }
    // cout << "\n";
    vec.push_back(v2);
  }

  std::cout << "Finnished loading data\n";

  KMeans kMeans;
  kMeans.initialize(vec, o_num_clusters);
  KMeans::TerminationCriterion terminationCriterion =
      KMeans::TerminationCriterion::MaxDiffClusterSizes;

  bool stopWhenBalanced = false;
  bool useFunctionIter = true;

  // auto startTime = std::chrono::high_resolution_clock::now();
  kMeans.run(terminationCriterion, o_maxdiff, stopWhenBalanced, o_partly_remaining_factor,
             o_increasing_penalty_factor, useFunctionIter, o_postprocess_iterations, o_iter);
  int *labels = kMeans.getLabels();
  py_labels = array_to_py(labels, kMeans.size);

  std::cout << "Finnished running algorithm\n";

  return py_labels;
}

extern "C" {

static PyObject *balkmeans_py(PyObject *self, PyObject *args, PyObject *kwargs);

// Define python accessible methods
static PyMethodDef BalkmeansMethods[] = {
    {"balkmeans", balkmeans_py, METH_VARARGS | METH_KEYWORDS, "Balanced k-means clustering"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT, "balkmeans", NULL, -1, BalkmeansMethods, NULL, NULL, NULL, NULL};

PyMODINIT_FUNC PyInit_balkmeans(void) {
  PyObject *m;
  m = PyModule_Create(&moduledef);
  if (!m) {
    return NULL;
  }
  return m;
}

static PyObject *balkmeans_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  PyListObject *py_v;
  int o_num_clusters = 20;
  float o_partly_remaining_factor = 0.15;
  float o_increasing_penalty_factor = 1.01;
  int o_iter = 100000;
  int o_maxdiff = 1; // Allow only a maximum size difference of 1 between largest and smallest clusters
  int o_verbose = 0;
  int o_seed = time(NULL);
  int switchPostp = 10;
  int o_postprocess_iterations = 50;

  PyObject *ret;
  static char *kwlist[] = {"v",
                           "num_clusters",
                           "partly_remaining_factor",
                           "increasing_penalty_factor",
                           "iter",
                           "maxdiff",
                           "verbose",
                           "seed",
                           "postprocess_iterations",
                           NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|ffiiiii", kwlist, &PyList_Type, &py_v,
                                   &o_num_clusters, &o_partly_remaining_factor,
                                   &o_increasing_penalty_factor, &o_iter, &o_maxdiff, &o_verbose,
                                   &o_seed, &o_postprocess_iterations)) {
    return NULL;
  }


  ret = py_balkmeans(py_v, o_num_clusters, o_partly_remaining_factor, o_increasing_penalty_factor,
                     o_iter, o_maxdiff, o_verbose, o_seed, switchPostp, o_postprocess_iterations);

  return ret;
}

} // END extern "C"
