#include "Common.h"
#include "DetectorManager.h"

DetectorManager::DetectorManager() : unitClosestToEnemy(NULL) { }

void DetectorManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & detectorUnits = getUnits();

	if (detectorUnits.empty())
	{
		return;
	}

	for (size_t i(0); i<targets.size(); ++i)
	{
		// do something here if there's targets
	}

	cloakedUnitMap.clear();
	UnitVector cloakedUnits;
	bool enemyHasCloak = false;
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

	bool detectorUnitInBattle = false;

	// for each detectorUnit
	BOOST_FOREACH(BWAPI::Unit * detectorUnit, detectorUnits)
	{
		// if we need to regroup, move the detectorUnit to that location
		if (!detectorUnitInBattle && unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid() && enemyHasCloak)
		{
			smartMove(detectorUnit, unitClosestToEnemy->getPosition());
			detectorUnitInBattle = true;
		}
		// otherwise there is no battle or no closest to enemy so we don't want our detectorUnit to die
		// send him to scout around the map
		else
		{
			// get the enemy base location, if we have one
			BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

			// determine the region that the enemy is in
			BWTA::Region * enemyRegion = enemyBaseLocation ? enemyBaseLocation->getRegion() : NULL;
			// determine the region the observer is in
			BWAPI::TilePosition scoutTile(detectorUnit->getPosition());
			BWTA::Region * observerRegion = scoutTile.isValid() ? BWTA::getRegion(scoutTile) : NULL;
			//if we know where the enemy is
			if(enemyRegion)
			{
				if(enemyRegion == observerRegion)
				{
					//if we are in the enemyregion go to the place in the region we visited last
					//should probably check timeframe here, no need to go around and around
					BWAPI::Position explorePosition = MapGrid::Instance().getLeastExploredIn(enemyRegion->getPolygon());
					smartMove(detectorUnit, explorePosition);
				}
				else
				{
					//else go there
					smartMove(detectorUnit, enemyBaseLocation->getPosition());
				}
			}
			else
			{
				BWAPI::Position explorePosition = MapGrid::Instance().getLeastExplored();
				smartMove(detectorUnit, explorePosition);
			}
		}
	}
}


BWAPI::Unit * DetectorManager::closestCloakedUnit(const UnitVector & cloakedUnits, BWAPI::Unit * detectorUnit)
{
	BWAPI::Unit * closestCloaked = NULL;
	double closestDist = 100000;

	BOOST_FOREACH (BWAPI::Unit * unit, cloakedUnits)
	{
		// if we haven't already assigned an detectorUnit to this cloaked unit
		if (!cloakedUnitMap[unit])
		{
			double dist = unit->getDistance(detectorUnit);

			if (!closestCloaked || (dist < closestDist))
			{
				closestCloaked = unit;
				closestDist = dist;
			}
		}
	}

	return closestCloaked;
}