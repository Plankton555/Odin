#include "Common.h"
#include "ObserverManager.h"

ObserverManager::ObserverManager() : unitClosestToEnemy(NULL) { }

void ObserverManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & observers = getUnits();

	if (observers.empty())
	{
		return;
	}

	for (size_t i(0); i<targets.size(); ++i)
	{
		// do something here if there's targets
	}

	cloakedUnitMap.clear();
	UnitVector cloakedUnits;

	// figure out targets
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// conditions for targeting
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker ||
			unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar ||
			unit->getType() == BWAPI::UnitTypes::Terran_Wraith) 
		{
			cloakedUnits.push_back(unit);
			cloakedUnitMap[unit] = false;
		}
	}

	bool observerInBattle = false;

	// for each observer
	BOOST_FOREACH(BWAPI::Unit * observer, observers)
	{
		// if we need to regroup, move the observer to that location
		if (!observerInBattle && unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid())
		{
			smartMove(observer, unitClosestToEnemy->getPosition());
			observerInBattle = true;
		}
		// otherwise there is no battle or no closest to enemy so we don't want our observer to die
		// send him to scout around the map
		else
		{
			BWAPI::Position explorePosition = MapGrid::Instance().getLeastExplored();
			smartMove(observer, explorePosition);
		}
	}
}


BWAPI::Unit * ObserverManager::closestCloakedUnit(const UnitVector & cloakedUnits, BWAPI::Unit * observer)
{
	BWAPI::Unit * closestCloaked = NULL;
	double closestDist = 100000;

	BOOST_FOREACH (BWAPI::Unit * unit, cloakedUnits)
	{
		// if we haven't already assigned an observer to this cloaked unit
		if (!cloakedUnitMap[unit])
		{
			double dist = unit->getDistance(observer);

			if (!closestCloaked || (dist < closestDist))
			{
				closestCloaked = unit;
				closestDist = dist;
			}
		}
	}

	return closestCloaked;
}