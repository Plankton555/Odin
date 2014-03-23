#include "Common.h"
#include "DetectorManager.h"

DetectorManager::DetectorManager() : unitClosestToEnemy(NULL), detectorsInMain(false), detectorsInNat(false) { }

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

	//These bools will help us decide when there is more than one observer
	bool detectorUnitInBattle = false;
	bool scoutingMain = false;
	bool scoutingNat = false;

	// for each detectorUnit
	BOOST_FOREACH(BWAPI::Unit * detectorUnit, detectorUnits)
	{
		// if we need to regroup, move the detectorUnit to that location
		if (!detectorUnitInBattle && unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid() && InformationManager::Instance().enemyHasCloakedUnits())
		{
			smartMove(detectorUnit, unitClosestToEnemy->getPosition());
			detectorUnitInBattle = true;
		}
		// otherwise there is no battle or no closest to enemy so we don't want our detectorUnit to die
		// send him to scout around the map
		else
		{
			//start off by scouting main base
			if( !baseRecentlyScouted(getMainBase()) && !scoutingMain )
			{
				scoutBase(detectorUnit, getMainBase());
				scoutingMain = true;
			}
			//else scout the other bases
			else
			{
				//if( !baseRecentlyScouted(getNatBase()) && !scoutingNat )
				//{
				//	scoutBase(detectorUnit, getNatBase());
				//	scoutingNat = true;
				//}
				//else
				//{
					scoutOtherBases(detectorUnit);
				//}
			}
			
			

			
		}
	}
}

//Returns if the given base has a cell which havent been seen in 2 minutes.
bool DetectorManager::baseRecentlyScouted(BWTA::Region * base)
{
	//gets the longest time since a place in the natural expansion was scouted
	BWAPI::Position leastExplored = MapGrid::Instance().getLeastExploredIn(base->getPolygon());
	int seen = MapGrid::Instance().getCell(leastExplored).timeLastVisited;

	//compare to 2 min ingame (2500 frames)
	int currTime = BWAPI::Broodwar->getFrameCount();
	int recentAmountOfFrames = 2500;

	return (currTime - seen) < recentAmountOfFrames;
}


bool DetectorManager::scoutBase(BWAPI::Unit * obs, BWTA::Region * base)
{
	// determine the region the observer is in
	BWAPI::TilePosition scoutTile(obs->getPosition());
	BWTA::Region * observerRegion = scoutTile.isValid() ? BWTA::getRegion(scoutTile) : NULL;
	//if we know where the base is
	if( base && !detectorInBase(base) )
	{
		if(base == observerRegion)
		{
			//if we are in the enemyregion go to the place in the region we visited last
			BWAPI::Position explorePosition = MapGrid::Instance().getLeastExploredIn(base->getPolygon());
			//if we are under attack try to go around it
			if(obs->isUnderAttack())
			{
				setDetectorsInBase(base);
				return false;
			}
			else
			{
				smartMove(obs, explorePosition);
				return true;
			}
		}
		else
		{
			//go to base if not there
			smartMove(obs, base->getCenter());
			return true;
		}
	}
	else
	{
		return false;
	}
}


void DetectorManager::scoutOtherBases(BWAPI::Unit * obs)
{
	//scout otherbases, which right now is whereever..
	BWAPI::Position explorePosition = MapGrid::Instance().getLeastExplored();
	smartMove(obs, explorePosition);
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

bool DetectorManager::detectorInBase(BWTA::Region * base)
{
	if( base == getMainBase() )
	{
		return detectorsInMain;
	}
	else if (base == getNatBase() )
	{
		return detectorsInNat;
	}
	else
	{
		return false;
	}
}

void DetectorManager::setDetectorsInBase(BWTA::Region * base)
{
	if( base == getMainBase() )
	{
		detectorsInMain = true;
	}
	else if (base == getNatBase() )
	{
		detectorsInNat = true;
	}
	else
	{
		return;
	}
}

//Returns the region of the natural expansion
BWTA::Region * DetectorManager::getNatBase()
{
	if( !natBase ) 
	{
		//was going to put this code into mapgrid but visual studio didnt let me........................................
		BWAPI::Position natPos = MapGrid::Instance().getNaturalExpansion();
		BWTA::Region * natRegion = BWTA::getRegion(natPos);
		natBase = natRegion;
	}
		return natBase;
}

//Returns the region of the mainbase
BWTA::Region * DetectorManager::getMainBase()
{
	// get the enemy base location, if we have one
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	BWTA::Region * mainRegion = enemyBaseLocation ? enemyBaseLocation->getRegion() : NULL;
	return mainRegion;
}