#include "Common.h"
#include "ScoutManager.h"
#include "InformationManager.h"

ScoutManager::ScoutManager() : workerScout(NULL), numWorkerScouts(0), scoutUnderAttack(false), circlingDone(false), circling(NULL), circlingOpposite(false), mainObserver(NULL), secObserver(NULL), nextExp(NULL)
{
}

void ScoutManager::update(const std::set<BWAPI::Unit *> & scoutUnits)
{
	if (scoutUnits.size() == 1)
	{
		BWAPI::Unit * scoutUnit = *scoutUnits.begin();

		if (scoutUnit->getType().isWorker())
		{
			if (scoutUnit != workerScout)
			{
				numWorkerScouts++;
				workerScout = scoutUnit;
			}
		}
	}
	//Set the main and sec observers if any. if we have them, they are still in the list and still active
	bool hasMainObserver = mainObserver && (scoutUnits.find(mainObserver) != scoutUnits.end()) && mainObserver->getPosition().isValid();
	bool hasSecObserver = secObserver && (scoutUnits.find(secObserver) != scoutUnits.end()) && secObserver->getPosition().isValid();

	//if for some reason our mainobserver is no more then transfer the sec to main. 
	if( !hasMainObserver && hasSecObserver )
	{
		mainObserver = secObserver;
		secObserver = NULL;
		hasMainObserver = true;
		hasSecObserver = false;
	}

	BOOST_FOREACH(BWAPI::Unit * obs, scoutUnits)
	{
		if( obs->getType().getID() == BWAPI::UnitTypes::Protoss_Observer.getID() && obs->getPosition().isValid())
		{
			if( !hasMainObserver )
			{
				mainObserver = obs;
				hasMainObserver = true;
			}
			else if( hasMainObserver && !hasSecObserver && obs != mainObserver )
			{
				secObserver = obs;
				hasSecObserver = true;
			}
		}
	}

	

	moveScouts();
	moveObservers();
}

void ScoutManager::moveObservers()
{
	if( mainObserver && mainObserver->getPosition().isValid() )
	{
		//loop through enemy occupied bases check for not not recently scouted
		//check for detectors
		bool hasTarget = false;
		std::set<BWTA::Region *> regions = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());
		BOOST_FOREACH(BWTA::Region * base, regions)
		{
			if( !hasTarget && !baseRecentlyScouted(base) && !baseDetectors[base] )
			{
				scoutBase(mainObserver, base);
				hasTarget = true;
			}
		}
		if ( !hasTarget )
		{
			BWTA::Region * nextBase = nextExpansion();
			if ( !baseRecentlyScouted(nextBase) )
			{
				scoutBase(mainObserver, nextBase);
				hasTarget = true;
			}
			else
			{
				//If we have recently scouted all their bases and the next expansion we should probably just idle in main for now. 
				BWTA::BaseLocation * mainBase = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
				if( !baseDetectors[mainBase->getRegion()] )
				{
					scoutBase(mainObserver, mainBase->getRegion());
				}
				else
				{
					BWAPI::Position explorePosition = MapGrid::Instance().getLeastExplored();
					smartMove(mainObserver, explorePosition);
				}
			}
		}
	}
	if( secObserver && secObserver->getPosition().isValid() )
	{
		//try to find enemy army and follow it
		std::vector<UnitInfo> units;
		int minDistance = 10000;
		BWAPI::Position pos;
		BWAPI::Position cmpPos;
		BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
		{
			units.clear();
			InformationManager::Instance().getNearbyForce(units, unit->getPosition(), BWAPI::Broodwar->enemy(), 900);
			int dist = secObserver->getPosition().getDistance(unit->getPosition());
			if(units.size() > 4 && dist < minDistance)
			{
				//Dont get stuck at the bunkers.. or buildings in general :( TODO
				bool hasBunker = false;
				BOOST_FOREACH(UnitInfo inf, units)
				{
					if(inf.unitID == BWAPI::UnitTypes::Terran_Bunker.getID())
					{
						hasBunker = true;
					}
				}
				if( !hasBunker )
				{
					minDistance = dist;
					pos = unit->getPosition();
				}
			}
		}
		if( pos != cmpPos)
		{
			smartMove(secObserver, pos);
		}
		else
		{
			BWAPI::Position centerOfMainChokePoint = (*InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getRegion()->getChokepoints().begin())->getCenter();
			smartMove(secObserver, centerOfMainChokePoint);
		}
	}
}

