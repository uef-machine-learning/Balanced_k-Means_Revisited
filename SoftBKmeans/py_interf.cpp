

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

// #define v(x0, x1)                                                                                  \
  // (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     // (x1)*PyArray_STRIDES(py_v)[1])))
// #define v_shape(i) (py_v->dimensions[(i)])

PyObject *array_to_py(int *arr, int N) {
  // Convert c array to python format
  // printf("array_to_py size=%d\n", N);
  PyObject *pyarr = PyList_New(N);
  for (int i = 0; i < N; i++) {
    PyList_SetItem(pyarr, i, Py_BuildValue("i", arr[i]));
  }
  return pyarr;
}

PyObject *py_balkmeans(PyListObject *py_v, int num_clusters, int repeats, int seed) {

  PyObject *py_labels;

  int lsize = PyList_Size((PyObject *)py_v);
  PyObject *x = PyList_GetItem((PyObject *)py_v, 0);
  int dim = PyList_Size(x);
  printf("size=%d dim=%d\n", lsize, dim);

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
  // kMeans.initialize(vec, numClu, rand());
  kMeans.initialize(vec, 15, 232323);
  KMeans::TerminationCriterion terminationCriterion =
      KMeans::TerminationCriterion::MaxDiffClusterSizes;

  int maxIter = 100000;
  double terminationCriterionValue = 1.0;
  bool stopWhenBalanced = false;
  double partlyRemainingFraction = 0.15; // 0 < partlyremainingFraction < 1
  double increasingPenaltyFactor = 1.01; // increasingPenaltyFactor >= 1
  bool useFunctionIter = true;
  int switchPostp = 10;
  bool reproducible = false;

  // auto startTime = std::chrono::high_resolution_clock::now();
  kMeans.run(terminationCriterion, terminationCriterionValue, stopWhenBalanced,
             partlyRemainingFraction, increasingPenaltyFactor, useFunctionIter, switchPostp,
             maxIter);
  int* labels = kMeans.getLabels();
  py_labels = array_to_py(labels,kMeans.size);

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
  int num_clusters = 10;
  char *type = NULL;

  int o_repeats = 100;
  int o_num_clusters = 20;
  char *o_graph_type;
  char *o_costf;
  char *o_scale;
  int o_verbose = 0;
  int o_seed = 0;

  PyObject *ret;
  static char *kwlist[] = {"v",     "num_clusters", "repeats", "costf",
                           "scale", "verbose",      "seed",    NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|issii", kwlist, &PyList_Type, &py_v,
                                   &o_num_clusters, &o_repeats, &o_costf, &o_scale, &o_verbose,
                                   &o_seed)) {
    return NULL;
  }


  ret = py_balkmeans(py_v, NULL, NULL, NULL);

  return ret;
}

} // END extern "C"
