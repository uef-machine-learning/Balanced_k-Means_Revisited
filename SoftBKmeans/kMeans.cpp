#include <random>
#include <iostream>
#include <string>
#include <iomanip>
#include <math.h>
#include <fstream>
#include <cstdio>

#include "kMeans.h"
#include "gnuplot.h"

void KMeans::initialize(vector<vector<double>> vec, int numClusters) {

  int size = vec.size();
  int dimension = vec[0].size();
  this->dimension = dimension;
  this->size = size;
  this->numClusters = numClusters;
  points = new Point[size];
  bestAssignment = new int[size];
  // read points from file
  // std::fstream f;
  // // f.open("data\\" + nameDataSet + ".txt", std::ios::in);
  // string fname = "data/" + nameDataSet + ".txt";
  // f.open(fname, std::ios::in);
  // if (f.fail()) {
  // cout << "File open failed:" << fname << "\n";
  // exit(0);
  // }
  for (int i = 0; i < size; i++) {
    double *values = new double[dimension];
    for (int j = 0; j < dimension; j++) {
      values[j] = vec[i][j];
    }
    Coordinate coord = Coordinate(values, dimension);
    points[i] = Point(coord);
  }
  // check if more clusters than points
  if (numClusters > size) {
    std::cout << "More clusters than points!";
    return;
  }
  // initialize clusters
  initializeCenters();
}

KMeans::~KMeans() {
  for (int i = 0; i < size; i++) {
    points[i].deletePoint();
  }
  delete[] points;
  for (int i = 0; i < numClusters; i++) {
    clusters[i].deleteCluster();
  }
  delete[] clusters;
}

bool sortbysec(const pair<int, double> &a, const pair<int, double> &b) {
  return (a.second < b.second);
}

void KMeans::run(TerminationCriterion terminationCriterion, double terminationCriterionValue,
                 bool stopWhenBalanced, double partlyRemainingFraction,
                 double increasingPenaltyFactor, bool useFunctionIter, int switchPostp,
                 int _maxIter) {
  double penaltyNow = 0.0;
  double penaltyNext = std::numeric_limits<double>::max();
  bool balanceReq = false;
  bool balanced = false;
  bool terminate = false;
  bool keepPenalty = false;
  double MSE = std::numeric_limits<double>::max();
  double bestMSE = std::numeric_limits<double>::max();
  int numIter = 0;
  int maxIter = _maxIter;
  // repeat the following steps until the stopping condition is met
  while (!terminate) {
    // assign points to clusters
    for (int i = 0; i < size; i++) {
      points[i].assignToCluster(clusters, numClusters, numIter, penaltyNow, &penaltyNext,
                                partlyRemainingFraction);
    }

    if (maxIter == 0) {
      this->saveAssignments();
      return;
    }
    // check if balance requirements are satisfied or if data set is already completely balanced
    // choose between the following balance criterions:
    switch (terminationCriterion) {
    case TerminationCriterion::MaxDiffClusterSizes:
      balanceReq = this->checkMaxDiffClusterSizes((int)terminationCriterionValue);
      break;
    case TerminationCriterion::MaxSDCS:
      balanceReq = this->checkMaxSDCS(terminationCriterionValue);
      break;
    case TerminationCriterion::MinNormEntro:
      balanceReq = this->checkMinNormEntro(terminationCriterionValue);
      break;
    case TerminationCriterion::MinClusterSize:
      balanceReq = this->checkMinClusterSize((int)terminationCriterionValue);
      break;
    default:
      break;
    }
    //
    balanced = this->checkMaxDiffClusterSizes(1); // always stop if balanced
    // check for termination
    if (balanceReq) {
      if (this->meanSquaredError() < bestMSE) {
        bestMSE = this->meanSquaredError();
        this->saveAssignments();
        keepPenalty = true;
      } else if (stopWhenBalanced || balanced) {
        terminate = true;
        keepPenalty = true;
      }
    }

    if (numIter != 0 && !keepPenalty) {
      // set penalty for next iteration
      if (useFunctionIter) {
        penaltyNow = functionIter(numIter) * penaltyNext;
      } else {
        penaltyNow = increasingPenaltyFactor * penaltyNext;
      }
      penaltyNext = std::numeric_limits<double>::max();
    }
    // printf("i=%d SSE=%f\n",numIter,this->sumOfSquaredErrors());
    keepPenalty = false;
    numIter++;
    if (numIter > maxIter) {
      break;
    }
  }

  if (!(numIter >= maxIter)) {
    restoreBestResults();
  }

  // this->saveAssignments();

  if (switchPostp > 0) {

    vector<std::set<int>> cluvec;
    for (int i = 0; i < numClusters; i++) {
      std::set<int> v;
      cluvec.push_back(v);
    }
    for (int i = 0; i < size; i++) {
      cluvec[points[i].clusterId].insert(i);
    }

    int cluAid = 1;
    int cluBid = 2;
    int switchCount = 1;
    int iter = 1;
    while (switchCount > 0 && iter <= switchPostp) {
      switchCount = 0;
      for (cluAid = 0; cluAid < numClusters - 1; cluAid++) {
        for (cluBid = cluAid + 1; cluBid < numClusters; cluBid++) {
          switchCount += switchOpt(cluAid, cluBid, cluvec[cluAid], cluvec[cluBid]);
          clusters[cluAid].setCentroidSeq();
          clusters[cluBid].setCentroidSeq();
        }
      }
      // printf("iter=%d switchCount=%d\n", iter, switchCount);
      iter++;
    }
    this->saveAssignments();
  }

  printf("iterations=%d\n", numIter);

  this->saveAssignments();
  // cout << "negCount:" << negCountA << " " << negCountB << " ";
}

