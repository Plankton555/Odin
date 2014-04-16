#include "Common.h"
#include "RangedManager.h"

RangedManager::RangedManager() : unitClosestToEnemy(NULL) { }

void RangedManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & rangedUnits = getUnits();

	// figure out targets
	UnitVector rangedUnitTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (targets[i]->isVisible()) 
		{
			rangedUnitTargets.push_back(targets[i]);
		}
	}

	// for each zealot
	BOOST_FOREACH(BWAPI::Unit * rangedUnit, rangedUnits)
	{
		//try to train subUnits
		trainSubUnits(rangedUnit);
		
		//Carriers should be microed differently since they are flying and we want them in the back of the army
		if( rangedUnit->getType() == BWAPI::UnitTypes::Protoss_Carrier )
		{
			microCarrier(rangedUnit, rangedUnitTargets);
		}
		else
		{
			// if the order is to attack or defend
			if (order.type == order.Attack || order.type == order.Defend) {

				// if there are targets
				if (!rangedUnitTargets.empty())
				{
					// find the best target for this zealot
					BWAPI::Unit * target = getTarget(rangedUnit, rangedUnitTargets);

					// attack it
					kiteTarget(rangedUnit, target);
				}
				// if there are no targets
				else
				{
					// if we're not near the order position
					if (rangedUnit->getDistance(order.position) > 100)
					{
						// move to it
						smartAttackMove(rangedUnit, order.position);
					}
				}
			}

			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) 
			{
				BWAPI::Broodwar->drawLineMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(), 
					rangedUnit->getTargetPosition().x(), rangedUnit->getTargetPosition().y(), Options::Debug::COLOR_LINE_TARGET);
			}
		}
	}
}

void RangedManager::kiteTarget(BWAPI::Unit * rangedUnit, BWAPI::Unit * target)
{
	
	double range(rangedUnit->getType().groundWeapon().maxRange());
	if (rangedUnit->getType() == BWAPI::UnitTypes::Protoss_Dragoon && BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Singularity_Charge))
	{
		range = 6*32;
	}

	// determine whether the target can be kited
	if (range <= target->getType().groundWeapon().maxRange())
	{
		// if we can't kite it, there's no point
		smartAttackUnit(rangedUnit, target);
		return;
	}

	double		minDist(64);
	bool		kite(true);
	double		dist(rangedUnit->getDistance(target));
	double		speed(rangedUnit->getType().topSpeed());

	double	timeToEnter = std::max(0.0,(dist - range) / speed);
	if ((timeToEnter >= rangedUnit->getGroundWeaponCooldown()) && (dist >= minDist))
	{
		kite = false;
	}

	if (target->getType().isBuilding() && target->getType() != BWAPI::UnitTypes::Terran_Bunker)
	{
		kite = false;
	}

	if (rangedUnit->isSelected())
	{
		BWAPI::Broodwar->drawCircleMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(), 
			(int)range, BWAPI::Colors::Cyan);
	}

	// if we can't shoot, run away
	if (kite)
	{
		BWAPI::Position fleePosition(rangedUnit->getPosition() - target->getPosition() + rangedUnit->getPosition());

		BWAPI::Broodwar->drawLineMap(rangedUnit->getPosition().x(), rangedUnit->getPosition().y(), 
			fleePosition.x(), fleePosition.y(), BWAPI::Colors::Cyan);

		smartMove(rangedUnit, fleePosition);
	}
	// otherwise shoot
	else
	{
		smartAttackUnit(rangedUnit, target);
	}
}

