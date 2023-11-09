

#include <Python.h>

#include <stdio.h>
#include <limits.h>
#include <cstring>
#include <pthread.h>
// #include "contrib/argtable3.h"

// using namespace std;

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

// #include "heap.cpp"
// #include "linked_list.hpp"
// #include "nngraph.hpp"
// #include "util.hpp"

// #include "balkmeans_options.h"

// #include "timer.h"
// #include "util.cpp"
// #include "linked_list.cpp"
// #include "nngraph.cpp"
// #include "graphclu.h"

// #include "graphclu_lib.cpp"

using namespace std;

struct stat2 {
  int num_calc_clu_dist;
  int num_pruned;
};

int g_use_heap = 0;
struct stat2 g_stat;

void print_stat() {
  printf("STAT num_calc_clu_dist=%d num_pruned=%d\n", g_stat.num_calc_clu_dist, g_stat.num_pruned);
}

#include <Python.h>
#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include <math.h>
#include <numpy/arrayobject.h>
// #ifdef Py_PYTHON_H
// #include "rknng_lib.h"
#include <stdio.h>
// #include "rknng/rknng_lib.h"
// #include "dencl/dencl.hpp"

// using namespace std;

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

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

  // int numnodes = 0;
  // for (int i = 0; i < lsize; i++) {
  // PyObject *x = PyList_GetItem((PyObject *)py_v, i);
  // PyObject *idA = PyList_GetItem((PyObject *)x, 0);
  // PyObject *idB = PyList_GetItem((PyObject *)x, 1);
  // int a = PyLong_AsLong(idA);
  // int b = PyLong_AsLong(idB);
  // if (numnodes < a) {
  // numnodes = a;
  // }
  // if (numnodes < b) {
  // numnodes = b;
  // }
  // }
  // numnodes = numnodes + 1; // Indexes are from 0 to (N-1)
  // printf("nodes=%d\n", numnodes);

  // nnGraph *graph = init_nnGraph(numnodes);

  // for (int i = 0; i < lsize; i++) {
  // PyObject *x = PyList_GetItem((PyObject *)py_v, i);
  // PyObject *idA = PyList_GetItem((PyObject *)x, 0);
  // PyObject *idB = PyList_GetItem((PyObject *)x, 1);
  // PyObject *weight = PyList_GetItem((PyObject *)x, 2);
  // int a = PyLong_AsLong(idA);
  // int b = PyLong_AsLong(idB);
  // float w = PyFloat_AsDouble(weight);
  // // nng_add_neighbor_safe(graph, a, b, w);
  // // nng_add_neighbor_safe(graph, b, a, w);

  // if (!nng_has_neighbor(graph, a, b)) {
  // nng_add_neighbor(graph, a, b, w);
  // }

  // if (!nng_has_neighbor(graph, b, a)) {
  // nng_add_neighbor(graph, b, a, w);
  // }
  // }

  // Clustering *clu;
  // init_Clustering(&clu, graph->size, num_clusters);

  // if (g_opt.costf <= 3) {
  // g_opt.minimize = 1;
  // g_opt.costmultip = -1.0;
  // } else {
  // g_opt.minimize = 0;
  // g_opt.costmultip = 1.0;
  // }

  // if (g_opt.scale == 1) {
  // if (g_opt.graph_type == DISTANCE) {
  // scale_weights(graph, 1);
  // } else { // SIMILARITY
  // scale_weights(graph, 2);
  // }
  // } else {
  // // No scale
  // scale_weights(graph, -1);
  // }

  // g_timer.tick();
  // if (seed == 0) {
  // seed = time(NULL);
  // }
  // g_opt.seed = seed;
  // rand_seed(seed);

  // m_algo(graph, clu, repeats, num_clusters);

  // for (int i = 0; i < clu->N; i++) {
  // clu->part[i] += 1;
  // }
  // py_labels = array_to_py(clu->part, clu->N);
  int arr[20] = {1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 2, 2, 2};
  py_labels = array_to_py(arr, 10);

  std::cout << "Finnished running algorithm\n";

  return py_labels;
}

extern "C" {

static PyObject *balkmeans_py(PyObject *self, PyObject *args, PyObject *kwargs);

// Define python accessible methods
static PyMethodDef BalkmeansMethods[] = {
    {"balkmeans", balkmeans_py, METH_VARARGS | METH_KEYWORDS, "Balanced k-means clustering"},
    {NULL, NULL, 0, NULL}};

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

/* This initiates the module using the above definitions. */
// #if PY_VERSION_HEX >= 0x03000000
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
  import_array();
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

  // g_opt.repeats = 0;
  // g_opt.dissolve = 1;
  // g_opt.clusters = 10;
  // g_opt.graph_type = DISTANCE;
  // g_opt.debug = 0;
  // g_opt.grow_factor = 0.8;
  // g_opt.grow_factor_start = 1.0;
  // g_opt.grow_factor_end = 1.0;
  // g_opt.verbose = 1;
  // g_opt.costf = 1;    // 1=cond, 2=inv
  // g_opt.minimize = 0; // maximize by default
  // g_opt.debug_save_intermediate_part = 0;
  // g_opt.density_method = 1;
  // g_opt.max_iter = 200;
  // g_opt.scale = 1;

  PyObject *ret;
  static char *kwlist[] = {"v",     "num_clusters", "repeats", "costf",
                           "scale", "verbose",      "seed",    NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|issii", kwlist, &PyList_Type, &py_v,
                                   &o_num_clusters, &o_repeats, &o_costf, &o_scale, &o_verbose,
                                   &o_seed)) {
    return NULL;
  }

  // g_opt.verbose = o_verbose;
  // g_opt.repeats = o_repeats;

  // if (strcmp("no", o_scale) == 0) {
  // g_opt.scale = 0;
  // }

  // if (strcmp("cond", o_costf) == 0) {
  // g_opt.costf = COND;
  // } else if (strcmp("inv", o_costf) == 0) {
  // g_opt.costf = INV;
  // } else if (strcmp("meanw", o_costf) == 0) {
  // g_opt.costf = MEANW;
  // }

  // printf("COSFSFSF=%d\n", g_opt.costf);

  // if (strcmp("similarity", o_graph_type) == 0) {
  // g_opt.graph_type = SIMILARITY;
  // } else if (strcmp("distance", o_graph_type) == 0) {
  // g_opt.graph_type = DISTANCE;
  // } else {
  // PyErr_SetString(PyExc_ValueError, "graph_type must be either 'similarity' (larger weight means
  // " "closer) or 'distance' (smaller weight means closer)}");
  // return NULL;
  // }

  ret = py_balkmeans(py_v, NULL, NULL, NULL);

  return ret;
}

} // END extern "C"