int KMeans::switchOpt(int cluAid, int cluBid, std::set<int> &cluA, std::set<int> &cluB) {
  vector<std::pair<int, double>> cluAdist;
  vector<std::pair<int, double>> cluBdist;
  int negCountA = 0;
  for (int x : cluA) {
    double distB = clusters[cluBid].getSqrDistance(points[x].coord);
    double distA = clusters[cluAid].getSqrDistance(points[x].coord);
    double delta = distB - distA;
    cluAdist.push_back(std::make_pair(x, delta));
    if (delta < 0) {
      negCountA++;
    }
    // std::cout << "SQE:" << distA << " " << distB << " " << delta << "\n";
  }
  sort(cluAdist.begin(), cluAdist.end(), sortbysec);
  // for (auto x : cluAdist) {
  // std::cout << "id,dist " << x.first << " " << x.second << "\n";
  // }

  int negCountB = 0;
  for (int x : cluB) {
    double distB = clusters[cluBid].getSqrDistance(points[x].coord);
    double distA = clusters[cluAid].getSqrDistance(points[x].coord);
    double delta = distA - distB;
    cluBdist.push_back(std::make_pair(x, delta));
    if (delta < 0) {
      negCountB++;
    }
    // std::cout << "SQE:" << distA << " " << distB << " " << delta << "\n";
  }
  sort(cluBdist.begin(), cluBdist.end(), sortbysec);
  // printf("Asize:%d Bsize:%d negCountA:%d negCountB:%d\n", cluAdist.size(), cluBdist.size(),
  // negCountA, negCountB);
  int minSize = std::min(cluBdist.size(), cluAdist.size());
  int switchCount = 0;
  for (int i = 0; i < minSize; i++) {
    double switchDelta = cluAdist[i].second + cluBdist[i].second;
    int id1 = cluAdist[i].first;
    int id2 = cluBdist[i].first;
    // printf("A=%d,B=%d,%f %f,
    // switchDelta=%f\n",cluAid,cluBid,cluAdist[i].second,cluBdist[i].second,switchDelta);
    if (switchDelta < 0.0) {

      cluA.erase(id1);
      cluB.erase(id2);
      cluA.insert(id2);
      cluB.insert(id1);

      clusters[cluAid].removeCoordSeq(points[id1].coord);
      clusters[cluAid].addCoordSeq(points[id2].coord);
      clusters[cluBid].removeCoordSeq(points[id2].coord);
      clusters[cluBid].addCoordSeq(points[id1].coord);

      // printf("make switch: %d %d %f\n", cluAdist[i].first, cluBdist[i].first, switchDelta);
      std::swap(points[id1].clusterId, points[id2].clusterId);
      switchCount++;
    } else {
      break;
    }
  }
  // printf("switchCount: %d\n", switchCount);
  return switchCount;
}

double KMeans::functionIter(int numIter) {
  if (numIter > 100) {
    return 1.01;
  } else {
    return 1.1009 - 0.0009 * numIter;
  }
}

