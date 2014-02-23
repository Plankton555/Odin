#ifndef STARCRAFT_SEARCH_GOAL_H
#define STARCRAFT_SEARCH_GOAL_H

#include <string.h>
#include <queue>
#include <algorithm>

#include "BWAPI.h"
#include "ActionSet.hpp"
#include "StarcraftAction.hpp"
#include "StarcraftData.hpp"
#include <math.h>

namespace BuildOrderSearch
{
class StarcraftSearchGoal
{

	UnitCountType 		goalUnits[MAX_ACTIONS];
	UnitCountType		goalUnitsMax[MAX_ACTIONS];
	
	SupplyCountType		supplyRequiredVal;

	ResourceCountType	mineralGoal,
						gasGoal;
	
	void calculateSupplyRequired()
	{
		supplyRequiredVal = 0;
		for (int i=0; i<DATA.size(); ++i)
		{
			supplyRequiredVal += goalUnits[i] * DATA[i].supplyRequired();
		}
	}
	
public:	
	
	StarcraftSearchGoal() : supplyRequiredVal(0), mineralGoal(0), gasGoal(0)
	{
		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			goalUnits[i] = 0;
			goalUnitsMax[i] = 0;
		}
	}
	
	StarcraftSearchGoal(const Action a) : supplyRequiredVal(0), mineralGoal(0), gasGoal(0)
	{
		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			goalUnits[i] = 0;
			goalUnitsMax[i] = 0;
		}
		
		goalUnits[a] = 1;
	}
	
	~StarcraftSearchGoal() {}
	
	bool operator == (const StarcraftSearchGoal & g)
	{
		for (Action i=0; i<MAX_ACTIONS; ++i)
		{
			if ((goalUnits[i] != g[i]) || (goalUnitsMax[i] != g.getMax(i)))
			{
				return false;
			}
		}

		return true;
	}

	UnitCountType operator [] (const int a) const
	{
		return goalUnits[a];
	}
	
	void setMineralGoal(const ResourceCountType m)
	{
		mineralGoal = m;
	}
	
	void setGasGoal(const ResourceCountType g)
	{
		gasGoal = g;
	}
	
	ResourceCountType getMineralGoal() const
	{
		return mineralGoal;
	}
	
	ResourceCountType getGasGoal() const
	{
		return gasGoal;
	}

	void setGoal(const Action a, const UnitCountType num)
	{
		assert(a >= 0 && a < DATA.size());
	
		goalUnits[a] = num;
		
		calculateSupplyRequired();
	}
	
	bool hasGoal() const
	{
		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			if (goalUnits[i] > 0)
			{
				return true;
			}
		}
		
		return false;
	}

	void addToGoal(Action a, UnitCountType num)
	{
		assert(a >= 0 && a < DATA.size());
	
		goalUnits[a] += num;
		
		calculateSupplyRequired();
	}
	
	void setGoalMax(const Action a, const UnitCountType num)
	{
		goalUnitsMax[a] = num;
	}
	
	UnitCountType get(const Action a) const
	{
		return goalUnits[a];
	}
	
	UnitCountType getMax(const Action a) const
	{
		return goalUnitsMax[a];
	}
	
	SupplyCountType supplyRequired() const
	{
		return supplyRequiredVal;
	}
	
	void printGoal() const
	{
		printf("\nSearch Goal Information\n\n");
	
		for (int i=0; i<DATA.size(); ++i)
		{
			if (goalUnits[i])
			{
				printf("        REQ %7d %s\n", goalUnits[i], DATA[i].getName().c_str());
			}
		}
		
		for (int i=0; i<DATA.size(); ++i)
		{
			if (goalUnitsMax[i])
			{
				printf("        MAX %7d %s\n", goalUnitsMax[i], DATA[i].getName().c_str());
			}
		}
	}
};
}
#endif