// get a target for the zealot to attack
BWAPI::Unit * RangedManager::getTarget(BWAPI::Unit * rangedUnit, UnitVector & targets)
{
	int range(rangedUnit->getType().groundWeapon().maxRange());
	if(rangedUnit->getType() == BWAPI::UnitTypes::Protoss_Carrier)
		range = 192;
	int highestInRangePriority(0);
	int highestNotInRangePriority(0);
	int lowestInRangeHitPoints(10000);
	int lowestNotInRangeDistance(10000);

	BWAPI::Unit * inRangeTarget = NULL;
	BWAPI::Unit * notInRangeTarget = NULL;

	BOOST_FOREACH(BWAPI::Unit * unit, targets)
	{
		int priority = getAttackPriority(rangedUnit, unit);
		int distance = rangedUnit->getDistance(unit);

		// if the unit is in range, update the target with the lowest hp
		if (rangedUnit->getDistance(unit) <= range)
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
int RangedManager::getAttackPriority(BWAPI::Unit * rangedUnit, BWAPI::Unit * target) 
{
	BWAPI::UnitType rangedUnitType = rangedUnit->getType();
	BWAPI::UnitType targetType = target->getType();

	bool canAttackUs = rangedUnitType.isFlyer() ? targetType.airWeapon() != BWAPI::WeaponTypes::None : targetType.groundWeapon() != BWAPI::WeaponTypes::None;

	

	// highest priority is something that can attack us or aid in combat
	if (targetType == BWAPI::UnitTypes::Terran_Medic || canAttackUs ||
		targetType ==  BWAPI::UnitTypes::Terran_Bunker) 
	{
		return 3;
	} 
	// next priority is worker
	else if (targetType.isWorker()) 
	{
		return 2;
	} 
	// then everything else
	else 
	{
		return 1;
	}
}

BWAPI::Unit * RangedManager::closestrangedUnit(BWAPI::Unit * target, std::set<BWAPI::Unit *> & rangedUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit * closest = NULL;

	BOOST_FOREACH (BWAPI::Unit * rangedUnit, rangedUnitsToAssign)
	{
		if(rangedUnit->getType() == BWAPI::UnitTypes::Protoss_Carrier)
		{
			continue;
		}
		double distance = rangedUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = rangedUnit;
		}
	}
	
	return closest;
}

void RangedManager::trainSubUnits(BWAPI::Unit * unit)
{
	int unitID = unit->getType().getID();
	if( unitID == BWAPI::UnitTypes::Protoss_Carrier.getID() )
	{
		int maxInterceptors = 4;
		if( BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Carrier_Capacity) > 0 )
		{
			maxInterceptors = 8;
		}
		int maxQueue = 2; //No reason to have more than 2 in queue
		int inQueue = unit->getTrainingQueue().size();
		int curInterceptors = unit->getInterceptorCount();
		
		if( BWAPI::Broodwar->self()->minerals() >= BWAPI::UnitTypes::Protoss_Interceptor.mineralPrice() && inQueue < maxQueue && curInterceptors + inQueue < maxInterceptors)
		{
			unit->train(BWAPI::UnitTypes::Protoss_Interceptor);
		}
	}
	else if( unitID == BWAPI::UnitTypes::Protoss_Reaver.getID()  && unit->exists() )
	{
		int maxScarabs = 5;
		if( BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Reaver_Capacity) > 0 )
		{
			maxScarabs = 10;
		}
		int maxQueue = 2;
		int inQueue = unit->getTrainingQueue().size();
		int curScarabs = unit->getScarabCount();

		//Train the max amount of reaver scarabs or fill the training queue. 
		int nbrToTrain = std::min(maxScarabs - (curScarabs + inQueue), maxQueue-inQueue);
		
		if( nbrToTrain > 0 && BWAPI::Broodwar->self()->minerals() >= BWAPI::UnitTypes::Protoss_Scarab.mineralPrice() )
		{
			unit->train(BWAPI::UnitTypes::Protoss_Scarab);
		}
	}
}

void RangedManager::microCarrier(BWAPI::Unit * carrier, UnitVector targets)
{
	if (order.type == order.Attack || order.type == order.Defend) {
		if(!targets.empty())
		{
			BWAPI::Unit * enemy = getTarget(carrier, targets);
			int enemyDist = carrier->getDistance(enemy);
			int enemyRange = enemy->getType().airWeapon().maxRange();
			bool greaterRange = enemyRange < 256;
			bool noShields = carrier->isUnderAttack() && carrier->getShields() < 25;
			bool noInterceptors = carrier->getInterceptorCount() == 0;

			if( (noShields && enemyDist < enemyRange && greaterRange) || noInterceptors)
			{
				BWAPI::Position carrierPos = (unitClosestToEnemy->getPosition() - enemy->getPosition() + unitClosestToEnemy->getPosition());
				smartMove(carrier, carrierPos);
			}
			else
			{
				smartAttackUnit(carrier, enemy);
			}
		}
		else
		{
			if (carrier->getDistance(order.position) > 500)
			{
				//dont just fly to the opponents main base, follow the army!
				smartAttackMove(carrier, unitClosestToEnemy->getPosition());
			}
		}
	}
}