void KMeans::initializeCenters() {
  clusters = new Cluster[numClusters];
  int numUsedPoints = 0;
  int *usedPoints = new int[numClusters];
  for (int i = 0; i < numClusters; i++) {
    usedPoints[i] = -1;
  }
  int randomInt = 0;
  while (numUsedPoints < numClusters) {
    if (size < RAND_MAX) { // RAND_MAX is the largest number, that rand() can produce
      // usual
      randomInt = rand() % size;
    } else {
      // e.g. birch sets
      randomInt = int(rand() * (size / RAND_MAX));
    }
    int index = randomInt % size;
    bool alreadyContained =
        std::find(usedPoints, usedPoints + numClusters, index) != (usedPoints + numClusters);
    if (!alreadyContained) {
      Cluster cluster = Cluster(points[index].getCoord());
      clusters[numUsedPoints] = cluster;
      usedPoints[numUsedPoints] = index;
      numUsedPoints++;
    }
  }
  delete[] usedPoints;
}

void KMeans::writeAssignments(std::fstream &f) {

  // Sort labels based on x value
  vector<std::pair<double, int>> vmap;
  vector<int> rmap(numClusters, 0);
  for (int i = 0; i < numClusters; i++) {
    Coordinate coord = clusters[i].getCentroid();
    double xval = coord.getValueInDim(0);
    vmap.push_back(std::make_pair(xval, i));
  }
  sort(vmap.begin(), vmap.end()); // Sort by first value of pair
  for (int i = 0; i < numClusters; i++) {
    // std::cout << vmap[i].first << " " << vmap[i].second << "\n";
    rmap[vmap[i].second] = i;
  }

  // write the assignments to the file f
  for (int i = 0; i < size; i++) {
    f << rmap[bestAssignment[i]] << "\n";
  }
}

int* KMeans::getLabels() {

  // Sort labels based on x value
  vector<std::pair<double, int>> vmap;
  // vector<int> labels;
  vector<int> rmap(numClusters, 0);
  int* labels = new int[size];
  for (int i = 0; i < numClusters; i++) {
    Coordinate coord = clusters[i].getCentroid();
    double xval = coord.getValueInDim(0);
    vmap.push_back(std::make_pair(xval, i));
  }
  sort(vmap.begin(), vmap.end()); // Sort by first value of pair
  for (int i = 0; i < numClusters; i++) {
    // std::cout << vmap[i].first << " " << vmap[i].second << "\n";
    rmap[vmap[i].second] = i;
  }

  for (int i = 0; i < size; i++) {
    // labels.push_back(rmap[bestAssignment[i]]);
    labels[i] = rmap[bestAssignment[i]];
  }
  return labels;
}


void KMeans::printCentroids() {
  std::cout << "CENTROIDS\n";
  for (int i = 0; i < numClusters; i++) {
    Coordinate coord = clusters[i].getCentroid();
    // std::cout << "dim: " << coord.getDimension() << "\n";
    for (int i_dim = 0; i_dim < coord.getDimension(); i_dim++) {
      std::cout << coord.getValueInDim(i_dim) << " ";
      if (i_dim < coord.getDimension() - 1) {
      }
    }
    std::cout << "\n";
  }
  std::cout << "=========\n";
}

void KMeans::writeCentroids(std::fstream &f) {
  // std::cout << "KMeans::writeCentroids" << " ";
  for (int i = 0; i < numClusters; i++) {
    Coordinate coord = clusters[i].getCentroid();
    // std::cout << "dim: " << coord.getDimension() << "\n";
    for (int i_dim = 0; i_dim < coord.getDimension(); i_dim++) {
      f << coord.getValueInDim(i_dim);
      // std::cout << coord.getValueInDim(i_dim) << " ";
      if (i_dim < coord.getDimension() - 1) {
        f << " ";
      }
    }
    // std::cout << "\n";
    f << "\n";
  }
}

void KMeans::saveAssignments() {
  for (int i = 0; i < size; i++) {
    int clusterId = points[i].getClusterId();
    bestAssignment[i] = clusterId;
  }
}

double KMeans::sumOfSquaredErrors() {
  double SSE = 0.0;
  for (int i = 0; i < size; i++) {
    int clusterId = points[i].getClusterId();
    double sqrError = points[i].computeSqrDist(clusters[clusterId]);
    SSE += sqrError;
  }
  return SSE;
}

double KMeans::meanSquaredError() { return sumOfSquaredErrors() / (double)(size); }

bool KMeans::checkMaxDiffClusterSizes(int maxDiffClusterSizes) {
  int diffClusterSizes = computeDiffClusterSizes();
  return diffClusterSizes <= maxDiffClusterSizes;
}

int KMeans::computeDiffClusterSizes() {
  int maxClusterSize = 0;
  int minClusterSize = size;
  for (int j = 0; j < numClusters; j++) {
    int clusterSize = clusters[j].getNumPointsSeq();
    if (clusterSize > maxClusterSize) {
      maxClusterSize = clusterSize;
    }
    if (clusterSize < minClusterSize) {
      minClusterSize = clusterSize;
    }
  }
  int diffClusterSizes = maxClusterSize - minClusterSize;
  return diffClusterSizes;
}

