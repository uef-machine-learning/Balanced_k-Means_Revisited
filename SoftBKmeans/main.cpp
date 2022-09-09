#include <string>
#include <fstream>
#include <iomanip>
#include <random>
#include <iostream>
#include <chrono>

#include "kMeans.h"

int main(int argc, char* argv[]) {

	//choose data set
	std::string nameDataSet = "S2";
	int dimension = 2;
	int size = 5000;
	int numClusters = 15;

	//setting parameters
	//the termination criterion to be used, already implemented are MaxDiffClusterSize, MinClusterSize, MaxSDCS and MinNormEntro
	KMeans::TerminationCriterion terminationCriterion = KMeans::TerminationCriterion::MaxDiffClusterSizes;
	//the value that should be reached by the termination criterion
	//a balanced clustering corresponds to maximum difference in cluster sizes as termination criterion and terminationCriterion = 1, balanced is not 0!
	double terminationCriterionValue = 50.0;
	bool stopWhenBalanced = false;
	double partlyRemainingFraction = 0.15; //0 < partlyremainingFraction < 1
	double increasingPenaltyFactor = 1.01; //increasingPenaltyFactor >= 1
	bool useFunctionIter = true;
	int numRuns = 1;
	bool reproducible = false;
	bool graphicalRepresentation = true;
	
	//set random seeds for the runs
	if (reproducible) {
		srand(7843); //any other int can be chosen
	}
	else {
		srand((int)time(NULL));
	}
	int* seeds = new int[numRuns];
	for (int i = 0; i < numRuns; i++) {
		seeds[i] = rand();
	}

	//prepare files for the results
	std::string start = "results\\" + nameDataSet;
	std::string end = ".txt";
	std::fstream summary;
	summary.open(start + "_summary" + end, std::ios::out);
	summary.close();

	//run kMeans numRuns times
	for (int run = 0; run < numRuns; run++) {

		std::cout << "run number" << run << "\n";

		//create a kMeans instance
		KMeans kMeans;
		kMeans.initialize(size, dimension, nameDataSet, numClusters, seeds[run]);
		//solve the kMeans instance
		auto startTime = std::chrono::high_resolution_clock::now();
		kMeans.run(terminationCriterion, terminationCriterionValue, stopWhenBalanced, partlyRemainingFraction, increasingPenaltyFactor, useFunctionIter);
		auto endTime = std::chrono::high_resolution_clock::now();
		double time = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
		//save assignments in file
		std::string filename = start + "_assignments_" + std::to_string(run) + end;
		std::fstream assignments;
		assignments.open(filename, std::ios::out);
		kMeans.writeAssignments(assignments);
		assignments.close();
		//save times in summary file
		summary.open(start + "_summary" + end, std::fstream::app);
		summary << time << std::endl;
		summary.close();
		if (graphicalRepresentation && dimension == 2) {
			kMeans.showResultsConvexHull(nameDataSet, run, time);
		}

	}

}
