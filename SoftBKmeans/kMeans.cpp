#include <random>
#include <iostream>
#include <string>
#include <iomanip>
#include <math.h>
#include <fstream>

#include "kMeans.h"
#include "gnuplot.h"

void KMeans::initialize(int size, int dimension, std::string nameDataSet, int numClusters, int seed) {
	this->dimension = dimension;
	this->size = size;
	this->numClusters = numClusters;
	points = new Point[size];
	bestAssignment = new int[size];
	//read points from file
	std::fstream f;
	f.open("data\\" + nameDataSet + ".txt", std::ios::in);
	for (int i = 0; i < size; i++) {
		double* values = new double[dimension];
		for (int j = 0; j < dimension; j++) {
			double value;
			f >> value;
			values[j] = value;
		}
		Coordinate coord = Coordinate(values, dimension);
		points[i] = Point(coord);
	}
	f.close();
	//check if more clusters than points
	if (numClusters > size) {
		std::cout << "More clusters than points!";
		return;
	}
	//initialize clusters
	initializeCenters();
}

KMeans::~KMeans()
{
	for (int i = 0; i < size; i++) {
		points[i].deletePoint();
	}
	delete[] points;
	for (int i = 0; i < numClusters; i++) {
		clusters[i].deleteCluster();
	}
	delete[] clusters;
}

