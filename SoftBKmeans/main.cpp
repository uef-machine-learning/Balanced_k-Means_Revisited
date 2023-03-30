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

  struct arg_int *a_numClu = arg_intn("k", "numclu", "<n>", 0, 1, "Number of clusters");
  struct arg_file *a_infn = arg_filen("i", NULL, "FILENAME", 1, 1, "Input filename");
  struct arg_file *a_partfn = arg_filen("o", NULL, "FILENAME", 1, 1, "Output partition filename");
  struct arg_file *a_cntfn = arg_filen("c", NULL, "FILENAME", 1, 1, "Output centroids filename");
  // struct arg_rem  *dest     = arg_rem ("DEST|DIRECTORY", NULL);
  struct arg_end *a_end = arg_end(20);
  void *argtable[] = {a_infn, a_numClu, a_partfn, a_cntfn, a_end};
  int exitcode = 0;
  int nerrors;
  int numClu = 64;

  if (arg_nullcheck(argtable) != 0) {
    /* NULL entries were detected, some allocations must have failed */
    printf("%s: insufficient memory\n", progname);
    // exitcode=1;
    // goto exit;
  }

  nerrors = arg_parse(argc, argv, argtable);

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

  cout << "infn=" << a_infn->filename[0] << " partfn=" << a_partfn->filename[0] << " k=" << numClu
       << "\n";
  // return 0;

  // choose data set
  // std::string nameDataSet = "S2";
  // int dimension = 2;
  // int size = 5000;
  // int numClusters = 15;

  // std::string nameDataSet = "santa5000";
  // int dimension = 2;
  // int size = 5000;
  // int numClusters = 20;
  int dimension, size, numClusters;

  // std::string nameDataSet = "santa1.4M";
  // std::string infn = "data/santa1.4M.txt";

  std::string nameDataSet = "unbalance";
  std::string infn = "data/unbalance.txt";
  numClusters = 8;

  // int dimension = 2;
  // int size = 1437195;
  // auto vec = load2DVec<double>(infn);
  auto vec = load2DVec<double>(a_infn->filename[0]);
  dimension = vec[0].size();
  size = vec.size();

  // For soft balance:
  // setting parameters
  // the termination criterion to be used, already implemented are MaxDiffClusterSize,
  // MinClusterSize, MaxSDCS and MinNormEntro
  KMeans::TerminationCriterion terminationCriterion =
      KMeans::TerminationCriterion::MaxDiffClusterSizes;
  // the value that should be reached by the termination criterion
  // a balanced clustering corresponds to maximum difference in cluster sizes as termination
  // criterion and terminationCriterion = 1, balanced is not 0!
  double terminationCriterionValue = 50.0;

  // For hard balance:
  terminationCriterion = KMeans::TerminationCriterion::MaxDiffClusterSizes;
  terminationCriterionValue = 1.0;

  bool stopWhenBalanced = false;
  double partlyRemainingFraction = 0.15; // 0 < partlyremainingFraction < 1
  double increasingPenaltyFactor = 1.01; // increasingPenaltyFactor >= 1
  bool useFunctionIter = true;
  int numRuns = 100;
  bool reproducible = true;
  bool graphicalRepresentation = true;

  // set random seeds for the runs
  if (reproducible) {
    srand(7843); // any other int can be chosen
  } else {
    srand((int)time(NULL));
  }
  int *seeds = new int[numRuns];
  for (int i = 0; i < numRuns; i++) {
    seeds[i] = rand();
  }

  if (1) {
    int run = 0;
    KMeans kMeans;
    kMeans.initialize(vec, numClu, seeds[run]);
    // solve the kMeans instance
    auto startTime = std::chrono::high_resolution_clock::now();
    kMeans.run(terminationCriterion, terminationCriterionValue, stopWhenBalanced,
               partlyRemainingFraction, increasingPenaltyFactor, useFunctionIter);
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
    cout << "Time=" << time << " SSE=" << kMeans.sumOfSquaredErrors() << " MSE=" << kMeans.meanSquaredError() << "\n";
    
    // save times in summary file
    // summary.open(start + "_summary" + end, std::fstream::app);
    // summary << time << std::endl;
    // summary.close();
    // if (graphicalRepresentation && dimension == 2) {
    // kMeans.showResultsConvexHull(nameDataSet, run, time);
    // }
  }

  return 0;

  // prepare files for the results
  std::string start = "results/" + nameDataSet;
  std::string end = ".txt";
  std::fstream summary;
  summary.open(start + "_summary" + end, std::ios::out);
  summary.close();

  std::cout.setf(std::ios::unitbuf);
  if (1) {
    // string labelfn = "/home/sami/Drive/uef/bkmeans/regularized-k-means/labels5k.csv";
    // string labelfn = "/home/sami/Drive/uef/bkmeans/regularized-k-means/labels14M_10.csv";
    // string labelfn = "/home/sami/Drive/uef/bkmeans/regularized-k-means/labels14M_3.csv";
    string labelfn = "/home/sjs/Drive/uef/bkmeans/regularized-k-means/unbalance_labels.txt.csv";

    vector<int> vect = readIntVec(labelfn);
    // for (auto i : vect) {
    // std::cout << i << ' ';
    // }
    // cout << "\n";
    // return 0;

    KMeans kMeans;
    kMeans.initialize(size, dimension, nameDataSet, numClusters, 23232323);
    // kMeans.points[0].coord.values
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < size; i++) {

      // kMeans.points[i].clusterId =
      int cluid = floor(numClusters * (static_cast<float>(i) / static_cast<float>(size)));
      if (cluid >= numClusters) {
        cluid = numClusters - 1;
      }
      cluid = vect[i];
      // cout << "cluid:" << cluid << endl;

      Coordinate coord = kMeans.points[i].getCoord();
      kMeans.clusters[cluid].addPointSeq();
      kMeans.clusters[cluid].addCoordSeq(coord);
      kMeans.points[i].setClusterId(cluid);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double time =
        std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
    kMeans.showResultsConvexHull2(nameDataSet, 0, time);

    // kMeans.points[0].coord =
  }

  else {

    // run kMeans numRuns times
    for (int run = 99; run < numRuns; run++) {

      std::cout << "run number" << run << "\n";

      // create a kMeans instance
      KMeans kMeans;
      // kMeans.initialize(size, dimension, nameDataSet, numClusters, seeds[run]);
      kMeans.initialize(vec, numClusters, seeds[run]);
      // solve the kMeans instance
      auto startTime = std::chrono::high_resolution_clock::now();
      kMeans.run(terminationCriterion, terminationCriterionValue, stopWhenBalanced,
                 partlyRemainingFraction, increasingPenaltyFactor, useFunctionIter);
      auto endTime = std::chrono::high_resolution_clock::now();
      double time =
          std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
      // save assignments in file
      std::string filename = start + "_assignments_" + std::to_string(run) + end;
      std::fstream assignments;
      assignments.open(filename, std::ios::out);
      kMeans.writeAssignments(assignments);
      assignments.close();
      // save times in summary file
      summary.open(start + "_summary" + end, std::fstream::app);
      summary << time << std::endl;
      summary.close();
      if (graphicalRepresentation && dimension == 2) {
        kMeans.showResultsConvexHull(nameDataSet, run, time);
      }
    }
  }
}
