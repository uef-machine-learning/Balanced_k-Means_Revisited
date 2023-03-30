#pragma once

#include "cluster.h"

#include <cmath>

class Point {

public:
	Point() {};
	Point(Coordinate coord);
	void deletePoint();

	int getClusterId();
	Coordinate getCoord();

	void assignToCluster(Cluster* clusters, int numClusters, int numIter, double penaltyNow, double* penaltyNext,
		double partlyRemainingFraction);
	double computeSqrDist(Cluster cluster); //computes squared distance between the point and the centroid of the cluster

	//these functions are necessary to compute the convex hull, only necessary for graphical representation
	double computeAngle(Point end);
	void orderAngle(Point* point, int length);
	void mergeSortAngle(Point* point, int start, int end);
	void mergeAngle(Point* point, int start, int end, int middle);
	bool isLeft(Point start, Point end);
	void nullPoint() { clusterId = -2; };
	bool isNull() { return clusterId == -2; };
	void setClusterId(int id) { clusterId = id; };

// private:
	Coordinate coord;
	int clusterId;

	void assignmentStandard(Cluster* clusters, int numClusters);
	void assignmentWithPenalty(Cluster* clusters, int numClusters, double penaltyNow, double* penaltyNext,
		double partlyRemainingFraction, int oldClusterId);

};