void KMeans::run(TerminationCriterion terminationCriterion, double terminationCriterionValue, bool stopWhenBalanced, double partlyRemainingFraction, 
	double increasingPenaltyFactor, bool useFunctionIter) {
	double penaltyNow = 0.0;
	double penaltyNext = std::numeric_limits<double>::max();
	bool balanceReq = false;
	bool balanced = false;
	bool terminate = false;
	bool keepPenalty = false;
	double MSE = std::numeric_limits<double>::max();
	double bestMSE = std::numeric_limits<double>::max();
	int numIter = 0;
	//repeat the following steps until the stopping condition is met
	while (!terminate) {
		//assign points to clusters
		for (int i = 0; i < size; i++) {
			points[i].assignToCluster(clusters, numClusters, numIter, penaltyNow, &penaltyNext, partlyRemainingFraction);
		}
		//check if balance requirements are satisfied or if data set is already completely balanced
		//choose between the following balance criterions:
		switch (terminationCriterion)
		{
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
		balanced = this->checkMaxDiffClusterSizes(1); //always stop if balanced
		//check for termination
		if (balanceReq) {
			if (this->meanSquaredError() < bestMSE) {
				bestMSE = this->meanSquaredError();
				this->saveAssignments();
				keepPenalty = true;
			}
			if (stopWhenBalanced || balanced) {
				terminate = true;
				keepPenalty = true;
			}
		}
		if (numIter != 0 && !keepPenalty) {
			//set penalty for next iteration
			if (useFunctionIter) {
				penaltyNow = functionIter(numIter) * penaltyNext;
			}
			else {
				penaltyNow = increasingPenaltyFactor * penaltyNext;
			}
			penaltyNext = std::numeric_limits<double>::max();
		}
		keepPenalty = false;
		numIter++;
	}
}

double KMeans::functionIter(int numIter) {
	if (numIter > 100) {
		return 1.01;
	}
	else {
		return 1.1009 - 0.0009 * numIter;
	}
}

void KMeans::initializeCenters() {
	clusters = new Cluster[numClusters];
	int numUsedPoints = 0;
	int* usedPoints = new int[numClusters];
	for (int i = 0; i < numClusters; i++) {
		usedPoints[i] = -1;
	}
	int randomInt = 0;
	while (numUsedPoints < numClusters) {
		if (size < RAND_MAX) { //RAND_MAX is the largest number, that rand() can produce
			//usual
			randomInt = rand() % size;
		}
		else {
			//e.g. birch sets
			randomInt = int(rand() * (size / RAND_MAX));
		}
		int index = randomInt % size;
		bool alreadyContained = std::find(usedPoints, usedPoints + numClusters, index) != (usedPoints + numClusters);
		if (!alreadyContained) {
			Cluster cluster = Cluster(points[index].getCoord());
			clusters[numUsedPoints] = cluster;
			usedPoints[numUsedPoints] = index;
			numUsedPoints++;
		}
	}
	delete[] usedPoints;
}

void KMeans::writeAssignments(std::fstream& f) {
	//write the assignments in the file f
	for (int i = 0; i < size; i++) {
		f << bestAssignment[i] << "\n";
	}
}

void KMeans::saveAssignments() {
	for (int i = 0; i < size; i++) {
		int clusterId = points[i].getClusterId();
		bestAssignment[i] = clusterId;
	}
}

double KMeans::meanSquaredError() {
	double SSE = 0.0;
	for (int i = 0; i < size; i++) {
		int clusterId = points[i].getClusterId();
		double sqrError = points[i].computeSqrDist(clusters[clusterId]);
		SSE += sqrError;
	}
	return SSE / (double)(size);
}

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

void KMeans::showResultsConvexHull(string nameDataSet, int run, double timeInSec) {
	//restore the best assignment and update number of points in each cluster
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
	//first sort points regarding their clusters
	Point** orderedPoints = new Point*[numClusters];
	int* numPointsInCluster = new int[numClusters];
	for (int i = 0; i < numClusters; i++) {
		Point* pointsSameCluster = new Point[clusters[i].getNumPointsSeq()];
		orderedPoints[i] = pointsSameCluster;
		numPointsInCluster[i] = 0;
	}
	for (int i = 0; i < size; i++) {
		int clusterId = points[i].getClusterId();
		int position = numPointsInCluster[clusterId];
		orderedPoints[clusterId][position] = points[i];
		numPointsInCluster[clusterId]++;
	}
	//find points of the convex hull for each cluster
	for (int i = 0; i < numClusters; i++) {
		//find points of the convex hull for cluster i
		//points of convexHull are contained in array until nullPoint
		int numPointsConvexHull = this->computeConvexHull(orderedPoints[i], numPointsInCluster[i]);
		//now orderedPoints[i] contains the points of the convex hull
		//write these points in a .dat file
		std::string start = "computationConvexHull\\convexHull";
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
		//write first point again to get a closed circle
		Coordinate coord = orderedPoints[i][0].getCoord();
		double xValue = coord.getValueInDim(0);
		double yValue = coord.getValueInDim(1);
		f << xValue << "\t\t" << yValue << "\n";
		f.close();
	}
	//write all points in a .dat file
	std::fstream f;
	f.open("computationConvexHull\\points.dat", std::ios::out);
	for (int i = 0; i < size; i++) {
		Coordinate coord = points[i].getCoord();
		double xValue = coord.getValueInDim(0);
		double yValue = coord.getValueInDim(1);
		f << xValue << "\t\t" << yValue << "\n";
	}
	f.close();
	//then write the centroids in a .dat file
	//write them in three files: too few points = red, correct number of points = blue, too many points = green
	std::fstream tooFew;
	std::fstream correct;
	std::fstream tooMany;
	tooFew.open("computationConvexHull\\centroidsTooFew.dat", std::ios::out);
	correct.open("computationConvexHull\\centroidsCorrect.dat", std::ios::out);
	tooMany.open("computationConvexHull\\centroidsTooMany.dat", std::ios::out);
	double meanPointsPerCluster = (double)size / (double)numClusters;
	for (int i = 0; i < numClusters; i++) {
		Coordinate coord = clusters[i].getCentroid();
		double xValue = coord.getValueInDim(0);
		double yValue = coord.getValueInDim(1);
		//cluster has more points than average
		int penMore = clusters[i].getNumPointsSeq() - (int)ceil(meanPointsPerCluster);
		//cluster has less points than average
		int penLess = (int)floor(meanPointsPerCluster) - clusters[i].getNumPointsSeq();
		//std::cout << penMore << "\t" << penLess << "\n";
		if (penLess >= 1) {
			tooFew << xValue << "\t\t" << yValue << "\n";
		}
		else {
			if (penMore >= 1) {
				tooMany << xValue << "\t\t" << yValue << "\n";
			}
			else {
				correct << xValue << "\t\t" << yValue << "\n";
			}
		}
	}
	tooFew.close();
	correct.close();
	tooMany.close();
	Gnuplot plot;
	//plot to output files
	plot("set terminal pngcairo size 1125, 786 enhanced font 'Verdana,10'");
	std::string filenameCommand = "set output 'convexHulls\\" + nameDataSet + "_" + std::to_string(run) + "_ConvexHull.png'";
	plot(filenameCommand);
	//command for plotting the data sets
	std::string command = "plot ";
	command = command + "'computationConvexHull\\points.dat' ls 2, ";
	std::string start = "'computationConvexHull\\convexHull";
	std::string end = ".dat'";
	for (int i = 0; i < numClusters; i++) {
		std::string lineStyle = "with line ls 3,";
		command = command + start + std::to_string(i) + end + lineStyle;
	}
	//command = command + "'centroids.dat' ls 1";
	command = command + "'computationConvexHull\\centroidsCorrect.dat' ls 1, 'computationConvexHull\\centroidsTooFew.dat' ls 4, 'computationConvexHull\\centroidsTooMany.dat' ls 5";
	std::string stringMSE = std::to_string(meanSquaredError());
	std::string stringDiffClusterSizes = std::to_string(computeDiffClusterSizes());
	std::string stringSDCS = std::to_string(computeSDCS());
	std::string stringNormEntro = std::to_string(computeNormEntro());
	std::string timeInSecString = std::to_string(timeInSec);
	std::string commandTitle = "set title 'MSE =  " + stringMSE + 
		", diffClusterSizes = " + stringDiffClusterSizes +
		", SDCS = " + stringSDCS +
		", normEntro = " + stringNormEntro +
		", timeInSec = " + timeInSecString + "'";
	//for normal view
	double **bounds = getBounds();
	std::string commandxRange = "set xrange [" + std::to_string(bounds[0][0]) + "0 : " + std::to_string(bounds[0][1]) + "]";
	std::string commandyRange = "set yrange [" + std::to_string(bounds[1][0]) + "0 : " + std::to_string(bounds[1][1]) + "]";
	plot("set nokey");
	plot("set tics font ', 5'");
	plot("set title font ', 5'");
	plot("set size square");
	//set styles for points on convex hull
	plot("set style line 3 lt 1 lc rgb '#000000' lw 0.5");
	//set styles for points
	plot("set style line 2 lc rgb '#000000' pt 7 ps 0.2");
	//set styles for centroids
	plot("set style line 1 lc rgb 'blue' pt 7 ps 1"); //blue
													  //set styles for centroids with too few centroids -> green
	plot("set style line 4 lc rgb 'green' pt 7 ps 1"); //green
													   //set styles for centroids with too many points -> red
	plot("set style line 5 lc rgb 'red' pt 7 ps 1"); //red
													 //apply commands
	plot(commandxRange);
	plot(commandyRange);
	plot(commandTitle);
	plot(command);
	//memory
	for (int i = 0; i < numClusters; i++) {
		delete[] orderedPoints[i];
	}
	delete[] orderedPoints;
	delete[] numPointsInCluster;
}

int KMeans::computeConvexHull(Point* pointsInCluster, int numPoints) {
	//only one element -> don't change array, return 1
	if (numPoints == 1) {
		return 1;
	}
	//find point with the smallest y-value
	//same y-values -> take point with the smaller x-value
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
		}
		else if (yValue == minY) {
			double xValue = coord.getValueInDim(0);
			if (xValue < minX) {
				minX = xValue;
				startId = i;
			}
		}

	}
	//for each point P_i compute the angle between the x-axis and just found point P_start -> P_i
	//sort points depending on this angle in ascending order
	Point start = pointsInCluster[startId];
	//put start in first position
	pointsInCluster[startId] = pointsInCluster[0];
	pointsInCluster[0] = start;
	start.orderAngle(pointsInCluster, numPoints); //begins from the second point
												  //Graham Scan algorithm
	int counter = 2;
	int i = 2;
	while (i < numPoints) {
		//take current point
		Point point = pointsInCluster[i];
		if (point.isNull()) {
			//ignore point, process next point
			i++;
			continue;
		}
		Point lastPoint = pointsInCluster[counter - 1];
		Point secondToLastPoint = pointsInCluster[counter - 2];
		//check if the point is on the left side of the vector bewteen the
		//second to last and the last point
		bool left = point.isLeft(secondToLastPoint, lastPoint);
		if (left || counter == 2) {
			//point is left from that vector or only two previous elements
			//keep it
			pointsInCluster[counter] = point;
			counter++;
			//process next point
			i++;
		}
		else {
			//delete point -> decrease counter
			counter--;
			//process same point again -> do not increase i
		}
	}
	//now pointsInCluster contains the points of the convex hull until index counter - 1
	//return the number of points in the convex hull
	return counter;
}

double** KMeans::getBounds() {
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
	double* xBounds = new double[2]{ xMin, xMax };
	double* yBounds = new double[2]{ yMin, yMax };
	double** bounds = new double*[2]{ xBounds, yBounds };
	return bounds;
}

