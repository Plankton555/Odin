#include "Common.h"
#include "MeleeManager.h"

MeleeManager::MeleeManager() { }

void MeleeManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & meleeUnits = getUnits();

	// figure out targets
	UnitVector meleeUnitTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (!(targets[i]->getType().isFlyer()) && 
			!(targets[i]->isLifted()) &&
			!(targets[i]->getType() == BWAPI::UnitTypes::Zerg_Larva) && 
			!(targets[i]->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			targets[i]->isVisible()) 
		{
			meleeUnitTargets.push_back(targets[i]);
		}
	}

	// for each meleeUnit
	BOOST_FOREACH(BWAPI::Unit * meleeUnit, meleeUnits)
	{
		// if the order is to attack or defend
		if (order.type == order.Attack || order.type == order.Defend) {

			// if there are targets
			if (!meleeUnitTargets.empty())
			{
				// find the best target for this meleeUnit
				BWAPI::Unit * target = getTarget(meleeUnit, meleeUnitTargets);
				// attack it
				smartAttackUnit(meleeUnit, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (meleeUnit->getDistance(order.position) > 100)
				{
					// move to it
					smartMove(meleeUnit, order.position);
				}
			}
		}

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
		{
			BWAPI::Broodwar->drawLineMap(meleeUnit->getPosition().x(), meleeUnit->getPosition().y(), 
			meleeUnit->getTargetPosition().x(), meleeUnit->getTargetPosition().y(), Options::Debug::COLOR_LINE_TARGET);
		}
	}
}

// get a target for the meleeUnit to attack
BWAPI::Unit * MeleeManager::getTarget(BWAPI::Unit * meleeUnit, UnitVector & targets)
{
	int highPriority(0);
	int closestDist(100000);
	BWAPI::Unit * closestTarget = NULL;

	// for each target possiblity
	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(unit);
		if (meleeUnit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar && unit->getType().isWorker())
		{
			priority = 11;
		}

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y(), "%d", priority);

		int distance = meleeUnit->getDistance(unit);

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
int MeleeManager::getAttackPriority(BWAPI::Unit * unit) 
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if (type == BWAPI::UnitTypes::Terran_Medic || 
		(type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) || 
		type ==  BWAPI::UnitTypes::Terran_Bunker ||
		type == BWAPI::UnitTypes::Protoss_High_Templar ||
		type == BWAPI::UnitTypes::Protoss_Reaver ||
		(type.isWorker() && unitNearChokepoint(unit))) 
	{
		return 10;
	} 
	// next priority is worker
	else if (type.isWorker()) 
	{
		return 9;
	} 
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 4;
	}
	else if (type.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}

BWAPI::Unit * MeleeManager::closestMeleeUnit(BWAPI::Unit * target, std::set<BWAPI::Unit *> & meleeUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * meleeUnit, meleeUnitsToAssign)
	{
		double distance = meleeUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = meleeUnit;
		}
	}
	
	return closest;
}