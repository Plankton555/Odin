#include "Common.h"
#include "BuildLearner.h"

// constructor
BuildLearner::BuildLearner() 
{
}

void BuildLearner::update()
{
	
}

int BuildLearner::getUnitTypeCount(BWAPI::UnitType type)
{
	int count(0);

	BOOST_FOREACH (const MetaType & type, actionsTaken)
	{
		if (type.unitType == BWAPI::UnitTypes::Protoss_Zealot)
		{
			count++;
		}
	}

	return count;
}

void BuildLearner::onGameEnd()
{
	std::stringstream ss;

	BOOST_FOREACH (const MetaType & type, actionsTaken)
	{
		ss << "u" << type.unitType.getID() << " ";
	}

	ss << getUnitTypeCount(BWAPI::UnitTypes::Protoss_Zealot) << "\n";

	Logger::Instance().log(ss.str());
}

void BuildLearner::addAction(const MetaType & type)
{
	actionsTaken.push_back(type);
}

bool BuildLearner::isLegalAction(const BWAPI::UnitType type)
{
	if (BWAPI::Broodwar->self()->allUnitCount(type.whatBuilds().first) == 0)
	{
		return false;
	}

	if (BWAPI::Broodwar->self()->supplyUsed() + type.supplyRequired() > BWAPI::Broodwar->self()->supplyTotal())
	{
		return false;
	}

	std::map<BWAPI::UnitType, int>::const_iterator i;
	for(i = type.requiredUnits().begin(); i != type.requiredUnits().end(); i++)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(i->first) == 0)
		{
			return false;
		}
	}

	if (type.isRefinery() && BWAPI::Broodwar->self()->allUnitCount(BWAPI::Broodwar->self()->getRace().getWorker()) < 6)
	{
		return false;
	}

	if (type.requiresPsi() && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Pylon) == 0)
	{
		return false;
	}

	if (type.gasPrice() > 0 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::Broodwar->self()->getRace().getRefinery()) < 1)
	{
		return false;
	}

	if (!type.canProduce() && BWAPI::Broodwar->self()->allUnitCount(type) >= 1)
	{
		return false;
	}

	if (type.isResourceDepot())
	{
		return false;
	}

	if (type.isRefinery() && BWAPI::Broodwar->self()->allUnitCount(BWAPI::Broodwar->self()->getRace().getRefinery()) >= 1)
	{
		return false;
	}

	return true;
}

MetaType BuildLearner::getRandomLegalAction()
{
	std::vector<MetaType> legalActions;

	BOOST_FOREACH (BWAPI::UnitType type, BWAPI::UnitTypes::allUnitTypes())
	{
		if (type.getRace() == BWAPI::Broodwar->self()->getRace())
		{
			if (isLegalAction(type))
			{
				legalActions.push_back(MetaType(type));
			}
		}
	}

	return legalActions[rand() % legalActions.size()];
}

// parses current BWAPI game state and turns it into a StarcraftState of given type
BuildOrderSearch::StarcraftState BuildLearner::getCurrentState()
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

// converts from MetaType to StarcraftSearch Action
BuildOrderSearch::Action BuildLearner::getAction(MetaType t)
{
	//Logger::Instance().log("Action BuildLearner::getAction(" + t.getName() + ")\n");

	return t.isUnit() ? DATA.getAction(t.unitType) : (t.isTech() ? DATA.getAction(t.techType) : DATA.getAction(t.upgradeType));
}

MetaType BuildLearner::getMetaType(BuildOrderSearch::Action a)
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
