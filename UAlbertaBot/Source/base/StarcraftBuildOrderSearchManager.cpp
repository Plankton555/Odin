#include "Common.h"
#include "StarcraftBuildOrderSearchManager.h"

typedef std::map<MetaType, UnitCountType> mapType;

// get an instance of this
StarcraftBuildOrderSearchManager & StarcraftBuildOrderSearchManager::Instance() 
{
	static StarcraftBuildOrderSearchManager instance;
	return instance;
}

// constructor
StarcraftBuildOrderSearchManager::StarcraftBuildOrderSearchManager() 
	: lastSearchFinishTime(0)
{
	BuildOrderSearch::getStarcraftDataInstance().init(BWAPI::Broodwar->self()->getRace());
}

void StarcraftBuildOrderSearchManager::update(double timeLimit)
{
	starcraftSearchData.update(timeLimit);
}

// function which is called from the bot
std::vector<MetaType> StarcraftBuildOrderSearchManager::findBuildOrder(const std::vector< std::pair<MetaType, UnitCountType> > & goalUnits)
{
	return getMetaVector(search(goalUnits));
}

// function which does all the searching
BuildOrderSearch::SearchResults StarcraftBuildOrderSearchManager::search(const std::vector< std::pair<MetaType, UnitCountType> > & goalUnits)
{	
	// construct the Smart Starcraft Search
	BuildOrderSearch::SmartStarcraftSearch sss;

	// set the goal based on the input MetaType vector
	sss.setGoal(getGoal(goalUnits));

	// set the initial state to the current state of the BW game
	sss.setState(getCurrentState());

	// get the parameters from the smart search
	BuildOrderSearch::SearchParameters params = sss.getParameters();

	// pass this goal into the searcher
	starcraftSearchData.startNewSearch(params);

	// do the search and store the results
	BuildOrderSearch::SearchResults result = starcraftSearchData.getResults();

	if (result.solved)
	{
		lastSearchFinishTime = result.solutionLength;
		//BWAPI::Broodwar->printf("%12d[opt]%9llu[nodes]%7d[ms]", result.solutionLength, result.nodesExpanded, (int)result.timeElapsed);
	}
	else
	{
		//BWAPI::Broodwar->printf("No solution found!");
		//BWAPI::Broodwar->printf("%12d%12d%12d%14llu", result.upperBound, result.lowerBound, 0, result.nodesExpanded);
	}

	return result;
}


// gets the StarcraftState corresponding to the beginning of a Melee game
BuildOrderSearch::StarcraftState StarcraftBuildOrderSearchManager::getStartState()
{
	return BuildOrderSearch::StarcraftState(true);
}

// parses current BWAPI game state and turns it into a StarcraftState of given type
BuildOrderSearch::StarcraftState StarcraftBuildOrderSearchManager::getCurrentState()
{
	BuildOrderSearch::StarcraftState s;
	s.setFrame((unsigned short)BWAPI::Broodwar->getFrameCount());
	s.setSupply(BWAPI::Broodwar->self()->supplyUsed(), BWAPI::Broodwar->self()->supplyTotal());
	s.setResources(BWAPI::Broodwar->self()->minerals(), BWAPI::Broodwar->self()->gas());
	s.setMineralWorkers((unsigned char)WorkerManager::Instance().getNumMineralWorkers());
	s.setGasWorkers((unsigned char)WorkerManager::Instance().getNumGasWorkers());

	int numLarva(0);

	// for each unit we have
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Larva)
		{
			numLarva++;
			continue;
		}

		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			continue;
		}

		BuildOrderSearch::Action action(DATA.getAction(unit->getType()));

		// if the unit is completed
		if (unit->isCompleted())
		{
			// add the unit to the state
			s.addNumUnits(action, 1);

			// if it is a building
			if (unit->getType().isBuilding())
			{
				// add the building data accordingly
				FrameCountType trainTime = unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery ? 0 : unit->getRemainingTrainTime();
				s.addBuilding(action, (FrameCountType)(trainTime + unit->getRemainingResearchTime() + unit->getRemainingUpgradeTime()));

				if (unit->getRemainingResearchTime() > 0)
				{
					s.addActionInProgress(DATA.getAction(unit->getTech()), BWAPI::Broodwar->getFrameCount() + unit->getRemainingTrainTime());
				}
				else if (unit->getRemainingUpgradeTime() > 0)
				{
					s.addActionInProgress(DATA.getAction(unit->getUpgrade()), BWAPI::Broodwar->getFrameCount() + unit->getRemainingTrainTime());
				}

				if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery)
				{
					// TODO: fix the timing
					s.addHatchery(unit->getLarva().size(), BWAPI::Broodwar->getFrameCount());//unit->getRemainingTrainTime());
				}
			}
		}

		// if the unit is under construction (building)
		if (unit->isBeingConstructed())
		{
			s.addActionInProgress(DATA.getAction(unit->getType()), BWAPI::Broodwar->getFrameCount() + unit->getRemainingBuildTime());
		}
	}

	s.setNumLarva(numLarva);

	BOOST_FOREACH (BWAPI::UpgradeType type, BWAPI::UpgradeTypes::allUpgradeTypes())
	{
		if (BWAPI::Broodwar->self()->getUpgradeLevel(type) > 0)
		{
			//BWAPI::Broodwar->printf("I have %s", type.getName().c_str());
			s.addNumUnits(DATA.getAction(type), (UnitCountType)BWAPI::Broodwar->self()->getUpgradeLevel(type));
		}
	}

	BOOST_FOREACH (BWAPI::TechType type, BWAPI::TechTypes::allTechTypes())
	{
		if (BWAPI::Broodwar->self()->hasResearched(type))
		{
		//	BWAPI::Broodwar->printf("I have %s", type.getName().c_str());
		//	s.addNumUnits(DATA.getAction(type), 1);
		}
	}

	return s;
}

