#pragma once

#include <vector>

#define MAX_SAVE_ACTIONS 100

namespace BuildOrderSearch
{
class SearchSaveState
{

	int		depth;
	int		upperBound;
	int		currentActions[MAX_SAVE_ACTIONS];

public:

	SearchSaveState() : depth(0) {}

	SearchSaveState(std::vector<unsigned char> buildOrder, int ub) : upperBound(ub)
	{
		// set the depth equal to the size of the build order vector
		depth = buildOrder.size();
	
		// copy the values of the vector into the array (backwards due to buildorder vector being backwards)
		for (size_t i(0); i<buildOrder.size(); ++i)
		{ 
			currentActions[i] = buildOrder[buildOrder.size()-1-i];
		}
	}
	
	int getUpperBound() const
	{
		return upperBound;
	}

	int operator [] (const int index) const
	{
		return currentActions[index];
	}
	
	int getDepth() const
	{
		return depth;
	}

	int getAction(const int d) const
	{
		assert(d < depth);
	
		return currentActions[d];
	}

	void print() const
	{
		printf("Depth(%d) UpperBound(%d) Actions ", depth, upperBound);
			
		for (int i(0); i<depth; ++i)
		{
			printf("%d ", currentActions[i]);
		}
		
		printf("\n");
	}
};
}