BWTA::Region * ScoutManager::nextExpansion()
{
	std::set<BWTA::Region *> occReg = InformationManager::Instance().getOccupiedRegions(BWAPI::Broodwar->enemy());
	if( !nextExp || (occReg.find(nextExp) != occReg.end()) )
	{
		std::set<BWTA::BaseLocation*> bases = BWTA::getBaseLocations();
		BWAPI::Position enemyMain = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition();
		BWAPI::Position selfMain = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition();
		int minDistance = MapTools::Instance().getGroundDistance(enemyMain, selfMain);
		BWTA::BaseLocation * closestBase;
		
		BOOST_FOREACH(BWTA::BaseLocation * base, bases)
		{
			if( (occReg.find(base->getRegion()) == occReg.end()) && MapTools::Instance().getGroundDistance(enemyMain, base->getPosition()) < minDistance )
			{
				//The distance is saved in a map so not too bad to just call it again
				minDistance = MapTools::Instance().getGroundDistance(enemyMain, base->getPosition());
				closestBase = base;
			}
		}
		nextExp = closestBase->getRegion();
	}
		return nextExp;
}

void ScoutManager::scoutBase(BWAPI::Unit * obs, BWTA::Region * base)
{
	// determine the region the observer is in
	BWAPI::TilePosition scoutTile(obs->getPosition());
	BWTA::Region * observerRegion = scoutTile.isValid() ? BWTA::getRegion(scoutTile) : NULL;
	//if we know where the base is
	if( base )
	{
		if(base == observerRegion)
		{
			//if we are in the enemyregion go to the place in the region we visited last
			BWAPI::Position explorePosition = MapGrid::Instance().getLeastExploredIn(base->getPolygon());
			if(obs->isUnderAttack())
			{
				//if under attack go other way then intended
				baseDetectors[base] = true;
				int dx = obs->getPosition().x() - explorePosition.x();
				int dy = obs->getPosition().y() - explorePosition.y();
				BWAPI::Position newPos = BWAPI::Position(obs->getPosition().x()+dx, obs->getPosition().y()+dy);
				smartMove(obs, newPos);
			}
			else
			{
				smartMove(obs, explorePosition);
			}
		}
		else
		{
			//go to base if not there
			BWAPI::Position explorePosition = MapGrid::Instance().getLeastExploredIn(base->getPolygon());
			smartMove(obs, explorePosition);
		}
	}
}

bool ScoutManager::baseRecentlyScouted(BWTA::Region * base)
{
	//gets the longest time since a place in the natural expansion was scouted
	BWAPI::Position leastExplored = MapGrid::Instance().getLeastExploredIn(base->getPolygon());
	int seen = MapGrid::Instance().getCell(leastExplored).timeLastVisited;

	//compare to 2 min ingame (2500 frames)
	int currTime = BWAPI::Broodwar->getFrameCount();
	int recentAmountOfFrames = 2500;

	return (currTime - seen) < recentAmountOfFrames;
}

bool ScoutManager::detectorsInBase(BWTA::Region * base)
{
	if( baseDetectors.find(base) != baseDetectors.end())
	{
		return baseDetectors[base];
	}
	else
	{
		baseDetectors[base] = false;
		return false;
	}
}

