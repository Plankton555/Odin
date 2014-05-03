#include "Common.h"
#include "DragoonManager.h"

DragoonManager::DragoonManager() { }

void DragoonManager::executeMicro(const UnitVector & targets, BWAPI::Position regroup) 
{
	const UnitVector & dragoons = getUnits();

	// figure out targets
	UnitVector dragoonTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (targets[i]->isVisible()) 
		{
			dragoonTargets.push_back(targets[i]);
		}
	}

	// for each zealot
	BOOST_FOREACH(BWAPI::Unit * dragoon, dragoons)
	{
		if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(dragoon->getPosition().x(), dragoon->getPosition().y(), 
			dragoon->getTargetPosition().x(), dragoon->getTargetPosition().y(), BWAPI::Colors::White);

		// regroup if we have to regroup and we're not near an enemy
		if (order.type == order.Attack && regroup != BWAPI::Position(0,0))// && !unitNearEnemy(dragoon))
		{
			if (DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(dragoon->getPosition().x(), dragoon->getPosition().y(), 3, BWAPI::Colors::Orange, true);

			// if the zealot is outside the regroup area
			if (dragoon->getDistance(regroup) > 100)
			{
				// regroup it
				smartMove(dragoon, regroup);
			}
			else
			{
				smartAttackMove(dragoon, dragoon->getPosition());
			}
		}
		// otherwise this unit is not regrouping
		else
		{
			// if the order is to attack or defend
			if (order.type == order.Attack || order.type == order.Defend) {

				// if there are targets
				if (!dragoonTargets.empty())
				{
					// find the best target for this zealot
					BWAPI::Unit * target = getTarget(dragoon, dragoonTargets);
					// attack it
					smartAttackUnit(dragoon, target);
				}
				// if there are no targets
				else
				{
					// if we're not near the order position
					if (dragoon->getDistance(order.position) > 100)
					{
						// move to it
						smartAttackMove(dragoon, order.position);
					}
				}
			}
		}
	}
}

// get a target for the zealot to attack
BWAPI::Unit * DragoonManager::getTarget(BWAPI::Unit * dragoon, UnitVector & targets)
{
	int range(BWAPI::UnitTypes::Protoss_Dragoon.groundWeapon().maxRange());

	int highestInRangePriority(0);
	int highestNotInRangePriority(0);
	int lowestInRangeHitPoints(10000);
	int lowestNotInRangeDistance(10000);

	BWAPI::Unit * inRangeTarget = NULL;
	BWAPI::Unit * notInRangeTarget = NULL;

	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(unit);
		int distance = dragoon->getDistance(unit);

		// if the unit is in range, update the target with the lowest hp
		if (dragoon->getDistance(unit) <= range)
		{
			if (priority > highestInRangePriority ||
				(priority == highestInRangePriority && unit->getHitPoints() < lowestInRangeHitPoints))
			{
				lowestInRangeHitPoints = unit->getHitPoints();
				highestInRangePriority = priority;
				inRangeTarget = unit;
			}
		}
		// otherwise it isn't in range so see if it's closest
		else
		{
			if (priority > highestNotInRangePriority ||
				(priority == highestNotInRangePriority && distance < lowestNotInRangeDistance))
			{
				lowestNotInRangeDistance = distance;
				highestNotInRangePriority = priority;
				notInRangeTarget = unit;
			}
		}
	}

	// if there is a highest priority unit in range, attack it first
	return (highestInRangePriority >= highestNotInRangePriority) ? inRangeTarget : notInRangeTarget;
}

	// get the attack priority of a type in relation to a zergling
int DragoonManager::getAttackPriority(BWAPI::Unit * unit) 
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

BWAPI::Unit * DragoonManager::closestDragoon(BWAPI::Unit * target, std::set<BWAPI::Unit *> & dragoonsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * dragoon, dragoonsToAssign)
	{
		double distance = dragoon->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = dragoon;
		}
	}
	
	return closest;
}