bool KMeans::checkMinClusterSize(int minClusterSize) {
  for (int j = 0; j < numClusters; j++) {
    int clusterSize = clusters[j].getNumPointsSeq();
    if (clusterSize < minClusterSize) {
      return false;
    }
  }
  return true;
}

bool KMeans::checkMaxSDCS(double maxSDCS) {
  double SDCS = computeSDCS();
  return SDCS <= maxSDCS;
}

double KMeans::computeSDCS() {
  double avgSize = (double)size / (double)numClusters;
  double sum = 0.0;
  for (int j = 0; j < numClusters; j++) {
    int clusterSize = clusters[j].getNumPointsSeq();
    sum = sum + (clusterSize - avgSize) * (clusterSize - avgSize);
  }
  double SDCS = sqrt(sum / (double)(numClusters - 1));
  return SDCS;
}

bool KMeans::checkMinNormEntro(double minNormEntro) {
  double normEntro = computeNormEntro();
  return normEntro >= minNormEntro;
}

double KMeans::computeNormEntro() {
  double sum = 0.0;
  for (int j = 0; j < numClusters; j++) {
    double clusterSize = (double)clusters[j].getNumPointsSeq();
    sum = sum + (clusterSize / size) * log2(clusterSize / size);
  }
  double normEntro = -sum / log2((double)numClusters);
  return normEntro;
}

void KMeans::restoreBestResults() {
  // restore the best assignment and update number of points in each cluster
  for (int i = 0; i < size; i++) {
    int oldClusterId = points[i].getClusterId();
    int newClusterId = bestAssignment[i];
    if (newClusterId != oldClusterId) {
      Coordinate coord = points[i].getCoord();
      clusters[oldClusterId].removePointSeq();
      clusters[oldClusterId].removeCoordSeq(coord);
      clusters[newClusterId].addPointSeq();
      clusters[newClusterId].addCoordSeq(coord);
      points[i].setClusterId(newClusterId);
    }
  }
  for (int j = 0; j < numClusters; j++) {
    clusters[j].setCentroidSeq();
  }
}

