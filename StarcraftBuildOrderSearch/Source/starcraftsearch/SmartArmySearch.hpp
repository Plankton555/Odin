#pragma once

#include "DependencyGraph.hpp"
#include "ArmySearch.hpp"
#include "StarcraftSearchConstraint.hpp"
#include "Timer.hpp"

namespace BuildOrderSearch
{
class SmartArmySearch
{
	SearchParameters				params;
	
	int 							searchTimeLimit;

	Timer							searchTimer;
	
	void calculateSearchSettings()
	{
		// set the max number of resource depots to what we have since no expanding is allowed
		params.goal.setGoalMax(DATA.getResourceDepot(), (UnitCountType)params.initialState.getNumUnits(DATA.getResourceDepot()));
		
		// set the number of refineries
		params.goal.setGoalMax(DATA.getRefinery(), calculateRefineriesRequired());	
		
		// set the maximum number of workers to an initial ridiculously high upper bound
		params.goal.setGoalMax(DATA.getWorker(), calculateWorkerUpperBound());
		
		// set the number of supply providers required
		params.goal.setGoalMax(DATA.getSupplyProvider(), calculateSupplyProvidersRequired());
		
		// set the maximums for all goal prerequisites
		setBuildingMax();
		
		// set the repetitions
		setRepetitions();
		
		params.useRepetitions 				= true;
		params.useIncreasingRepetitions 	= true;
		params.useAlwaysMakeWorkers 		= true;
		params.initialUpperBound 			= params.initialState.getCurrentFrame() + 4000;
	}
	
	// calculates maximum number of refineries we'll need
	UnitCountType calculateRefineriesRequired()
	{
		UnitCountType numDepots = params.initialState.getNumUnits(DATA.getResourceDepot());
		UnitCountType numRefineries = params.initialState.getNumUnits(DATA.getRefinery()) + 1;
	
		return std::min(numDepots, numRefineries);
	}
	
	// handles all goalMax calculations for prerequisites of goal actions
	void setBuildingMax()
	{
		// set a maximum of one for each tech type prerequisite building
		for (Action a(0); a<DATA.size(); ++a)
		{
			// if it's a building that can't produce or attack
			if (DATA[a].isBuilding() && !DATA[a].canProduce() && !DATA[a].canAttack() && !(DATA[a].supplyProvided() > 0))
			{
				// it's a research building so limit our search to one of these
				params.goal.setGoalMax(a, 1);
			}
		}
		
		// set a maximum for each producing structure
		for (Action a(0); a<DATA.size(); ++a)
		{
			// if it is a non resource depot building that can produce 
			if (DATA[a].isBuilding() && DATA[a].canProduce() && !DATA[a].isResourceDepot())
			{
				// limit ourselves to 4 more of this building
				params.goal.setGoalMax(a, params.initialState.getNumUnits(a) + 4);
			}
		}
	}
	
	
	const UnitCountType calculateSupplyProvidersRequired() const
	{
		return (UnitCountType)params.initialState.getNumUnits(DATA.getSupplyProvider()) + 4;
	}
	
	const UnitCountType calculateWorkerUpperBound() const
	{
		return (UnitCountType)std::min(params.initialState.getWorkerCount() + 15, 100);
	}
	
	void setRepetitions()
	{
		// build supply providers in 2's after the first
		params.setRepetitions(DATA.getSupplyProvider(), 2);
		params.setRepetitionThreshold(DATA.getSupplyProvider(), 1);
		
		// build workers in 2s
		params.setRepetitions(DATA.getWorker(), 2);

		// for each action
		for (Action a = 0; a < DATA.size(); ++a)
		{
			// if it's a non depot producing building build in 2s after the first
			if (DATA[a].canProduce() && !DATA[a].isResourceDepot())
			{
				params.setRepetitions(a, 2);
				params.setRepetitionThreshold(a, 1);
			}
		}

	}
	
public:

	SmartArmySearch () {}
	~SmartArmySearch() {}
	
	void setState(const StarcraftState & s)
	{
		params.initialState = s;
		
		calculateSearchSettings();
	}
	
	void setConstraints(const StarcraftSearchConstraints & s)
	{
		params.ssc = s;
	}

	void addConstraint(StarcraftSearchConstraint & s)
	{
		params.ssc.addConstraint(s);
	}
	
	void setTimeLimit(int n)
	{
		searchTimeLimit = n;
	}
	
	SearchResults search()
	{		
		// construct the search object
		ArmySearch army(params);

		// run the search 
		SearchResults result = army.search();
		
		// return the results object
		return result;
	}
	
	void print()
	{
		params.initialState.printData();
		params.print();
		printf("\n\n");
	}
	
};

}