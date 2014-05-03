#include "Common.h"
#include "DarkTemplarManager.h"

DarkTemplarManager::DarkTemplarManager() { }

void DarkTemplarManager::executeMicro(const UnitVector & targets, BWAPI::Position regroup) 
{
	const UnitVector & darkTemplars = getUnits();

	// figure out targets
	UnitVector darkTemplarTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (!targets[i]->getType().isFlyer() && !targets[i]->isLifted()) 
		{
			darkTemplarTargets.push_back(targets[i]);
		}
	}

	// dark templars don't regroup
	regroup = BWAPI::Position(0,0);

	// for each darkTemplar
	BOOST_FOREACH(BWAPI::Unit * darkTemplar, darkTemplars)
	{
		if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(darkTemplar->getPosition().x(), darkTemplar->getPosition().y(), 
			darkTemplar->getTargetPosition().x(), darkTemplar->getTargetPosition().y(), BWAPI::Colors::White);

		// regroup if we have to regroup and we're not near an enemy
		if (order.type == order.Attack && regroup != BWAPI::Position(0,0) && !unitNearEnemy(darkTemplar))
		{
			if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(darkTemplar->getPosition().x(), darkTemplar->getPosition().y(), 3, BWAPI::Colors::Orange, true);

			// if the darkTemplar is outside the regroup area
			if (darkTemplar->getDistance(regroup) > 100)
			{
				// regroup it
				smartAttackMove(darkTemplar, regroup);
			}
			else
			{
				darkTemplar->stop();
			}
		}
		// otherwise this unit is not regrouping
		else
		{
			// if the order is to attack or defend
			if (order.type == order.Attack || order.type == order.Defend) {

				// if there are targets
				if (!darkTemplarTargets.empty())
				{
					// find the best target for this darkTemplar
					BWAPI::Unit * target = getTarget(darkTemplar, darkTemplarTargets);
					// attack it
					smartAttackUnit(darkTemplar, target);
				}
				// if there are no targets
				else
				{
					// if we're not near the order position
					if (darkTemplar->getDistance(order.position) > 100)
					{
						// move to it
						smartAttackMove(darkTemplar, order.position);
					}
				}
			}
		}
	}
}

// get a target for the darkTemplar to attack
BWAPI::Unit * DarkTemplarManager::getTarget(BWAPI::Unit * darkTemplar, UnitVector & targets)
{
	int highPriority(0);
	int closestDist(100000);
	BWAPI::Unit * closestTarget = NULL;

	// for each target possiblity
	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(unit);
		int distance = darkTemplar->getDistance(unit);

		// if it's a higher priority, or it's closer, set it
		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = unit;
		}
	}

	return closestTarget;
}

	// get the attack priority of a type in relation to a zergling
int DarkTemplarManager::getAttackPriority(BWAPI::Unit * unit) 
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if (type == BWAPI::UnitTypes::Terran_Medic || 
		type.groundWeapon() != BWAPI::WeaponTypes::None || 
		type ==  BWAPI::UnitTypes::Terran_Bunker) 
	{
		return 3;
	} 
	// next priority is worker
	else if (type.isWorker()) 
	{
		return 2;
	} 
	// then everything else
	else 
	{
		return 1;
	}
}

BWAPI::Unit * DarkTemplarManager::closestDarkTemplar(BWAPI::Unit * target, std::set<BWAPI::Unit *> & darkTemplarsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * darkTemplar, darkTemplarsToAssign)
	{
		double distance = darkTemplar->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = darkTemplar;
		}
	}
	
	return closest;
}
/*
BWAPI::Unit * DarkTemplarManager::chooseTarget(BWAPI::Unit * darkTemplar, const UnitVector & targets, std::map<BWAPI::Unit *, int> & numTargeting) {

	BWAPI::Unit * target(0);
	double targetDistance = 100000.0;
	int targetPriority;

	//loop through the targets and target the closest, highest priority unit
	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		// Additions by Sterling: Simple targeting priority system
		const BWAPI::UnitType type(unit->getType());
		int priority(0);

		if (numTargeting[unit] >= 2)
		{
			continue;
		}
		
		if (type.isWorker()) {
			priority = 8;
		} else if (type == BWAPI::UnitTypes::Terran_Medic || type.groundWeapon() != BWAPI::WeaponTypes::None || type ==  BWAPI::UnitTypes::Terran_Bunker) {
			priority = 9;
		} else if (type ==  BWAPI::UnitTypes::Protoss_High_Templar) {
			priority = 7;
		} else if (type ==  BWAPI::UnitTypes::Protoss_Photon_Cannon || type == BWAPI::UnitTypes::Zerg_Sunken_Colony) {
			priority = 3;
		} else if (type.groundWeapon() != BWAPI::WeaponTypes::None) {
			priority = 2;
		} else if (type.supplyProvided() > 0) {
			priority = 1;
		} 

		if(unit->getTransport() || !unit->isDetected())
		{
			// we can't attack units in bunkers
			continue;
		}

		const double distance(unit->getDistance(darkTemplar));
		if(!target || priority > targetPriority || (priority == targetPriority && distance < targetDistance))
		{
			target			= unit;
			targetPriority	= priority;
			targetDistance	= distance;
		}
	}

	return target;
}*/