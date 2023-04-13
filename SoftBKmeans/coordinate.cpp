#include "coordinate.h"
#include <math.h>

Coordinate::Coordinate(int dimension) {
	this->dimension = dimension;
	this->values = new double[dimension];
	for (int i = 0; i < dimension; i++) {
		this->values[i] = 0.0;
	}
}

Coordinate::Coordinate(double* values, int dimension) {
	this->dimension = dimension;
	this->values = values;
}

void Coordinate::deleteCoordinate()
{
	delete[] values;
}

double Coordinate::getSqrDistance(Coordinate coord) {
	//returns squared distance between the coordinates
	double sum = 0.0;
	for (int i = 0; i < dimension; i++) {
		double diff=(values[i] - coord.values[i]);
		sum = sum + diff*diff;
		// sum = sum + (values[i] - coord.values[i]) * (values[i] - coord.values[i]);
	}
	sum = sqrt(sum);
	sum = sum*sum;
	return sum;
}

int Coordinate::getDimension() {
	return dimension;
}

void Coordinate::addCoord(Coordinate coord) {
	for (int i = 0; i < dimension; i++) {
		values[i] += coord.values[i];
	}
}

void Coordinate::subCoord(Coordinate coord) {
	for (int i = 0; i < dimension; i++) {
		values[i] -= coord.values[i];
	}
}

void Coordinate::newCentroid(int numPoints, Coordinate centroid) {
	for (int i = 0; i < dimension; i++) {
		double newCentroidCoord = values[i] / (double)numPoints;
		centroid.values[i] = newCentroidCoord;
	}
}

Coordinate Coordinate::copy() {
	double* newValues = new double[dimension];
	for (int i = 0; i < dimension; i++) {
		newValues[i] = values[i];
	}
	Coordinate newCoord = Coordinate(newValues, dimension);
	return newCoord;
}

double Coordinate::getValueInDim(int dim) {
	return values[dim];
}