void ScoutManager::moveScouts()
{
	if (!workerScout || !workerScout->exists() || !workerScout->getPosition().isValid())
	{
		return;
	}

	// get the enemy base location, if we have one
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

	// determine the region that the enemy is in
	BWTA::Region * enemyRegion = enemyBaseLocation ? enemyBaseLocation->getRegion() : NULL;

	// determine the region the scout is in
	BWAPI::TilePosition scoutTile(workerScout->getPosition());
	BWTA::Region * scoutRegion = scoutTile.isValid() ? BWTA::getRegion(scoutTile) : NULL;

	// we only care if the scout is under attack within the enemy region
	// this ignores if their scout worker attacks it on the way to their base
	if (workerScout->isUnderAttack() && (scoutRegion == enemyRegion))
	{
		scoutUnderAttack = true;
	}

	if (!workerScout->isUnderAttack() && !enemyWorkerInRadius())
	{
		scoutUnderAttack = false;
	}

	// if we know where the enemy region is and where our scout is
	if (enemyRegion && scoutRegion)
	{
		// if the scout is in the enemy region
		if (scoutRegion == enemyRegion)
		{
			std::vector<GroundThreat> groundThreats;
			fillGroundThreats(groundThreats, workerScout->getPosition());

			// get the closest enemy worker
			BWAPI::Unit * closestWorker = closestEnemyWorker();

			// if the worker scout is not under attack
			if (!scoutUnderAttack)
			{
				
				// if circling is done and there is a worker nearby, harass it
				if (circlingDone && closestWorker && (workerScout->getDistance(closestWorker) < 800))
				{
					smartAttack(workerScout, closestWorker);
					BWAPI::Broodwar->drawTextMap(workerScout->getPosition().x(), workerScout->getPosition().y(), "HARASSING");
				}

				// if the worker is close to the enemyBaseLocation, circle around base
				else if (workerScout->getDistance(enemyBaseLocation->getPosition()) < 300)
				{

					//Circle when possible
					 if (!workerScout->isMoving() || !circling)
					{
						//The the start position of the circling so we can know when a lap is done
						if (!circling) {
							circling = new BWAPI::Position((workerScout->getPosition()));
						}

						int xDiff = workerScout->getPosition().x() - enemyBaseLocation->getPosition().x();
						int yDiff = workerScout->getPosition().y() - enemyBaseLocation->getPosition().y();
						//Rotate
						int newX = yDiff;
						int newY = -xDiff;
						//move
						BWAPI::Position newPosition (enemyBaseLocation->getPosition().x()+newX, enemyBaseLocation->getPosition().y()+newY);
						smartMove(workerScout, newPosition);
						BWAPI::Broodwar->drawTextMap(workerScout->getPosition().x(), workerScout->getPosition().y(), "STARTED CIRCLING");
					}
					//Else just keep circling and check if done
					else
					{
						//Check if the worker has reached the opposite side of the startposition
						BWAPI::Position opposite (2*enemyBaseLocation->getPosition().x()-circling->x(), 2*enemyBaseLocation->getPosition().y()-circling->y());
						if ((workerScout->getPosition().x()-enemyBaseLocation->getPosition().x()) * (opposite.x()-enemyBaseLocation->getPosition().x()) > 0
								&& (workerScout->getPosition().y()-enemyBaseLocation->getPosition().y()) * (opposite.y()-enemyBaseLocation->getPosition().y()) > 0) //Check quadrants, not the specific position
						{
							circlingOpposite = true;
						}

						//Check if the worker has reached backt to the start
						if (circlingOpposite && (workerScout->getPosition().x()-enemyBaseLocation->getPosition().x()) * (circling->x()-enemyBaseLocation->getPosition().x()) > 0
								&& (workerScout->getPosition().y()-enemyBaseLocation->getPosition().y()) * (circling->y()-enemyBaseLocation->getPosition().y()) > 0) //Check quadrants
						{
							circlingDone = true;
						}


						std::string out ("Circling");
						if(circlingOpposite) out.append(" - OPPOSITE");
						if(circlingDone) out.append(" - DONE");

						BWAPI::Broodwar->drawTextMap(workerScout->getPosition().x(), workerScout->getPosition().y(), out.c_str());
						//BWAPI::Broodwar->drawTextMap(workerScout->getPosition().x(), workerScout->getPosition().y(), "CIRCLING");
					}
				}

				// otherwise keep moving to the enemy region
				else
				{
					// move to the enemy region
					smartMove(workerScout, enemyBaseLocation->getPosition());
					BWAPI::Broodwar->drawLineMap(workerScout->getPosition().x(), workerScout->getPosition().y(), 
						enemyBaseLocation->getPosition().x(), enemyBaseLocation->getPosition().y(),
						BWAPI::Colors::Yellow);
					BWAPI::Broodwar->drawTextMap(workerScout->getPosition().x(), workerScout->getPosition().y(), "MOVING TO BASE");
				}
				
			}
			// if the worker scout is under attack
			else
			{
				BWAPI::Position fleeTo = calcFleePosition(groundThreats, NULL);
				if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(fleeTo.x(), fleeTo.y(), 10, BWAPI::Colors::Red);

				BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsInRadius(fleeTo, 10))
				{
					if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 5, BWAPI::Colors::Cyan, true);
				}

				smartMove(workerScout, fleeTo);
			}
		}
		// if the scout is not in the enemy region
		else if (scoutUnderAttack)
		{
			smartMove(workerScout, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
		}
		else
		{
			// move to the enemy region
			smartMove(workerScout, enemyBaseLocation->getPosition());	
		}
		
	}

	// for each start location in the level
	if (!enemyRegion)
	{
		BOOST_FOREACH (BWTA::BaseLocation * startLocation, BWTA::getStartLocations()) 
		{
			// if we haven't explored it yet
			if (!BWAPI::Broodwar->isExplored(startLocation->getTilePosition())) 
			{
				// assign a zergling to go scout it
				smartMove(workerScout, BWAPI::Position(startLocation->getTilePosition()));			
				return;
			}
		}
	}
}

