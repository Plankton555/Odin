#pragma once

#include "DependencyGraph.hpp"
#include "DFBBStarcraftSearch.hpp"
#include "StarcraftSearchConstraint.hpp"
#include "Timer.hpp"

namespace BuildOrderSearch
{
class SmartStarcraftSearch
{
	SearchParameters				params;

	StarcraftSearchGoal 			goal;
	StarcraftState					initialState;
	StarcraftSearchConstraints		ssc;
	
	int 							searchTimeLimit;

	Timer							searchTimer;
	
	SearchResults doSearch()
	{	
		params.goal = goal;
		params.initialState = initialState;
		params.ssc = ssc;
		params.useRepetitions 				= true;
		params.useIncreasingRepetitions 	= true;
		params.useAlwaysMakeWorkers 		= true;
		params.useSupplyBounding 			= true;
		//params.useConstraints				= true;
		
		//params.print();
	
		// construct the search object
		DFBBStarcraftSearch dfbb(params);

		// run the search 
		SearchResults result = dfbb.search();
		
		// return the results object
		return result;
	}
	
	void calculateSearchSettings()
	{
		// set the max number of resource depots to what we have since no expanding is allowed
		goal.setGoalMax(DATA.getResourceDepot(), (UnitCountType)initialState.getNumUnits(DATA.getResourceDepot()) + 1);
		
		// set the number of refineries
		goal.setGoalMax(DATA.getRefinery(), (UnitCountType)std::max(3, calculateRefineriesRequired()));	
		
		// set the maximum number of workers to an initial ridiculously high upper bound
		goal.setGoalMax(DATA.getWorker(), (UnitCountType)std::min(initialState.getWorkerCount() + 15, 100));
		
		// set the number of supply providers required
		goal.setGoalMax(DATA.getSupplyProvider(), (UnitCountType)calculateSupplyProvidersRequired());
		
		// set the maximums for all goal prerequisites
		setPrerequisiteGoalMax();
		
		// set the repetitions
		setRepetitions();
	}
	
	// calculates maximum number of refineries we'll need
	int calculateRefineriesRequired()
	{
		// will gas be required
		bool gasRequired(false);

		if (goal[DATA.getRefinery()])
		{
			return goal[DATA.getRefinery()];
		}
	
		// loop to check if we need gas
		for (int a = 0; a < DATA.size(); ++a)
		{		
			if (goal[a] > 0 && DATA[a].gasPrice() > 0)
			{				
				gasRequired = true;
				break;
			}
		}
		
		// if we need gas return the number of resource depots
		if (gasRequired)
		{
			return initialState.getNumResourceDepots();
		}
		// otherwise we don't need any refineries
		// unless the goal specifically asks for it
		else
		{
			return 0;
		}
	}
	
