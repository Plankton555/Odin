//
// SearchResults.hpp
//
// This class is used to store results of StarcraftSearch search instances.
//
// All of its member variables are public for ease of access, nothing here is critical.
//

#pragma once

#include <vector>
#include "SearchSaveState.hpp"

namespace BuildOrderSearch
{
class SearchResults
{

public:

	bool 				solved,			// whether ot not a solution was found
						timedOut;		// did the search time-out?
	
	int 				solutionLength,	// the length of the solution
						upperBound,		// upper bound of first node
						lowerBound;		// lower bound of first node
	
	unsigned long long 	nodesExpanded;	// number of nodes expanded in the search
	
	double 				timeElapsed,	// time elapsed in milliseconds
						avgBranch;		// avg branching factor
	
	std::vector<Action> buildOrder;		// the build order
	
	int					minerals,
						gas;
						
	SearchSaveState		saveState;

	int					frameCompleted;
	
	SearchResults() 
		: 	solved(false), timedOut(false), 
			solutionLength(-1), upperBound(-1), lowerBound(-1), 
			nodesExpanded(0), timeElapsed(0), avgBranch(0), minerals(0), gas(0), frameCompleted(0) {}
	
	SearchResults(bool s, int len, unsigned long long n, double t, std::vector<Action> solution) 
		: 	solved(s), timedOut(false), 
			solutionLength(len), upperBound(0), lowerBound(0), 
			nodesExpanded(n), timeElapsed(t),  avgBranch(0), buildOrder(solution), minerals(0), gas(0), frameCompleted(0) {}
		
	
	void printResults(bool pbo = true)
	{				
		printf("%12d%12d%12d%14llu%12.4lf%12d       ", upperBound, lowerBound, solutionLength, nodesExpanded, avgBranch, (int)timeElapsed);
		
		if (pbo)
		{
			printBuildOrder();
		}
		
		printf("\n");
	}
	
		
	void printBuildOrder()
	{
		for (size_t i(0); i<buildOrder.size(); ++i)
		{ 
			printf("%d ", buildOrder[buildOrder.size()-1-i]);
		}
	}
};
}