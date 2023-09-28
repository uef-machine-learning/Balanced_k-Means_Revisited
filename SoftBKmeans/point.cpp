#include <limits>
#include <iostream>

#include "point.h"

Point::Point(Coordinate coord) {
  this->coord = coord;
  this->clusterId = -1;
}

void Point::deletePoint() { coord.deleteCoordinate(); }

int Point::getClusterId() { return clusterId; }

Coordinate Point::getCoord() { return coord; }

void Point::assignToCluster(Cluster *clusters, int numClusters, int numIter, double penaltyNow,
                            double *penaltyNext, double partlyRemainingFraction) {

  if (numIter == 0) {
    assignmentStandard(clusters, numClusters);
  } else {
    int oldClusterId = clusterId;
    // remove point from old cluster BUT
    if (clusters[oldClusterId].getNumPointsSeq() == 1) {
      // cluster contains only a single point
      // do not do anything, ow you will divide by zero!!!
      return;
    }
    clusters[oldClusterId].removePointSeq();
    clusters[oldClusterId].removeCoordSeq(coord);
    clusters[oldClusterId].setCentroidSeq();
    assignmentWithPenalty(clusters, numClusters, penaltyNow, penaltyNext, partlyRemainingFraction,
                          oldClusterId);
  }
  clusters[clusterId].addPointSeq();
  clusters[clusterId].addCoordSeq(coord);
  if (numIter != 0) { 
    clusters[clusterId].setCentroidSeq();
  }
}

void Point::assignmentWithPenalty(Cluster *clusters, int numClusters, double penaltyNow,
                                  double *penaltyNext, double partlyRemainingFraction,
                                  int oldClusterId) {

  double cost = std::numeric_limits<double>::max();
  double sqrDistOldCluster = clusters[oldClusterId].getSqrDistance(coord);
  double numPointsOldCluster = clusters[oldClusterId].getNumPointsSeq() + partlyRemainingFraction;
  for (int j = 0; j < numClusters; j++) {
    double sqrDistJ = clusters[j].getSqrDistance(coord);
    double numPointsJ = clusters[j].getNumPointsSeq();
    double penaltyNeeded = (sqrDistJ - sqrDistOldCluster) / (numPointsOldCluster - numPointsJ);
    if (numPointsOldCluster > numPointsJ) {
      if (penaltyNow < penaltyNeeded) {
        if (penaltyNeeded < *penaltyNext) {
          *penaltyNext = penaltyNeeded;
        }
      } else {
        if (sqrDistJ + penaltyNow * numPointsJ < cost && j != oldClusterId) {
          cost = sqrDistJ + penaltyNow * numPointsJ;
          clusterId = j;
        }
      }
    } else {
      if (penaltyNow < penaltyNeeded && sqrDistJ + penaltyNow * numPointsJ < cost) {
        cost = sqrDistJ + penaltyNow * numPointsJ;
        clusterId = j;
      }
    }
  }
}

void Point::assignmentStandard(Cluster *clusters, int numClusters) {
  double cost = std::numeric_limits<double>::max();
  for (int j = 0; j < numClusters; j++) {
    double sqrDist = clusters[j].getSqrDistance(coord);
    if (sqrDist < cost) {
      cost = sqrDist;
      clusterId = j;
    }
  }
}

double Point::computeSqrDist(Cluster cluster) {
  double sqrDist = coord.getSqrDistance(cluster.getCentroid());
  return sqrDist;
}

double Point::computeAngle(Point end) {
  if (end.isNull()) {
    return 0.0;
  }
  // this is start
  // compute angle between x-axis and start->end
  // sin(angle) = GK / H
  // GK: difference in y-yoordinate between start and end
  // cannot be negative because start is the point with the smallest y-value
  // H: length of start -> end
  double xStart = coord.getValueInDim(0);
  double xEnd = end.getCoord().getValueInDim(0);
  double yStart = coord.getValueInDim(1);
  double yEnd = end.getCoord().getValueInDim(1);
  double GK = yEnd - yStart;
  double H = sqrt(coord.getSqrDistance(end.getCoord()));
  double angle = asin(GK / H);
  if (xEnd < xStart) {
    angle = 180 - angle;
  }
  return angle;
}

void Point::orderAngle(Point *points, int size) {
  // start ordering from second point, first is already fixed
  this->mergeSortAngle(points, 1, size - 1);
}

void Point::mergeSortAngle(Point *points, int start, int end) {
  if (start < end) {
    int middle = (start + end) / 2;
    this->mergeSortAngle(points, start, middle);
    this->mergeSortAngle(points, middle + 1, end);
    this->mergeAngle(points, start, end, middle);
  }
}

void Point::mergeAngle(Point *points, int start, int end, int middle) {
  int left = start;
  int right = middle + 1;
  int counter = 0;
  Point *pointsWork = new Point[end - start + 1];
  while (left <= middle || right <= end) {
    if (left > middle) {
      pointsWork[counter] = points[right];
      right++;
    } else if (right > end) {
      pointsWork[counter] = points[left];
      left++;
    } else if (this->computeAngle(points[left]) < this->computeAngle(points[right])) {
      pointsWork[counter] = points[left];
      left++;
    } else {
      if (this->computeAngle(points[left]) > this->computeAngle(points[right])) {
        pointsWork[counter] = points[right];
        right++;
      } else {
        // if angles are equal then the point with the smaller xCoord is deleted
        double xValueLeft = points[left].getCoord().getValueInDim(0);
        double xValueRight = points[right].getCoord().getValueInDim(0);
        if (xValueLeft < xValueRight) {
          // delete left point
          points[left].nullPoint();
          pointsWork[counter] = points[left];
          left++;
        } else {
          // delete right point
          points[right].nullPoint();
          pointsWork[counter] = points[right];
          right++;
        }
      }
    }
    counter++;
  }
  for (int i = 0; i < end - start + 1; i++) {
    points[start + i] = pointsWork[i];
  }
  delete[] pointsWork;
}

bool Point::isLeft(Point start, Point end) {
  // returns true if this is left from the vector start -> end
  double xStart = start.coord.getValueInDim(0);
  double yStart = start.coord.getValueInDim(1);
  double xEnd = end.coord.getValueInDim(0);
  double yEnd = end.coord.getValueInDim(1);
  double xThis = this->coord.getValueInDim(0);
  double yThis = this->coord.getValueInDim(1);
  double leftFrom = (xEnd - xStart) * (yThis - yStart) - (xThis - xStart) * (yEnd - yStart);
  return (leftFrom > 0.0);
}
