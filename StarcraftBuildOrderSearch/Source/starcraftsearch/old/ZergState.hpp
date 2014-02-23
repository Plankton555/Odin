#ifndef ZERG_STATE_H
#define ZERG_STATE_H

#include <string.h>
#include <queue>
#include <algorithm>

#include "BWAPI.h"
#include "ActionSet.hpp"
#include "StarcraftAction.hpp"
#include "StarcraftData.hpp"
#include "StarcraftState.hpp"

class ZergState : public StarcraftState
{

	Hatchery	hatcheries[MAX_HATCHERIES];		// array of hatchery statuses
	int			numHatcheries, numLarva;

public:


	ZergState(bool init) 
	{
		// only call this for 'new' StarcraftStates, others will have bitmasks set on add
		if (init) 
		{
			actionPerformed = (Action)-1; 
			parent = NULL; 
			currentFrame = 0;
			mineralWorkers = 0; 
			gasWorkers = 0; 
			
			for (int i=0; i<MAX_ACTIONS; ++i)
			{
				numUnits[i] = 0;
			}
			
			setGameStartData();
			setUnitMask(); 
			setProgressMask();
		}
	}
	
	ZergState()
	{
		actionPerformed = (Action)-1; 
		parent = NULL;

		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			numUnits[i] = 0;
		}
	}

	~ZergState() {}

	virtual bool specificIsLegal(Action a)
	{
		if (GSN_DEBUG) printf("\tSpecific When Ready %d\n", a);
		return true;
	}

	virtual void setGameStartData()
	{
		// starting resources and supply
		minerals = 50; 
		gas = 0; 
		maxSupply = 18; 
		currentSupply = 8;
		
		buildings.addBuilding(DATA.getAction(BWAPI::UnitTypes::Zerg_Hatchery));

		// starting units
		setNumUnits(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), 4);
		setNumUnits(DATA.getAction(BWAPI::UnitTypes::Zerg_Hatchery), 1);

		mineralWorkers = 4;

		// hatchery status	
		numHatcheries = 1;
		numLarva = 3;
		hatcheries[0] = Hatchery(3,0);
	}

	// zerg specific, when will larva be ready, if we need one
	virtual FrameCountType specificWhenReady(Action action)
	{
		if (GSN_DEBUG) printf("\tSpecific When Ready\n");
		FrameCountType l = currentFrame;		
	
		// if the current action is a unit
		if (DATA[action].isUnit()) 
		{
			// if we don't have any larva ready, figure out when it will pop
			if (numLarva == 0)
			{
				if (GSN_DEBUG) printf("\t\tZerg Larva Not Ready\n");

				// min value placeholder
				int min = 88888888;

				// figure out the hatchery with the least recent used larva
				for (int i(0); i<numHatcheries; ++i) 
				{
					// set the minimum accordingly
					min = hatcheries[i].lastThirdUsed < min ? hatcheries[i].lastThirdUsed : min;
				}

				l = min + ZERG_LARVA_TIMER + 5;
			}
		}

		if (GSN_DEBUG) printf("\tSpecific When Ready End (return %d)\n", l);
		return l;
	}

	virtual void specificDoAction(const Action action, const FrameCountType ffTime)
	{
		if (GSN_DEBUG) printf("\tSpecific Do Action %d\n", ffTime);

		// zerg must subtract a larva if the action was unit creation
		if (DATA[action].isUnit() && !DATA[action].isBuilding()) 
		{		
			if (GSN_DEBUG) printf("\t\tZerg Unit - Remove Larva\n");
			// subtract the larva
			numLarva--;
		
			// for each of our hatcheries
			for (int i(0); i<numHatcheries; ++i) 
			{			
				// if this hatchery has a larva available
				if (hatcheries[i].numLarva > 0) 
				{
					// if there were 3 larva previously, update lastUsed
					if (hatcheries[i].numLarva == 3) hatcheries[i].lastThirdUsed = currentFrame;

					// subtract it from this hatchery
					hatcheries[i].numLarva--;
				
					// break out of the hatchery loop
					break;
				}
			}
		}
		else if (DATA[action].isBuilding())
		{
			if (GSN_DEBUG) printf("\t\tZerg Building - Remove Drone\n");
			addNumUnits(DATA.getWorker(), -1);
			addMineralWorkers(-1);
			currentSupply -= DATA[DATA.getWorker()].supplyRequired();
		}

		if (GSN_DEBUG) printf("\tSpecific Do Action End\n");
	}

	// zerg specific fast forwarding needs to determine number of larva
	virtual void specificFastForward(const int toFrame)
	{
		if (GSN_DEBUG) printf("\t\tSpecific Fast Forward\n");

		// for each hatchery we have
		for (int i(0); i<numHatcheries; ++i) 
		{	
			// only do this if there's less than 3 larva present
			if (hatcheries[i].numLarva < 3) 
			{
				// how much time will have past since the last larva used
				FrameCountType diff = toFrame - hatcheries[i].lastThirdUsed;

				// figure out how many larva should have spawned
				UnitCountType numLarvaSpawn = (UnitCountType)(diff / ZERG_LARVA_TIMER);

				// cut that number down to make sure no more than 3 total exist
				if (numLarvaSpawn > (3 - hatcheries[i].numLarva)) numLarvaSpawn = 3 - hatcheries[i].numLarva;

				// add that to the number of larva
				hatcheries[i].numLarva += numLarvaSpawn;
				numLarva += numLarvaSpawn;

				// if it's less than 3, update the timer ticker
				if (hatcheries[i].numLarva < 3) 
				{
					hatcheries[i].lastThirdUsed += (numLarvaSpawn * ZERG_LARVA_TIMER);
				}
			
				// if it's exactly 3, set it to toFrame (this might not even matter)
				if (hatcheries[i].numLarva == 3) 
				{
					hatcheries[i].lastThirdUsed = toFrame;
				}

				//printf("Num Larva Spawn: %d %d\n", numLarvaSpawn, hatcheries[i].numLarva);
			} 
			else 
			{
				hatcheries[i].lastThirdUsed = toFrame;
			}	
		}

		if (GSN_DEBUG) printf("\t\tSpecific Fast Forward End\n");
	}

	virtual void specificFinishAction(const Action action) 
	{
		if (GSN_DEBUG) printf("\tSpecific Finish Action %d\n", action);

		// TODO: if it's a hatchery, add a new one and update larva... etc

		if (GSN_DEBUG) printf("\tSpecific Finish Action End\n");
	}
};

#endif
