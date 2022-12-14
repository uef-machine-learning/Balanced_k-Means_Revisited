#pragma once

#include "point.h"
#include "cluster.h"

class KMeans {

public:
	KMeans() {};
	~KMeans();

	enum class TerminationCriterion { MaxDiffClusterSizes, MinClusterSize, MaxSDCS, MinNormEntro };

	void initialize(int size, int dimension, std::string nameDataSet, int numClusters, int seed);
	void run(TerminationCriterion terminationCriterion, double terminationCriterionValue, bool stopWhenBalanced, double partlyRemainingFraction, 
		double increasingPenaltyFactor, bool useFunctionIter);
	void writeAssignments(std::fstream &assignments);

	//the following function is necessary for the graphical representation of the results for two-dimensional data sets
	void showResultsConvexHull(std::string nameDataSet, int run, double timeInSec);

private:
	int dimension;
	int size;
	Point* points;
	int numClusters;
	Cluster* clusters;
	int* bestAssignment; //save the assignments of the clustering with the lowest MSE satisfying the balancing constraints

	void initializeCenters();
	double meanSquaredError();
	bool checkMaxDiffClusterSizes(int maxDiffClusterSizes);
	int computeDiffClusterSizes();
	bool checkMinClusterSize(int minClusterSize);
	bool checkMaxSDCS(double maxSDCS);
	double computeSDCS();
	bool checkMinNormEntro(double minNormEntro);
	double computeNormEntro();
	double functionIter(int numIter);
	void saveAssignments();

	//the following functions are necessary to compute the convex hull for a graphical representation of the results
	int computeConvexHull(Point* pointsInCluster, int numPoints);
	double** getBounds();

};
