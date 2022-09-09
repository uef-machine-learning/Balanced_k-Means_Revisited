#include <iostream>

#include "cluster.h"

Cluster::Cluster(Coordinate centroid) {
	this->centroid = centroid.copy();
	numPointsSeq = 0;
	sumCoords = Coordinate(centroid.getDimension());
}

void Cluster::deleteCluster()
{
	centroid.deleteCoordinate();
	sumCoords.deleteCoordinate();
}

void Cluster::addPointSeq() {
	numPointsSeq++;
}

void Cluster::removePointSeq() {
	numPointsSeq--;
}

void Cluster::addCoordSeq(Coordinate coord) {
	sumCoords.addCoord(coord);
}

void Cluster::removeCoordSeq(Coordinate oldCoord) {
	sumCoords.subCoord(oldCoord);
}

double Cluster::getSqrDistance(Coordinate coord) {
	//returns squared distance from some argument to the center of the cluster
	double sqrDist = centroid.getSqrDistance(coord);
	return sqrDist;
}

void Cluster::setCentroidSeq() {
	//set new centroid, keep sumCoords
	sumCoords.newCentroid(numPointsSeq, centroid);
}

int Cluster::getNumPointsSeq() {
	return numPointsSeq;
}

Coordinate Cluster::getCentroid() {
	return centroid;
}