BWAPI::Position ScoutManager::calcFleePosition(const std::vector<GroundThreat> & threats, BWAPI::Unit * target) 
{
	// calculate the standard flee vector
	double2 fleeVector = getFleeVector(threats);

	// vector to the target, if one exists
	double2 targetVector(0,0);

	// normalise the target vector
	if (target) 
	{
		targetVector = target->getPosition() - workerScout->getPosition();
		targetVector.normalise();
	}

	int mult = 1;

	if (workerScout->getID() % 2) 
	{
		mult = -1;
	}

	// rotate the flee vector by 30 degrees, this will allow us to circle around and come back at a target
	//fleeVector.rotate(mult*30);
	double2 newFleeVector;

	double fleeDistance = 24;

	int r = 0;
	int sign = 1;
	int iterations = 0;
		
	// keep rotating the vector until the new position is able to be walked on
	while (r <= 360) 
	{
		// rotate the flee vector
		fleeVector.rotate(mult*r);

		// re-normalize it
		fleeVector.normalise();

		// new vector should semi point back at the target
		newFleeVector = fleeVector * 2 + targetVector;

		// the position we will attempt to go to
		BWAPI::Position test(workerScout->getPosition() + newFleeVector * fleeDistance);

		// draw the debug vector
		//if (drawDebugVectors) 
		//{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(workerScout->getPosition().x(), workerScout->getPosition().y(), test.x(), test.y(), BWAPI::Colors::Cyan);
		//}

		// if the position is able to be walked on, break out of the loop
		if (isValidFleePosition(test))
		{
			break;
		}

		r = r + sign * (r + 10);
		sign = sign * -1;

		if (++iterations > 36)
		{
			break;
		}
	}

	// go to the calculated 'good' position
	BWAPI::Position fleeTo(workerScout->getPosition() + newFleeVector * fleeDistance);
	
	
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(workerScout->getPosition().x(), workerScout->getPosition().y(), fleeTo.x(), fleeTo.y(), BWAPI::Colors::Orange);
	

	return fleeTo;
}

double2 ScoutManager::getFleeVector(const std::vector<GroundThreat> & threats)
{
	double2 fleeVector(0,0);

	BOOST_FOREACH (const GroundThreat & threat, threats)
	{
		// Get direction from enemy to mutalisk
		const double2 direction(workerScout->getPosition() - threat.unit->getPosition());

		// Divide direction by square distance, weighting closer enemies higher
		//  Dividing once normalises the vector
		//  Dividing a second time reduces the effect of far away units
		const double distanceSq(direction.lenSq());
		if(distanceSq > 0)
		{
			// Enemy influence is direction to flee from enemy weighted by danger posed by enemy
			const double2 enemyInfluence( (direction / distanceSq) * threat.weight );

			fleeVector = fleeVector + enemyInfluence;
		}
	}

	if(fleeVector.lenSq() == 0)
	{
		// Flee towards our base
		fleeVector = double2(1,0);	
	}

	fleeVector.normalise();

	BWAPI::Position p1(workerScout->getPosition());
	BWAPI::Position p2(p1 + fleeVector * 100);

	return fleeVector;
}

