#include "Common.h"
#include "HighTemplarManager.h"

HighTemplarManager::HighTemplarManager()  { }

void HighTemplarManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & templarUnits = getUnits();

	if (templarUnits.empty())
	{
		return;
	}

	// for each templar
	BOOST_FOREACH(BWAPI::Unit * templar, templarUnits)
	{
		if (order.type == order.Attack || order.type == order.Defend) 
		{			

			BWAPI::TechType psiStorm = BWAPI::TechTypes::Psionic_Storm;
			
			if(!targets.empty())
			{
				if (BWAPI::Broodwar->self()->hasResearched(psiStorm))
				{	
					if (templar->getEnergy() >= 75)
					{
						BWAPI::Unit* target = targets[0];
						if (target != NULL)
						{
							templar->useTech(psiStorm, target->getPosition());
						}
					}
				}				

			}else
			{
				// if we're not near the order position
				if (templar->getDistance(order.position) > 100)
				{
					// move to it
					smartMove(templar, order.position);
				}

			}
	
		}
	}
}