void KMeans::showResultsConvexHull(string nameDataSet, int run, double timeInSec) {
  // restore the best assignment and update number of points in each cluster
  // TODO:
  for (int i = 0; i < size; i++) {
    int oldClusterId = points[i].getClusterId();
    int newClusterId = bestAssignment[i];
    if (newClusterId != oldClusterId) {
      Coordinate coord = points[i].getCoord();
      clusters[oldClusterId].removePointSeq();
      clusters[oldClusterId].removeCoordSeq(coord);
      clusters[newClusterId].addPointSeq();
      clusters[newClusterId].addCoordSeq(coord);
      points[i].setClusterId(newClusterId);
    }
  }
  for (int j = 0; j < numClusters; j++) {
    clusters[j].setCentroidSeq();
  }
  // first sort points regarding their clusters
  Point **orderedPoints = new Point *[numClusters];
  int *numPointsInCluster = new int[numClusters];
  for (int i = 0; i < numClusters; i++) {
    Point *pointsSameCluster = new Point[clusters[i].getNumPointsSeq()];
    orderedPoints[i] = pointsSameCluster;
    numPointsInCluster[i] = 0;
  }
  for (int i = 0; i < size; i++) {
    int clusterId = points[i].getClusterId();
    int position = numPointsInCluster[clusterId];
    orderedPoints[clusterId][position] = points[i];
    numPointsInCluster[clusterId]++;
  }
  // find points of the convex hull for each cluster
  for (int i = 0; i < numClusters; i++) {
    // find points of the convex hull for cluster i
    // points of convexHull are contained in array until nullPoint
    int numPointsConvexHull = this->computeConvexHull(orderedPoints[i], numPointsInCluster[i]);
    // now orderedPoints[i] contains the points of the convex hull
    // write these points in a .dat file

    std::string start = "computationConvexHull/convexHull";
    std::string end = ".dat";
    std::fstream f;
    std::string filename = start + std::to_string(i) + end;
    f.open(filename, std::ios::out);
    for (int j = 0; j < numPointsConvexHull; j++) {
      Coordinate coord = orderedPoints[i][j].getCoord();
      double xValue = coord.getValueInDim(0);
      double yValue = coord.getValueInDim(1);
      f << xValue << "\t\t" << yValue << "\n";
    }
    // write first point again to get a closed circle
    Coordinate coord = orderedPoints[i][0].getCoord();
    double xValue = coord.getValueInDim(0);
    double yValue = coord.getValueInDim(1);
    f << xValue << "\t\t" << yValue << "\n";
    f.close();
  }
  // write all points in a .dat file
  std::fstream f;
  f.open("computationConvexHull/points.dat", std::ios::out);
  for (int i = 0; i < size; i++) {
    Coordinate coord = points[i].getCoord();
    double xValue = coord.getValueInDim(0);
    double yValue = coord.getValueInDim(1);
    f << xValue << "\t\t" << yValue << "\n";
  }
  f.close();
  // then write the centroids in a .dat file
  // write them in three files: too few points = red, correct number of points = blue, too many
  // points = green
  std::fstream tooFew;
  std::fstream correct;
  std::fstream tooMany;
  tooFew.open("computationConvexHull/centroidsTooFew.dat", std::ios::out);
  correct.open("computationConvexHull/centroidsCorrect.dat", std::ios::out);
  tooMany.open("computationConvexHull/centroidsTooMany.dat", std::ios::out);
  double meanPointsPerCluster = (double)size / (double)numClusters;
  for (int i = 0; i < numClusters; i++) {
    Coordinate coord = clusters[i].getCentroid();
    double xValue = coord.getValueInDim(0);
    double yValue = coord.getValueInDim(1);
    // cluster has more points than average
    int penMore = clusters[i].getNumPointsSeq() - (int)ceil(meanPointsPerCluster);
    // cluster has less points than average
    int penLess = (int)floor(meanPointsPerCluster) - clusters[i].getNumPointsSeq();
    // std::cout << penMore << "\t" << penLess << "\n";
    if (penLess >= 1) {
      tooFew << xValue << "\t\t" << yValue << "\n";
    } else {
      if (penMore >= 1) {
        tooMany << xValue << "\t\t" << yValue << "\n";
      } else {
        correct << xValue << "\t\t" << yValue << "\n";
      }
    }
  }
  tooFew.close();
  correct.close();
  tooMany.close();
  Gnuplot plot;
  // plot to output files
  // 1125*2
  // 2250
  // ,
  // 786*2
  // 1572

  // plot("set terminal pngcairo size 1125, 786 enhanced font 'Verdana,10'");
  plot("set terminal pngcairo size 2250, 1572 enhanced font 'Verdana,14'");
  std::string filenameCommand =
      "set output 'convexHulls/B" + nameDataSet + "_" + std::to_string(run) + "_ConvexHull.png'";
  plot(filenameCommand);
  // command for plotting the data sets
  std::string command = "plot ";
  command = command + "'computationConvexHull/points.dat' ls 2, ";
  std::string start = "'computationConvexHull/convexHull";
  std::string end = ".dat'";
  for (int i = 0; i < numClusters; i++) {
    std::string lineStyle = "with line ls 3,";
    command = command + start + std::to_string(i) + end + lineStyle;
  }
  // command = command + "'centroids.dat' ls 1";
  command = command + "'computationConvexHull/centroidsCorrect.dat' ls 1, "
                      "'computationConvexHull/centroidsTooFew.dat' ls 4, "
                      "'computationConvexHull/centroidsTooMany.dat' ls 5";
  std::string stringMSE = std::to_string(meanSquaredError());
  std::string stringDiffClusterSizes = std::to_string(computeDiffClusterSizes());
  std::string stringSDCS = std::to_string(computeSDCS());
  std::string stringNormEntro = std::to_string(computeNormEntro());
  std::string timeInSecString = std::to_string(timeInSec);
  std::string commandTitle = "set title 'MSE =  " + stringMSE +
                             ", diffClusterSizes = " + stringDiffClusterSizes +
                             ", SDCS = " + stringSDCS + ", normEntro = " + stringNormEntro +
                             ", timeInSec = " + timeInSecString + "'";
  cout << commandTitle << endl;
  // for normal view
  double **bounds = getBounds();
#ifdef DISABLED
  std::string commandxRange =
      "set xrange [" + std::to_string(bounds[0][0]) + "0 : " + std::to_string(bounds[0][1]) + "]";
  std::string commandyRange =
      "set yrange [" + std::to_string(bounds[1][0]) + "0 : " + std::to_string(bounds[1][1]) + "]";
#endif

  double x_min = bounds[0][0];
  double x_max = bounds[0][1];
  double y_min = bounds[1][0];
  double y_max = bounds[1][1];
  double x_length = x_max - x_min;
  double y_length = y_max - y_min;
  if (x_length > y_length) {
    double y_correction = (x_length - y_length) / 2.0;
    y_min = y_min - y_correction;
    y_max = y_max + y_correction;
  } else {
    double x_correction = (y_length - x_length) / 2.0;
    x_min = x_min - x_correction;
    x_max = x_max + x_correction;
  }
  std::string commandxRange =
      "set xrange [" + std::to_string(x_min) + "0 : " + std::to_string(x_max) + "]";
  std::string commandyRange =
      "set yrange [" + std::to_string(y_min) + "0 : " + std::to_string(y_max) + "]";
  // End new version of range

  plot("set nokey");
  plot("set tics font ', 5'");
  plot("set title font ', 5'");
  plot("set size square");

  plot("unset border");
  plot("unset xtics");
  plot("unset ytics");

  // set styles for points on convex hull
  plot("set style line 3 lt 1 lc rgb '#000000' lw 1.0");
  // set styles for points
  plot("set style line 2 lc rgb '#000000' pt 7 ps 0.6");
  // set styles for centroids
  plot("set style line 1 lc rgb 'blue' pt 7 ps 2");  // blue
                                                     // set styles for centroids with too few
                                                     // centroids -> green
  plot("set style line 4 lc rgb 'green' pt 7 ps 2"); // green
                                                     // set styles for centroids with too many
                                                     // points -> red
  plot("set style line 5 lc rgb 'red' pt 7 ps 2");   // red
                                                     // apply commands
  plot(commandxRange);
  plot(commandyRange);
  // plot(commandTitle);
  plot(command);
  // memory
  for (int i = 0; i < numClusters; i++) {
    delete[] orderedPoints[i];
  }
  delete[] orderedPoints;
  delete[] numPointsInCluster;
}

