#include <string>
#include <fstream>
#include <iomanip>
#include <random>
#include <iostream>
#include <chrono>

#include <sstream>
#include <vector>
#include <fstream>

#include "contrib/argtable/argtable3.h"

using std::cout;
using std::string;
using std::vector;

#include "kMeans.h"
// using namespace std;

vector<int> readIntVec(string fname) {
  vector<int> vect;
  std::fstream f;
  f.open(fname, std::ios::in);
  if (f.fail()) {
    cout << "File open failed:" << fname << "\n";
    exit(0);
  }
  int value;
  while (f >> value) {
    vect.push_back(value);
  }
  f.close();
  return vect;
}

template <class TYPE> vector<vector<TYPE>> load2DVec(string fn) {
  vector<vector<TYPE>> vec;

  std::ifstream source;
  source.open(fn, std::ios_base::in);
  if (source.fail()) {
    cout << "Failed to open file: " << fn << "\n";
    return vec;
  }
  for (std::string line; std::getline(source, line);) {
    vector<TYPE> v2;
    std::istringstream in(line);
    TYPE item;
    while (in >> item) {
      v2.push_back(item);
    }
    vec.push_back(v2);
  }
  return vec;
}

int main(int argc, char *argv[]) {

  const char *progname = "bkmeans";
  int ok;

  struct arg_int *a_numClu =
      arg_intn("k", "numclu", "<n>", 0 /*mincount*/, 1 /*maxcount*/, "Number of clusters");
  struct arg_file *a_infn = arg_filen("i", NULL, "FILENAME", 1, 1, "Input filename");
  struct arg_file *a_partfn = arg_filen("o", NULL, "FILENAME", 1, 1, "Output partition filename");
  struct arg_file *a_cntfn = arg_filen("c", NULL, "FILENAME", 1, 1, "Output centroids filename");
  struct arg_str *a_vizprefix =
      arg_str0(NULL, "vizpref", "PREFIX", "Prefix to use for visualization filename");
  struct arg_int *a_seed = arg_intn(NULL, "seed", "INT", 0, 1, "Random number seed");
  struct arg_int *a_switch = arg_intn(NULL, "switch", "INT", 0, 1, "Delta switch postprocess iterations (default: 10)");
  struct arg_int *a_iter = arg_intn(NULL, "iter", "INT", 0, 1, "Limit maximum iterations");
  struct arg_dbl *a_termcv = arg_dbln(NULL, "maxdiff", "DBL", 0, 1, "Max difference between partition sizes (default: 1)");

  struct arg_lit *a_viz =
      arg_litn(NULL, "visualize", 0, 1, "Visualize clustering results of 2D set.");
  struct arg_lit *a_help = arg_litn(NULL, "help", 0, 1, "Help");

  struct arg_end *a_end = arg_end(20);
  void *argtable[] = {a_infn,   a_numClu, a_partfn,    a_cntfn, a_seed, a_viz,
                      a_switch, a_help,   a_vizprefix, a_iter, a_termcv, a_end};
  int exitcode = 0;
  int nerrors;
  int numClu = 64;

  // value == 1 for hard balance, value > 1 for soft balance 
  double terminationCriterionValue = 1.0;
  bool stopWhenBalanced = false;
  double partlyRemainingFraction = 0.15; // 0 < partlyremainingFraction < 1
  double increasingPenaltyFactor = 1.01; // increasingPenaltyFactor >= 1
  bool useFunctionIter = true;
  bool reproducible = false;

  if (arg_nullcheck(argtable) != 0) {
    /* NULL entries were detected, some allocations must have failed */
    printf("%s: insufficient memory\n", progname);
    // exitcode=1;
    // goto exit;
  }

  nerrors = arg_parse(argc, argv, argtable);

  if (a_help->count > 0) {
    printf("Usage: %s", progname);
    arg_print_syntax(stdout, argtable, "\n");
    // printf("Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n\n");
    arg_print_glossary(stdout, argtable, "  %-30s %s\n");
    // exitcode=0;
    return 0;
  }

  if (nerrors > 0) {
    /* Display the error details contained in the arg_end struct.*/
    arg_print_errors(stdout, a_end, progname);
    printf("Try '%s --help' for more information.\n", progname);
    exitcode = 1;
    // goto exit;
  }

  if (a_infn->count > 0) {
  } else {
    printf("No infile\n");
    ok = 0;
  }

  if (a_numClu->count > 0) {
    numClu = a_numClu->ival[0];
  }
  
  
  if (a_termcv->count > 0) {
    terminationCriterionValue = a_termcv->dval[0];
  }

  int switchPostp = 10;
  if (a_switch->count > 0) {
    switchPostp = a_switch->ival[0];
  }

    KMeans::TerminationCriterion terminationCriterion =
        KMeans::TerminationCriterion::MaxDiffClusterSizes;
    // the value that should be reached by the termination criterion
    // a balanced clustering corresponds to maximum difference in cluster sizes as termination
    // // criterion and terminationCriterion = 1, balanced is not 0!

  cout << "infn=" << a_infn->filename[0] << " partfn=" << a_partfn->filename[0] << " k=" << numClu
       << "\n";

  int dimension, size, numClusters;

  auto vec = load2DVec<double>(a_infn->filename[0]);
  dimension = vec[0].size();
  size = vec.size();

  int maxIter = 100000;

  // set random seeds for the runs
  if (reproducible) {
    srand(7843); // any other int can be chosen
  } else {
    srand((int)time(NULL));
  }

  if (a_iter->count > 0) {
    maxIter = a_iter->ival[0];
  }

  if (a_seed->count > 0) {
    srand(a_seed->ival[0]);
  }

  printf("Parameters: terminationCriterionValue=%f partlyRemainingFraction=%f increasingPenaltyFactor=%f maxIter=%d\n",terminationCriterionValue, partlyRemainingFraction, increasingPenaltyFactor, maxIter);
  
  int run = 0;
  KMeans kMeans;
  kMeans.initialize(vec, numClu);
  // solve the kMeans instance
  auto startTime = std::chrono::high_resolution_clock::now();
  kMeans.run(terminationCriterion, terminationCriterionValue, stopWhenBalanced,
             partlyRemainingFraction, increasingPenaltyFactor, useFunctionIter, switchPostp,
             maxIter);
  auto endTime = std::chrono::high_resolution_clock::now();
  double time =
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
  // save assignments in file
  // std::string filename =  std::to_string(run) + end;

  if (a_partfn->count > 0) {
    std::fstream partf;
    partf.open(a_partfn->filename[0], std::ios::out);
    kMeans.writeAssignments(partf);
    partf.close();
  }
  if (a_cntfn->count > 0) {
    std::fstream centroidf;
    centroidf.open(a_cntfn->filename[0], std::ios::out);
    kMeans.writeCentroids(centroidf);
    centroidf.close();
  }
  cout << "time=" << time << " SSE=" << kMeans.sumOfSquaredErrors()
       << " MSE=" << kMeans.meanSquaredError() << "\n";

  if (a_viz->count > 0) {
    kMeans.showResultsConvexHull2("TODO", 0, time);
  }

  if (a_vizprefix->count > 0) {
    kMeans.showResultsConvexHull2(a_vizprefix->sval[0], 0, time);
  }

  return 0;
}
