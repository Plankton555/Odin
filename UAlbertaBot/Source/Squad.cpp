#include "Common.h"
#include "Squad.h"

int  Squad::lastRetreatSwitch = 0;
bool Squad::lastRetreatSwitchVal = false;

Squad::Squad(const UnitVector & units, SquadOrder order) 
	: units(units)
	, order(order)
{
}

void Squad::update()
{
	// update all necessary unit information within this squad
	updateUnits();

	// determine whether or not we should regroup
	const bool needToRegroup(needsToRegroup());
	
	// draw some debug info
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG && order.type == SquadOrder::Attack) 
	{
		BWAPI::Broodwar->drawTextScreen(200, 330, "%s", regroupStatus.c_str());

		BWAPI::Unit * closest = unitClosestToEnemy();
		if (closest && (BWAPI::Broodwar->getFrameCount() % 24 == 0))
		{
			//BWAPI::Broodwar->setScreenPosition(closest->getPosition().x() - 320, closest->getPosition().y() - 200);
		}
	}

	// if we do need to regroup, do it
	if (needToRegroup)
	{
		InformationManager::Instance().lastFrameRegroup = 1;

		const BWAPI::Position regroupPosition(calcRegroupPosition());
		BWAPI::Broodwar->drawTextScreen(200, 150, "REGROUP");

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x(), regroupPosition.y(), 30, BWAPI::Colors::Purple, true);

		meleeManager.regroup(regroupPosition);
		rangedManager.regroup(regroupPosition);
	}
	else // otherwise, execute micro
	{
		InformationManager::Instance().lastFrameRegroup = 1;

		meleeManager.execute(order);
		rangedManager.execute(order);
		transportManager.execute(order);

		detectorManager.setUnitClosestToEnemy(unitClosestToEnemy());
		detectorManager.execute(order);
	}
}

void Squad::updateUnits()
{
	setAllUnits();
	setNearEnemyUnits();
	setManagerUnits();
}

void Squad::setAllUnits()
{
	// clean up the units vector just in case one of them died
	UnitVector goodUnits;
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		if( unit->isCompleted() && 
			unit->getHitPoints() > 0 && 
			unit->exists() &&
			unit->getPosition().isValid() &&
			unit->getType() != BWAPI::UnitTypes::Unknown)
		{
			goodUnits.push_back(unit);
		}
	}
	units = goodUnits;
}

void Squad::setNearEnemyUnits()
{
	nearEnemy.clear();
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		int x = unit->getPosition().x();
		int y = unit->getPosition().y();

		int left = unit->getType().dimensionLeft();
		int right = unit->getType().dimensionRight();
		int top = unit->getType().dimensionUp();
		int bottom = unit->getType().dimensionDown();

		nearEnemy[unit] = unitNearEnemy(unit);
		if (nearEnemy[unit])
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x-left, y - top, x + right, y + bottom, Options::Debug::COLOR_UNIT_NEAR_ENEMY);
		}
		else
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x-left, y - top, x + right, y + bottom, Options::Debug::COLOR_UNIT_NOTNEAR_ENEMY);
		}
	}
}

void Squad::setManagerUnits()
{
	UnitVector meleeUnits;
	UnitVector rangedUnits;
	UnitVector detectorUnits;
	UnitVector transportUnits;

	// add units to micro managers
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		if(unit->isCompleted() && unit->getHitPoints() > 0 && unit->exists())
		{
			// select dector units
			if (unit->getType().isDetector() && !unit->getType().isBuilding())
			{
				detectorUnits.push_back(unit);
			}
			// select transport units
			else if (unit->getType() == BWAPI::UnitTypes::Protoss_Shuttle || unit->getType() == BWAPI::UnitTypes::Terran_Dropship)
			{
				transportUnits.push_back(unit);
			}
			// select ranged units
			else if ((unit->getType().groundWeapon().maxRange() > 32) || (unit->getType() == BWAPI::UnitTypes::Protoss_Reaver))
			{
				rangedUnits.push_back(unit);
			}
			// select melee units
			else if (unit->getType().groundWeapon().maxRange() <= 32)
			{
				meleeUnits.push_back(unit);
			}
		}
	}

	meleeManager.setUnits(meleeUnits);
	rangedManager.setUnits(rangedUnits);
	detectorManager.setUnits(detectorUnits);
	transportManager.setUnits(detectorUnits);
}

