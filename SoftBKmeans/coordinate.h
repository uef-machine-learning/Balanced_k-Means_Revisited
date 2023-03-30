#pragma once

class Coordinate {

public:
	Coordinate() {};
	Coordinate(int dimension);
	Coordinate(double* values, int dimension);
	void deleteCoordinate();

	double getSqrDistance(Coordinate coord);
	int getDimension();

	void addCoord(Coordinate coord);
	void subCoord(Coordinate coord);
	void newCentroid(int numPoints, Coordinate centroid);
	Coordinate copy();

	//the following function is only neessary for the graphical representation
	double getValueInDim(int dim);

// private:
	int dimension;
	double* values;

};