void plotcout(string s) { cout << s << endl; }
void KMeans::showResultsConvexHull2(string nameDataSet, int run, double timeInSec) {
  // restore the best assignment and update number of points in each cluster
  // for (int i = 0; i < size; i++) {
  // int oldClusterId = points[i].getClusterId();
  // // int newClusterId = bestAssignment[i];
  // int newClusterId = oldClusterId;

  // // if (newClusterId != oldClusterId) {
  // Coordinate coord = points[i].getCoord();
  // clusters[oldClusterId].removePointSeq();
  // clusters[oldClusterId].removeCoordSeq(coord);
  // clusters[newClusterId].addPointSeq();
  // clusters[newClusterId].addCoordSeq(coord);
  // points[i].setClusterId(newClusterId);
  // // }
  // }
  // break kMeans.cpp:457
  // p(
  std::cout << "showres\n";
  for (int j = 0; j < numClusters; j++) {
    // clusters[j].setCentroidSeq();
  }
  // first sort points regarding their clusters
  Point **orderedPoints = new Point *[numClusters];
  int *numPointsInCluster = new int[numClusters];
  for (int i = 0; i < numClusters; i++) {
    Point *pointsSameCluster = new Point[clusters[i].getNumPointsSeq()];
    orderedPoints[i] = pointsSameCluster;
    numPointsInCluster[i] = 0;
  }
  for (int i = 0; i < size; i++) {
    int clusterId = points[i].getClusterId();
    int position = numPointsInCluster[clusterId];
    orderedPoints[clusterId][position] = points[i];
    numPointsInCluster[clusterId]++;
  }
  // find points of the convex hull for each cluster
  for (int i = 0; i < numClusters; i++) {
    // find points of the convex hull for cluster i
    // points of convexHull are contained in array until nullPoint
    int numPointsConvexHull = this->computeConvexHull(orderedPoints[i], numPointsInCluster[i]);
    // now orderedPoints[i] contains the points of the convex hull
    // write these points in a .dat file
    std::string start = "computationConvexHull/convexHull";
    std::string end = ".dat";
    std::fstream f;
    std::string filename = start + std::to_string(i) + end;
    f.open(filename, std::ios::out);
    for (int j = 0; j < numPointsConvexHull; j++) {
      Coordinate coord = orderedPoints[i][j].getCoord();
      double xValue = coord.getValueInDim(0);
      double yValue = coord.getValueInDim(1);
      f << xValue << "\t\t" << yValue << "\n";
    }
    // write first point again to get a closed circle
    Coordinate coord = orderedPoints[i][0].getCoord();
    double xValue = coord.getValueInDim(0);
    double yValue = coord.getValueInDim(1);
    f << xValue << "\t\t" << yValue << "\n";
    f.close();
  }
  // write all points in a .dat file
  std::fstream f;
  f.open("computationConvexHull/points.dat", std::ios::out);
  for (int i = 0; i < size; i++) {
    Coordinate coord = points[i].getCoord();
    double xValue = coord.getValueInDim(0);
    double yValue = coord.getValueInDim(1);
    f << xValue << "\t\t" << yValue << "\n";
  }
  f.close();
  // then write the centroids in a .dat file
  // write them in three files: too few points = red, correct number of points = blue, too many
  // points = green
  std::fstream tooFew;
  std::fstream correct;
  std::fstream tooMany;
  tooFew.open("computationConvexHull/centroidsTooFew.dat", std::ios::out);
  correct.open("computationConvexHull/centroidsCorrect.dat", std::ios::out);
  tooMany.open("computationConvexHull/centroidsTooMany.dat", std::ios::out);
  double meanPointsPerCluster = (double)size / (double)numClusters;
  for (int i = 0; i < numClusters; i++) {
    Coordinate coord = clusters[i].getCentroid();
    double xValue = coord.getValueInDim(0);
    double yValue = coord.getValueInDim(1);
    // cluster has more points than average
    int penMore = clusters[i].getNumPointsSeq() - (int)ceil(meanPointsPerCluster);
    // cluster has less points than average
    int penLess = (int)floor(meanPointsPerCluster) - clusters[i].getNumPointsSeq();
    // std::cout << penMore << "\t" << penLess << "\n";
    if (penLess >= 1) {
      tooFew << xValue << "\t\t" << yValue << "\n";
    } else {
      if (penMore >= 1) {
        tooMany << xValue << "\t\t" << yValue << "\n";
      } else {
        correct << xValue << "\t\t" << yValue << "\n";
      }
    }
  }
  tooFew.close();
  correct.close();
  tooMany.close();
  Gnuplot plot;
  // plot to output files
  // 1125*2
  // 2250
  // ,
  // 786*2
  // 1572

  // plot("set terminal pngcairo size 1125, 786 enhanced font 'Verdana,10'");
  plot("set terminal pngcairo size 2250, 1572 enhanced font 'Verdana,14'");
  std::string filenameCommand =
      "set output 'convexHulls/" + nameDataSet + "_" + std::to_string(run) + "_ConvexHull.png'";
  plot(filenameCommand);
  // command for plotting the data sets
  std::string command = "plot ";
  command = command + "'computationConvexHull/points.dat' ls 2, ";
  std::string start = "'computationConvexHull/convexHull";
  std::string end = ".dat'";
  for (int i = 0; i < numClusters; i++) {
    std::string lineStyle = "with line ls 3,";
    command = command + start + std::to_string(i) + end + lineStyle;
  }
  // command = command + "'centroids.dat' ls 1";
  command = command + "'computationConvexHull/centroidsCorrect.dat' ls 1, "
                      "'computationConvexHull/centroidsTooFew.dat' ls 4, "
                      "'computationConvexHull/centroidsTooMany.dat' ls 5";
  std::string stringMSE = std::to_string(meanSquaredError());
  std::string stringDiffClusterSizes = std::to_string(computeDiffClusterSizes());
  std::string stringSDCS = std::to_string(computeSDCS());
  std::string stringNormEntro = std::to_string(computeNormEntro());
  std::string timeInSecString = std::to_string(timeInSec);
  std::string commandTitle = "set title 'MSE =  " + stringMSE +
                             ", diffClusterSizes = " + stringDiffClusterSizes +
                             ", SDCS = " + stringSDCS + ", normEntro = " + stringNormEntro +
                             ", timeInSec = " + timeInSecString + "'";

  // for normal view
  double **bounds = getBounds();
#ifdef DISABLED
  std::string commandxRange =
      "set xrange [" + std::to_string(bounds[0][0]) + "0 : " + std::to_string(bounds[0][1]) + "]";
  std::string commandyRange =
      "set yrange [" + std::to_string(bounds[1][0]) + "0 : " + std::to_string(bounds[1][1]) + "]";
#endif

  double x_min = bounds[0][0];
  double x_max = bounds[0][1];
  double y_min = bounds[1][0];
  double y_max = bounds[1][1];
  double x_length = x_max - x_min;
  double y_length = y_max - y_min;
  if (x_length > y_length) {
    double y_correction = (x_length - y_length) / 2.0;
    y_min = y_min - y_correction;
    y_max = y_max + y_correction;
  } else {
    double x_correction = (y_length - x_length) / 2.0;
    x_min = x_min - x_correction;
    x_max = x_max + x_correction;
  }
  std::string commandxRange =
      "set xrange [" + std::to_string(x_min) + "0 : " + std::to_string(x_max) + "]";
  std::string commandyRange =
      "set yrange [" + std::to_string(y_min) + "0 : " + std::to_string(y_max) + "]";
  // End new version of range

  plot("set nokey");
  plot("set tics font ', 5'");
  plot("set title font ', 5'");
  plot("set size square");

  plot("unset border");
  plot("unset xtics");
  plot("unset ytics");

  // set styles for points on convex hull
  plot("set style line 3 lt 1 lc rgb '#000000' lw 1.0");
  // set styles for points

  // plot("set style line 2 lc rgb '#000000' pt 7 ps 0.6");
  plot("set style line 2 lc rgb '#000000' pt 7 ps 0.3");

  // set styles for centroids
  plot("set style line 1 lc rgb '#3333ff' pt 7 ps 2"); // blue
                                                       // set styles for centroids with too few
                                                       // centroids -> green
  plot("set style line 4 lc rgb 'green' pt 7 ps 2");   // green
                                                       // set styles for centroids with too many
                                                       // points -> red
  plot("set style line 5 lc rgb 'red' pt 7 ps 2");     // red
                                                       // apply commands
  plot(commandxRange);
  plot(commandyRange);
  // plot(commandTitle);
  plot(command);
  // memory
  for (int i = 0; i < numClusters; i++) {
    delete[] orderedPoints[i];
  }
  delete[] orderedPoints;
  delete[] numPointsInCluster;
}