BuildOrderSearch::StarcraftSearchGoal StarcraftBuildOrderSearchManager::getGoal(const std::vector< std::pair<MetaType, UnitCountType> > & goalUnits)
{
	BuildOrderSearch::StarcraftSearchGoal goal;

	for (size_t i=0; i<goalUnits.size(); ++i)
	{
		goal.setGoal(getAction(goalUnits[i].first), goalUnits[i].second);
	}

	return goal;
}

// converts SearchResults.buildOrder vector into vector of MetaType
std::vector<MetaType> StarcraftBuildOrderSearchManager::getMetaVector(const BuildOrderSearch::SearchResults & results)
{
	std::vector<MetaType> metaVector;

	const std::vector<BuildOrderSearch::Action> & buildOrder = results.buildOrder;
	
	//Logger::Instance().log("Get Meta Vector:\n");

	// for each item in the results build order, add it
	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		// retrieve the action from the build order
		BuildOrderSearch::Action a = buildOrder[buildOrder.size()-1-i];

		metaVector.push_back(getMetaType(a));
	}

	return metaVector;
}

// converts from MetaType to StarcraftSearch Action
BuildOrderSearch::Action StarcraftBuildOrderSearchManager::getAction(MetaType t)
{
	//Logger::Instance().log("Action StarcraftBuildOrderSearchManager::getAction(" + t.getName() + ")\n");

	return t.isUnit() ? DATA.getAction(t.unitType) : (t.isTech() ? DATA.getAction(t.techType) : DATA.getAction(t.upgradeType));
}

std::vector<MetaType> StarcraftBuildOrderSearchManager::getOpeningBuildOrder()
{	
	return getMetaVector(StrategyManager::Instance().getOpeningBook());
}

MetaType StarcraftBuildOrderSearchManager::getMetaType(BuildOrderSearch::Action a)
{
	const BuildOrderSearch::StarcraftAction & s = DATA[a];

	MetaType meta;

	// set the appropriate type
	if (s.getType() == BuildOrderSearch::StarcraftAction::UnitType)
	{
		meta = MetaType(s.getUnitType());
	}
	else if (s.getType() == BuildOrderSearch::StarcraftAction::UpgradeType)
	{
		meta = MetaType(s.getUpgradeType());
	} 
	else if (s.getType() == BuildOrderSearch::StarcraftAction::TechType)
	{
		meta = MetaType(s.getTechType());
	}
	else
	{
		assert(false);
	}
	
	return meta;
}

std::vector<MetaType> StarcraftBuildOrderSearchManager::getMetaVector(std::string buildString)
{
	std::stringstream ss;
	ss << buildString;
	std::vector<MetaType> meta;

	int action(0);
	while (ss >> action)
	{
		meta.push_back(getMetaType((BuildOrderSearch::Action)action));
	}

	return meta;
}

void StarcraftBuildOrderSearchManager::drawSearchInformation(int x, int y)
{
	starcraftSearchData.drawSearchResults(10, 240);
}

	
		