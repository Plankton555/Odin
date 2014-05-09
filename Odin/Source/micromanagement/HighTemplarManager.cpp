#include "Common.h"
#include "HighTemplarManager.h"

HighTemplarManager::HighTemplarManager() : unitClosestToEnemy(NULL) { }

void HighTemplarManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & templarUnits = getUnits();
	BWAPI::TechType psiStorm = BWAPI::TechTypes::Psionic_Storm;

	if (templarUnits.empty())
	{
		return;
	}
	if (!BWAPI::Broodwar->self()->hasResearched(psiStorm))
	{//If we don't have storm don't do anything
		return;
	}
	UnitVector hTUnitTargets;
	for (size_t i(0); i<targets.size(); i++) 
	{
		// conditions for targeting
		if (
			!(targets[i]->getType().isBuilding()) &&
			!(targets[i]->getType() == BWAPI::UnitTypes::Zerg_Larva) && 
			!(targets[i]->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			targets[i]->isVisible() ) 
		{
			hTUnitTargets.push_back(targets[i]);
		}
	}

	// for each templar
	BOOST_FOREACH(BWAPI::Unit * templar, templarUnits)
	{
		if (order.type == order.Attack || order.type == order.Defend) 
		{				

			
			if(!hTUnitTargets.empty())
			{			
					
				int i = 0;
						
				BWAPI::Unit* target = hTUnitTargets[0];
						
				if (target != NULL)
				{	

					int enemyDist = templar->getDistance(target);
					int enemyRange = target->getType().airWeapon().maxRange();
					bool noShields = templar->isUnderAttack() && templar->getShields() < 25;
					bool enoughEnergy = templar->getEnergy() >= 75;

					if( (noShields && enemyDist < enemyRange ) || !enoughEnergy)
					{
						BWAPI::Position templarPos = (unitClosestToEnemy->getPosition() - target->getPosition() + unitClosestToEnemy->getPosition());
						smartMove(templar, templarPos);
					}else
					{						
						smartStormUnit(templar, target);
					}

				}		

			}
			else
			{
				// if we're not near the order position
				if (templar->getDistance(order.position) > 300)
				{
					// move to it
					smartMove(templar, unitClosestToEnemy->getPosition());
				}

			}
	
		}
	}
}

void HighTemplarManager::smartStormUnit(BWAPI::Unit * attacker, BWAPI::Unit * target) const
{	
	
	BWAPI::TechType psiStorm = BWAPI::TechTypes::Psionic_Storm;

	assert(attacker && target);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target)
	{
	

		return;
	}

	// if nothing prevents it, attack the target
	attacker->useTech(psiStorm, target->getPosition());

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(	attacker->getPosition().x(), attacker->getPosition().y(),
									target->getPosition().x(), target->getPosition().y(),
									BWAPI::Colors::Red );

}