// calculates whether or not to regroup
bool Squad::needsToRegroup()
{
	// if we are not attacking, never regroup
	if (units.empty() || (order.type != SquadOrder::Attack))
	{
		regroupStatus = std::string("\x04 No combat units available");
		return false;
	}

	// if we are DT rushing and we haven't lost a DT yet, no retreat!
	if (StrategyManager::Instance().getCurrentStrategy() == StrategyManager::ProtossDarkTemplar &&
		(BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
	{
		regroupStatus = std::string("\x04 DARK TEMPLAR HOOOOO!");
		return false;
	}

	BWAPI::Unit * unitClosest = unitClosestToEnemy();

	if (!unitClosest)
	{
		regroupStatus = std::string("\x04 No closest unit");
		return false;
	}

	CombatSimulation sim;
	sim.setCombatUnits(unitClosest->getPosition(), Options::Micro::COMBAT_REGROUP_RADIUS + InformationManager::Instance().lastFrameRegroup*300);
	ScoreType score = sim.simulateCombat();

    bool retreat = score < 0;
    int switchTime = 100;
    bool waiting = false;

    // we should not attack unless 5 seconds have passed since a retreat
    if (retreat != lastRetreatSwitchVal)
    {
        if (retreat == false && (BWAPI::Broodwar->getFrameCount() - lastRetreatSwitch < switchTime))
        {
            waiting = true;
            retreat = lastRetreatSwitchVal;
        }
        else
        {
            waiting = false;
            lastRetreatSwitch = BWAPI::Broodwar->getFrameCount();
            lastRetreatSwitchVal = retreat;
        }
    }
	
	if (retreat)
	{
		regroupStatus = std::string("\x04 Retreat - simulation predicts defeat");
	}
	else
	{
		regroupStatus = std::string("\x04 Attack - simulation predicts success");
	}

	return retreat;
}

void Squad::setSquadOrder(const SquadOrder & so)
{
	order = so;
}

bool Squad::unitNearEnemy(BWAPI::Unit * unit)
{
	assert(unit);

	UnitVector enemyNear;

	MapGrid::Instance().GetUnits(enemyNear, unit->getPosition(), 400, false, true);

	return enemyNear.size() > 0;
}

BWAPI::Position Squad::calcCenter()
{
	BWAPI::Position accum(0,0);
	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		accum += unit->getPosition();
	}
	return BWAPI::Position(accum.x() / units.size(), accum.y() / units.size());
}

BWAPI::Position Squad::calcRegroupPosition()
{
	BWAPI::Position regroup(0,0);

	int minDist(100000);

	BOOST_FOREACH(BWAPI::Unit * unit, units)
	{
		if (!nearEnemy[unit])
		{
			int dist = unit->getDistance(order.position);
			if (dist < minDist)
			{
				minDist = dist;
				regroup = unit->getPosition();
			}
		}
	}

	if (regroup == BWAPI::Position(0,0))
	{
		return BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition())->getCenter();
	}
	else
	{
		return regroup;
	}
}

BWAPI::Unit * Squad::unitClosestToEnemy()
{
	BWAPI::Unit * closest = NULL;
	int closestDist = 100000;

	BOOST_FOREACH (BWAPI::Unit * unit, units)
	{
		if (unit->getType() == BWAPI::UnitTypes::Protoss_Observer)
		{
			continue;
		}

		// the distance to the order position
		int dist = MapTools::Instance().getGroundDistance(unit->getPosition(), order.position);

		if (dist != -1 && (!closest || dist < closestDist))
		{
			closest = unit;
			closestDist = dist;
		}
	}

	if (!closest)
	{
		BOOST_FOREACH (BWAPI::Unit * unit, units)
		{
			if (unit->getType() == BWAPI::UnitTypes::Protoss_Observer)
			{
				continue;
			}

			// the distance to the order position
			int dist = unit->getDistance(BWAPI::Position(BWAPI::Broodwar->enemy()->getStartLocation()));

			if (dist != -1 && (!closest || dist < closestDist))
			{
				closest = unit;
				closestDist = dist;
			}
		}
	}

	return closest;
}

int Squad::squadUnitsNear(BWAPI::Position p)
{
	int numUnits = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, units)
	{
		if (unit->getDistance(p) < 600)
		{
			numUnits++;
		}
	}

	return numUnits;
}

bool Squad::squadObserverNear(BWAPI::Position p)
{
	BOOST_FOREACH (BWAPI::Unit * unit, units)
	{
		if (unit->getType().isDetector() && unit->getDistance(p) < 300)
		{
			return true;
		}
	}

	return false;
}

const UnitVector &Squad::getUnits() const	
{ 
	return units; 
} 

const SquadOrder & Squad::getSquadOrder()	const			
{ 
	return order; 
}