#ifndef BUILDING_DATA_H
#define BUILDING_DATA_H

#include <string.h>
#include <queue>
#include <algorithm>

#include "BWAPI.h"
#include "ActionSet.hpp"
#include "StarcraftAction.hpp"
#include "StarcraftData.hpp"
#include "StarcraftState.hpp"

namespace BuildOrderSearch
{
class BuildingStatus
{

public:

	// the type of unit this is
	Action type;
	
	// the number of frames remaining (from currentFrame) until this building is free
	FrameCountType timeRemaining;

	BuildingStatus() : type(0), timeRemaining(0) 
	{
		memset(this, 0, sizeof(*this));
	}
	
	~BuildingStatus() {}
	BuildingStatus(Action t) : type(t), timeRemaining(0) {}
	BuildingStatus(Action t, FrameCountType time) : type(t), timeRemaining(time) {}
};

class BuildingData
{

	BuildingStatus buildings[MAX_BUILDINGS];

public:

	int numBuildings;

	BuildingData() : numBuildings(0) {}

	void addBuilding(const Action t)
	{
		assert(DATA[t].isBuilding());
		assert(numBuildings < (MAX_BUILDINGS - 1));
	
		buildings[numBuildings++] = BuildingStatus(t, 0);
	}

	void addBuilding(const Action t, const FrameCountType timeUntilFree)
	{
		assert(DATA[t].isBuilding());
		assert(numBuildings < (MAX_BUILDINGS - 1));
	
		buildings[numBuildings++] = BuildingStatus(t, timeUntilFree);
	}

	const BuildingStatus & getBuilding(int i) const
	{
		assert(i >= 0 && i < MAX_BUILDINGS);
		
		return buildings[i];
	}
	
	// get the index of the first free building of type t
	int getFreeBuilding(const Action t) const
	{
		// for each building
		for (int i=0; i<numBuildings; ++i)
		{
			if (buildings[i].type == t && buildings[i].timeRemaining == 0)
			{
				return i;
			}
		}
		
		return BUILDING_ERROR;
	}
	
	// gets the time until building of type t is free
	// this will only ever be called if t exists, so min will always be set to a lower value
	FrameCountType timeUntilFree(const Action t) const
	{
		FrameCountType min = 99999;
	
		for (int i=0; i<numBuildings; ++i)
		{
			if (buildings[i].type == t)
			{
				min = (buildings[i].timeRemaining < min) ? buildings[i].timeRemaining : min;
			}
		}
		
		assert(min != 9999999);
	
		return min;
	}

	// queue an action
	void queueAction(const Action a)
	{	
		// for each building
		for (int i=0; i<numBuildings; ++i)
		{
		
			// if the building is free and of the correct type
			if (buildings[i].type == DATA[a].whatBuildsAction() && buildings[i].timeRemaining == 0)
			{
				// queue it here
				buildings[i].timeRemaining = (unsigned short)DATA[a].buildTime();
				//printf("\t\t%s is ready! Using it for %s.\n", DATA[buildings[i].type].getName().c_str(), DATA[a].getName().c_str());
				return;
			}
		}
		
		printf("Assert Incoming: %s which needed %s but didn't find it.\n", DATA[a].getName().c_str(), DATA[DATA[a].whatBuildsAction()].getName().c_str());
		printBuildingInformation();
		
		// this method should always work since we have fast forwarded to the correct point in time
		assert(false);
	}
	
	void fastForwardBuildings(const FrameCountType frames)
	{
		for (int i=0; i<numBuildings; ++i)
		{
			buildings[i].timeRemaining = ((buildings[i].timeRemaining - frames) > 0) ? (buildings[i].timeRemaining - frames) : 0;
			if (GSN_DEBUG && frames > 0) printf("\t\t\tBuilding %s reduced to %d frames remaining\n", DATA[buildings[i].type].getName().c_str(), buildings[i].timeRemaining);
		}
	}
	
	void printBuildingInformation() const
	{
		for (int i=0; i<numBuildings; ++i)
		{
			if (buildings[i].timeRemaining == 0) 
			{
				printf("BUILDING INFO: %s is free to assign\n", DATA[buildings[i].type].getName().c_str());
			}
			else 
			{
				printf("BUILDING INFO: %s will be free in %d frames\n", DATA[buildings[i].type].getName().c_str(), buildings[i].timeRemaining);
			}
		}
		
		printf("-----------------------------------------------------------\n\n");
	}
};
}
#endif