	// handles all goalMax calculations for prerequisites of goal actions
	void setPrerequisiteGoalMax()
	{
		if (DATA.getRace() == BWAPI::Races::Protoss || DATA.getRace() == BWAPI::Races::Terran)
		{
			// for each unit in the goal vector
			for (Action a = 0; a < DATA.size(); ++a)
			{		
				// if we want one of these
				if (goal[a] > 0)
				{				
					// set goalMax for each strict dependency equal to 1
					recurseOverStrictDependencies(a);
				}
			}		
		
			// vector which stores the number of goal units which are built by [index]
			std::vector<int> numGoalUnitsBuiltBy(DATA.size(), 0);
			
			// for each unit in the goal vector
			for (Action a = 0; a < DATA.size(); ++a)
			{		
				// if we want one of these
				if (goal[a] > 0)
				{				
					// add this to the sum
					numGoalUnitsBuiltBy[DATA[a].whatBuildsAction()] += goal[a]; 

					// if it's in the goal, max sure it's in the max
					UnitCountType max = goal.getMax(a);
					goal.setGoalMax(a, (unsigned char)std::max(goal[a], max));
				}
			}
			
			int productionBuildingLimit = 8;

			// now, for each unit in that vector
			for (Action a = 0; a < DATA.size(); ++a)
			{
				// if it's not a resource depot
				if (!DATA[a].isResourceDepot() && DATA[a].isBuilding())
				{
					// if this building produces units
					if (numGoalUnitsBuiltBy[a] > 0)
					{				
						// set the goal max to how many units
						goal.setGoalMax(a, (UnitCountType)std::min(productionBuildingLimit, numGoalUnitsBuiltBy[a]));
					}
				}
			}
		}
		else if (DATA.getRace() == BWAPI::Races::Zerg)
		{
			goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Spawning_Pool), 1);
			goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Extractor), 1);
			goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Lair), 1);
			goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Spire), 1);
			goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Hydralisk_Den), 1);
		}
		
	}
	
	// recursively checks the tech tree of Action and sets each to have goalMax of 1
	void recurseOverStrictDependencies(Action action)
	{
		if (DATA[action].isResourceDepot() || DATA[action].isWorker() || DATA[action].isSupplyProvider())
		{
			return;
		}
	
		// get the strict dependencies of this action
		ActionSet strictDependencies = DATA.getStrictDependency(action);
		
		// for each strict dependency
		while (!strictDependencies.isEmpty())
		{
			// get the next action from this dependency
			Action p = strictDependencies.popAction();
			
			if (DATA[p].isResourceDepot() || DATA[p].isWorker() || DATA[p].isSupplyProvider())
			{
				return;
			}
			
			//printf("Setting %s max = 1\n", DATA[p].getName().c_str());
			
			// set goalMax to the maximum of what's already there, or 1
			int max = goal.getMax(p);
			goal.setGoalMax(p, (unsigned char)std::max(1, max)); 
			
			// recurse down			
			recurseOverStrictDependencies(p);
		}
	}
	
	int calculateSupplyProvidersRequired()
	{
		// calculate the upper bound on supply for this goal
		int supplyNeeded = goal.getMax(DATA.getWorker()) * DATA[DATA.getWorker()].supplyRequired();
		
		// for each prerequisite of things in the goal which aren't production facilities set one of
		for (int a = 0; a < DATA.size(); ++a)
		{
			// add the supply required for this number of goal units and all units currently made
			supplyNeeded += std::max(goal[a], initialState.getNumUnits(a)) * DATA[a].supplyRequired();
		}
		
		// set the upper bound on supply based on these values
		int supplyFromResourceDepots = initialState.getNumResourceDepots() * DATA[DATA.getResourceDepot()].supplyProvided();
		
		// take this away from the supply needed
		supplyNeeded -= supplyFromResourceDepots;
		
		// return the number of supply providers required
		return supplyNeeded > 0 ? (int)ceil( (double)supplyNeeded / (double)DATA[DATA.getSupplyProvider()].supplyProvided() ) : 0;
	}
	
	int calculateWorkerUpperBound()
	{
		int goalMinerals(0), goalGas(0);
		
		for (int a = 0; a < DATA.size(); ++a)
		{
			// what's the most of this we will have
			int numAction = std::max(goal[a], goal.getMax(a));
	
			if (numAction > 0)
			{
				goalMinerals += numAction * DATA[a].mineralPrice();
				goalGas      += numAction * DATA[a].gasPrice();
			}
		}
	
		std::pair<int,int> result = initialState.resourceReadyLowerBound(goalMinerals, goalGas);
	
		printf("Num Workers: %d\n", result.second);
	
		return result.second;
	}
	
	void setRepetitions()
	{
		params.setRepetitions(DATA.getSupplyProvider(), 2);
		params.setRepetitionThreshold(DATA.getSupplyProvider(), 1);
		params.setRepetitions(DATA.getWorker(), 2);

		// for each action
		for (Action a = 0; a < DATA.size(); ++a)
		{
			// if if want 4 or more of something that isn't supply providing
			if (!DATA[a].isSupplyProvider() && goal[a] >= 5)
			{
				// set the repetitions to half of the value
				params.setRepetitions(a, std::min((UnitCountType)4, goal[a]));
				params.setRepetitions(DATA[a].whatBuildsAction(), 2);
				params.setRepetitionThreshold(DATA[a].whatBuildsAction(), 1);
			}
		}

		params.setRepetitions(DATA.getWorker(), 2);
	}
	
public:

	SmartStarcraftSearch () {}
	~SmartStarcraftSearch() {}
	
	void addGoal(Action a, UnitCountType count)
	{
		goal.setGoal(a, count);
	}
	
	void setGoal(const StarcraftSearchGoal & g)
	{
		goal = g;
	}
	
	void setState(const StarcraftState & s)
	{
		initialState = s;
	}
	
	void setConstraints(const StarcraftSearchConstraints & s)
	{
		ssc = s;
	}

	void addConstraint(StarcraftSearchConstraint & s)
	{
		ssc.addConstraint(s);
	}
	
	void setTimeLimit(int n)
	{
		searchTimeLimit = n;
	}
	
	SearchResults search()
	{
		calculateSearchSettings();
		
		print();
		
		return doSearch();
	}

	SearchParameters getParameters()
	{
		calculateSearchSettings();

		params.goal = goal;
		params.initialState = initialState;
		params.ssc = ssc;
		params.useRepetitions 				= true;
		params.useIncreasingRepetitions 	= true;
		params.useAlwaysMakeWorkers 		= true;
		params.useSupplyBounding 			= true;

		return params;
	}
	
	void print()
	{
		//initialState.printData();
		goal.printGoal();
		printf("\n\n");
	}
	
};

}