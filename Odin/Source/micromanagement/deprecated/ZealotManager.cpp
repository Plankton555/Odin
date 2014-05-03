#include "Common.h"
#include "ZealotManager.h"

ZealotManager::ZealotManager() { }

void ZealotManager::executeMicro(const UnitVector & targets, BWAPI::Position regroup) 
{
	const UnitVector & zealots = getUnits();

	// figure out targets
	UnitVector zealotTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (!(targets[i]->getType().isFlyer()) && 
			!(targets[i]->isLifted()) &&
			!(targets[i]->getType() == BWAPI::UnitTypes::Zerg_Larva) && 
			!(targets[i]->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			targets[i]->isVisible()) 
		{
			zealotTargets.push_back(targets[i]);
		}
	}

	// for each zealot
	BOOST_FOREACH(BWAPI::Unit * zealot, zealots)
	{
		if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(zealot->getPosition().x(), zealot->getPosition().y(), 
			zealot->getTargetPosition().x(), zealot->getTargetPosition().y(), BWAPI::Colors::White);

		// regroup if we have to regroup and we're not near an enemy
		if (order.type == order.Attack && regroup != BWAPI::Position(0,0))// && unitNearEnemy(zealot))
		{
			if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(zealot->getPosition().x(), zealot->getPosition().y(), 3, BWAPI::Colors::Orange, true);

			// if the zealot is outside the regroup area
			if (zealot->getDistance(regroup) > 100)
			{
				// regroup it
				smartMove(zealot, regroup);
			}
			else
			{
				smartAttackMove(zealot, zealot->getPosition());
			}
		}
		// otherwise this unit is not regrouping
		else
		{
			// if the order is to attack or defend
			if (order.type == order.Attack || order.type == order.Defend) {

				// if there are targets
				if (!zealotTargets.empty())
				{
					// find the best target for this zealot
					BWAPI::Unit * target = getTarget(zealot, zealotTargets);
					// attack it
					smartAttackUnit(zealot, target);
				}
				// if there are no targets
				else
				{
					// if we're not near the order position
					if (zealot->getDistance(order.position) > 100)
					{
						// move to it
						smartAttackMove(zealot, order.position);
					}
				}
			}
		}
	}
}

// get a target for the zealot to attack
BWAPI::Unit * ZealotManager::getTarget(BWAPI::Unit * zealot, UnitVector & targets)
{
	int highPriority(0);
	int closestDist(100000);
	BWAPI::Unit * closestTarget = NULL;

	// for each target possiblity
	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(unit);

		if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y(), "%d", priority);

		int distance = zealot->getDistance(unit);

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
int ZealotManager::getAttackPriority(BWAPI::Unit * unit) 
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

BWAPI::Unit * ZealotManager::closestZealot(BWAPI::Unit * target, std::set<BWAPI::Unit *> & zealotsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * zealot, zealotsToAssign)
	{
		double distance = zealot->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = zealot;
		}
	}
	
	return closest;
}