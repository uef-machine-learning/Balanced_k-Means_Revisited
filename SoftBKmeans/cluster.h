#pragma once

#include "coordinate.h"

class Cluster {

public:
	Cluster() {};
	Cluster(Coordinate centroid);
	void deleteCluster();

	void addPointSeq();
	void removePointSeq();
	void addCoordSeq(Coordinate coord);
	void removeCoordSeq(Coordinate coord);
	double getSqrDistance(Coordinate coord);
	void setCentroidSeq();

	int getNumPointsSeq();
	Coordinate getCentroid();

private:
	Coordinate centroid;
	int numPointsSeq;
	Coordinate sumCoords;

};
