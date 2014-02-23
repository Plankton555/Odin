#pragma once

#include "BWAPI.h"
#include "ActionSet.hpp"
#include <vector>

namespace BuildOrderSearch
{
class StarcraftSearchConstraint
{

public:

	Action 		action;
	int 		actionCount;
	int			frame;

	StarcraftSearchConstraint(Action a, int c, int f) : action(a), actionCount(c), frame(f) {}
};

class StarcraftSearchConstraints
{

	std::vector<StarcraftSearchConstraint> constraints;

public:

	StarcraftSearchConstraints() {}
	
	void addConstraint(const StarcraftSearchConstraint ssc)
	{
		constraints.push_back(ssc);
	}
	
	int size() const
	{
		return (int)constraints.size();
	}
	
	StarcraftSearchConstraint & getConstraint(int index) const
	{
		assert(index >= 0 && index < size());
	
		return const_cast<StarcraftSearchConstraint &>(constraints[index]);
	}
};
}