bool ScoutManager::isValidFleePosition(BWAPI::Position pos) 
{

	// get x and y from the position
	int x(pos.x()), y(pos.y());

	// walkable tiles exist every 8 pixels
	bool good = BWAPI::Broodwar->isWalkable(x/8, y/8);
	
	// if it's not walkable throw it out
	if (!good) return false;
	
	// for each of those units, if it's a building or an attacking enemy unit we don't want to go there
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(x/32, y/32)) 
	{
		if	(unit->getType().isBuilding() || unit->getType().isResourceContainer() || 
			(unit->getPlayer() != BWAPI::Broodwar->self() && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)) 
		{		
				return false;
		}
	}

	int geyserDist = 50;
	int mineralDist = 32;

	BWTA::BaseLocation * enemyLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

	BWAPI::Unit * geyser = getEnemyGeyser();
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(geyser->getPosition().x(), geyser->getPosition().y(), geyserDist, BWAPI::Colors::Red);

	if (geyser->getDistance(pos) < geyserDist)
	{
		return false;
	}

	BOOST_FOREACH (BWAPI::Unit * mineral, enemyLocation->getMinerals())
	{
		if (mineral->getDistance(pos) < mineralDist)
		{
			return false;
		}
	}

	BWTA::Region * enemyRegion = enemyLocation->getRegion();
	if (enemyRegion && BWTA::getRegion(BWAPI::TilePosition(pos)) != enemyRegion)
	{
		return false;
	}

	// otherwise it's okay
	return true;
}

// fills the GroundThreat vector within a radius of a target
void ScoutManager::fillGroundThreats(std::vector<GroundThreat> & threats, BWAPI::Position target)
{
	// radius of caring
	const int radiusWeCareAbout(1000);
	const int radiusSq(radiusWeCareAbout * radiusWeCareAbout);

	// for each of the candidate units
	const std::set<BWAPI::Unit*> & candidates(BWAPI::Broodwar->enemy()->getUnits());
	BOOST_FOREACH (BWAPI::Unit * e, candidates)
	{
		// if they're not within the radius of caring, who cares?
		const BWAPI::Position delta(e->getPosition() - target);
		if(delta.x() * delta.x() + delta.y() * delta.y() > radiusSq)
		{
			continue;
		}

		// default threat
		GroundThreat threat;
		threat.unit		= e;
		threat.weight	= 1;

		// get the air weapon of the unit
		BWAPI::WeaponType groundWeapon(e->getType().groundWeapon());

		// if it's a bunker, weight it as if it were 4 marines
		if(e->getType() == BWAPI::UnitTypes::Terran_Bunker)
		{
			groundWeapon	= BWAPI::WeaponTypes::Gauss_Rifle;
			threat.weight	= 4;
		}

		// weight the threat based on the highest DPS
		if(groundWeapon != BWAPI::WeaponTypes::None)
		{
			threat.weight *= (static_cast<double>(groundWeapon.damageAmount()) / groundWeapon.damageCooldown());
			threats.push_back(threat);
		}
	}
}

BWAPI::Unit * ScoutManager::closestEnemyWorker()
{
	BWAPI::Unit * enemyWorker = NULL;
	double maxDist = 0;

	
	BWAPI::Unit * geyser = getEnemyGeyser();
	
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && unit->isConstructing())
		{
			return unit;
		}
	}

	// for each enemy worker
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker())
		{
			double dist = unit->getDistance(geyser);

			if (dist < 800 && dist > maxDist)
			{
				maxDist = dist;
				enemyWorker = unit;
			}
		}
	}

	return enemyWorker;
}

BWAPI::Unit * ScoutManager::getEnemyGeyser()
{
	BWAPI::Unit * geyser = NULL;
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

	BOOST_FOREACH (BWAPI::Unit * unit, enemyBaseLocation->getGeysers())
	{
		geyser = unit;
	}

	return geyser;
}

bool ScoutManager::enemyWorkerInRadius()
{
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && (unit->getDistance(workerScout) < 300))
		{
			return true;
		}
	}

	return false;
}

bool ScoutManager::immediateThreat()
{
	UnitVector enemyAttackingWorkers;
	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && unit->isAttacking())
		{
			enemyAttackingWorkers.push_back(unit);
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawCircleMap(unit->getPosition().x(), unit->getPosition().y(), 5, BWAPI::Colors::Yellow);
		}
	}
	
	if (workerScout->isUnderAttack())
	{
		return true;
	}

	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		double dist = unit->getDistance(workerScout);
		double range = unit->getType().groundWeapon().maxRange();

		if (unit->getType().canAttack() && !unit->getType().isWorker() && (dist <= range + 32))
		{
			return true;
		}
	}

	return false;
}

void ScoutManager::smartMove(BWAPI::Unit * attacker, BWAPI::Position targetPosition)
{
	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Move && currentCommand.getTargetPosition() == targetPosition)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);
}

void ScoutManager::smartAttack(BWAPI::Unit * attacker, BWAPI::Unit * target)
{
	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit && currentCommand.getTarget() == target)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(target);
}