int KMeans::computeConvexHull(Point *pointsInCluster, int numPoints) {
  // only one element -> don't change array, return 1
  if (numPoints == 1) {
    return 1;
  }
  // find point with the smallest y-value
  // same y-values -> take point with the smaller x-value
  double minY = std::numeric_limits<double>::max();
  double minX = std::numeric_limits<double>::max();
  int startId = -1;
  for (int i = 0; i < numPoints; i++) {
    Coordinate coord = pointsInCluster[i].getCoord();
    double yValue = coord.getValueInDim(1);
    if (yValue < minY) {
      minY = yValue;
      minX = coord.getValueInDim(0);
      startId = i;
    } else if (yValue == minY) {
      double xValue = coord.getValueInDim(0);
      if (xValue < minX) {
        minX = xValue;
        startId = i;
      }
    }
  }
  // for each point P_i compute the angle between the x-axis and just found point P_start -> P_i
  // sort points depending on this angle in ascending order
  Point start = pointsInCluster[startId];
  // put start in first position
  pointsInCluster[startId] = pointsInCluster[0];
  pointsInCluster[0] = start;
  start.orderAngle(pointsInCluster, numPoints); // begins from the second point
                                                // Graham Scan algorithm
  int counter = 2;
  int i = 2;
  while (i < numPoints) {
    // take current point
    Point point = pointsInCluster[i];
    if (point.isNull()) {
      // ignore point, process next point
      i++;
      continue;
    }
    Point lastPoint = pointsInCluster[counter - 1];
    Point secondToLastPoint = pointsInCluster[counter - 2];
    // check if the point is on the left side of the vector bewteen the
    // second to last and the last point
    bool left = point.isLeft(secondToLastPoint, lastPoint);
    if (left || counter == 2) {
      // point is left from that vector or only two previous elements
      // keep it
      pointsInCluster[counter] = point;
      counter++;
      // process next point
      i++;
    } else {
      // delete point -> decrease counter
      counter--;
      // process same point again -> do not increase i
    }
  }
  // now pointsInCluster contains the points of the convex hull until index counter - 1
  // return the number of points in the convex hull
  return counter;
}

double **KMeans::getBounds() {
  double xMin = std::numeric_limits<double>::max();
  double xMax = std::numeric_limits<double>::min();
  double yMin = std::numeric_limits<double>::max();
  double yMax = std::numeric_limits<double>::min();
  for (int i = 0; i < size; i++) {
    Coordinate coord = points[i].getCoord();
    double xValue = coord.getValueInDim(0);
    if (xValue < xMin) {
      xMin = xValue;
    }
    if (xValue > xMax) {
      xMax = xValue;
    }
    double yValue = coord.getValueInDim(1);
    if (yValue < yMin) {
      yMin = yValue;
    }
    if (yValue > yMax) {
      yMax = yValue;
    }
  }
  double *xBounds = new double[2]{xMin, xMax};
  double *yBounds = new double[2]{yMin, yMax};
  double **bounds = new double *[2] { xBounds, yBounds };
  return bounds;
}
