#include "BWAPI.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "DependencyGraph.hpp"
#include "DFBBStarcraftSearch.hpp"
#include "StarcraftSearchConstraint.hpp"
#include "SmartStarcraftSearch.hpp"
#include "MacroActionBuilder.hpp"
#include "ArmySearch.hpp"
#include "SmartArmySearch.hpp"

using namespace BuildOrderSearch;

// returns a sample protoss goal
StarcraftSearchGoal defaultProtossGoal()
{
	// the goal itself
	StarcraftSearchGoal goal;
	
	// the things we want to limit in the search
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Nexus), 1);
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Cybernetics_Core), 1);
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Assimilator), 1);
	//goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Citadel_of_Adun), 1);
	//goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Templar_Archives), 1);
	//goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Robotics_Facility), 1);
	//goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Fleet_Beacon), 1);
	//goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Protoss_Arbiter_Tribunal), 1);
	
	// return the goal!
	return goal;
}

// returns a sample protoss goal
StarcraftSearchGoal defaultZergGoal()
{
	// the goal itself
	StarcraftSearchGoal goal;
	
	// the things we want to limit in the search
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Spawning_Pool), 1);
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Extractor), 1);
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Lair), 1);
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Spire), 1);
	goal.setGoalMax(DATA.getAction(BWAPI::UnitTypes::Zerg_Hydralisk_Den), 1);
	
	// return the goal!
	return goal;
}

StarcraftState openingBookStateProtoss()
{
	StarcraftState state(true);
	
	Action probe = DATA.getWorker();
	Action pylon = DATA.getSupplyProvider();
	//Action gateway = DATA.getAction(BWAPI::UnitTypes::Protoss_Gateway);
	
	//state.doAction(probe, state.resourcesReady(probe));
	//state.doAction(probe, state.resourcesReady(probe));
	//state.doAction(probe, state.resourcesReady(probe));
	//state.doAction(probe, state.resourcesReady(probe));
	//state.doAction(pylon, state.resourcesReady(pylon));
	//state.doAction(probe, state.resourcesReady(probe));
	//state.doAction(probe, state.resourcesReady(probe));
	
	return state;
}


void testArmySearch()
{
	//srand(time(NULL));
	
	SearchParameters params;
//	params.initialState = openingBookStateGateway();
	params.initialState = StarcraftState(true);
	params.initialUpperBound = 5000;
	params.goal = defaultProtossGoal();
	params.useAlwaysMakeWorkers = true;
	params.useIncreasingRepetitions = true;
	params.setRepetitionThreshold(DATA.getSupplyProvider(), 1);
	params.setRepetitionThreshold(DATA.getWorker(), 8);
	
	params.setRepetitions( DATA.getAction(BWAPI::UnitTypes::Protoss_Probe), 2);
	params.setRepetitions( DATA.getAction(BWAPI::UnitTypes::Protoss_Pylon), 2);
	params.setRepetitions( DATA.getAction(BWAPI::UnitTypes::Protoss_Gateway), 1);
	params.setRepetitions( DATA.getAction(BWAPI::UnitTypes::Protoss_Zealot), 2);
	params.setRepetitions( DATA.getAction(BWAPI::UnitTypes::Protoss_Dragoon), 2);
	
	params.goal.setGoalMax( DATA.getAction(BWAPI::UnitTypes::Protoss_Probe), 16 );
	params.goal.setGoalMax( DATA.getAction(BWAPI::UnitTypes::Protoss_Gateway), 4 );
	params.goal.setGoalMax( DATA.getAction(BWAPI::UnitTypes::Protoss_Pylon), 4);

	ArmySearch search(params);
	
	SearchResults result = search.search();

    printf("Search took %lf ms\n", result.timeElapsed);
}

void testSmartArmySearch()
{
	SmartArmySearch sas;
	sas.setState(openingBookStateProtoss());
	sas.print();
	SearchResults r = sas.search();
	
	printf("Army Search Took %lfms\n", r.timeElapsed);
}

void testSmartSearch()
{
	// default state
	StarcraftState initialState(true);

	SmartStarcraftSearch sss;
	sss.addGoal(DATA.getAction(BWAPI::UnitTypes::Zerg_Zergling), 16);
	sss.setState(initialState);
	
	// do the search
	SearchResults result = sss.search();
	
	if (result.solved)
	{
		result.printResults(true);
	}
}


void testSingleSearch()
{
	StarcraftState init(true);
	//init.doAction(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), init.resourcesReady(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone)));
	//init.doAction(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), init.resourcesReady(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone)));
	//init.doAction(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), init.resourcesReady(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone)));
	//init.doAction(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), init.resourcesReady(DATA.getAction(BWAPI::UnitTypes::Zerg_Drone)));
	//init.doAction(DATA.getAction(BWAPI::UnitTypes::Zerg_Spawning_Pool), init.resourcesReady(DATA.getAction(BWAPI::UnitTypes::Zerg_Spawning_Pool)));
	//init.doAction(DATA.getAction(BWAPI::UnitTypes::Zerg_Extractor), init.resourcesReady(DATA.getAction(BWAPI::UnitTypes::Zerg_Extractor)));

	init.printData();

	SearchParameters params(defaultZergGoal(), init, StarcraftSearchConstraints());

	//params.goal.setGoal( DATA.getAction(BWAPI::UnitTypes::Zerg_Zergling), 14);
	//params.goal.setGoal( DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), 9);
	params.goal.setGoal( DATA.getAction(BWAPI::UnitTypes::Zerg_Mutalisk), 4);
	params.goal.setGoalMax( DATA.getAction(BWAPI::UnitTypes::Zerg_Overlord), 3);
	params.goal.setGoalMax( DATA.getAction(BWAPI::UnitTypes::Zerg_Drone), 12);

	params.useRepetitions = true;

	DFBBStarcraftSearch SCSearch(params);

	SearchResults result = SCSearch.search();
	
	//if (result.solved)
	//{
		result.printResults(true);
	//}
}


int main()
{
    BWAPI::BWAPI_init();
	DATA.init(BWAPI::Races::Zerg);
	
	testSmartSearch();

    return 0;
}

