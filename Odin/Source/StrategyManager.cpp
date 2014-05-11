#include <stdlib.h>
#include "Common.h"
#include "StrategyManager.h"

#define IDLE_WORKERS_THRESHOLD_TO_EXPAND (10)
#define MINERAL_THRESHOLD_TO_EXPAND (2000)

const std::string BAYESNET_FOLDER = ODIN_DATA_FILEPATH + "bayesian_networks/";
const std::string OPENINGS_FOLDER = ODIN_DATA_FILEPATH + "openings/";

const int MINERALS_NEEDED_TO_TECH_EXPENSIVE_COUNTER = 300;
const int GAS_NEEDED_TO_TECH_EXPENSIVE_COUNTER = 150;
const int MAX_UNITS_PER_GOAL = 8;
const int PROBES_PER_CYCLE = 8;
const double ARMY_COMP_START_VAL = 0.1;
const double ARMY_COMP_THRESHOLD = 0.5;

// constructor
StrategyManager::StrategyManager() 
	: firstAttackSent(false)
	, currentStrategy(0)
	, openingStrategy(0)
	, selfRace(BWAPI::Broodwar->self()->getRace())
	, enemyRace(BWAPI::Broodwar->enemy()->getRace())
	, bayesianNet(NULL)
	, state(OPENING)
	, doStateUpdateNow(false)
	, timeSinceLastStateUpdate(0)
	, lastBnUpdate(0)
{
	loadBayesianNetwork();
	addStrategies();
	setStrategy();
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

 void StrategyManager::update()
 {
    if (doStateUpdate())
	{
        updateState();

		timeSinceLastStateUpdate = 0;
		doStateUpdateNow = false;
	}

    switch (state)
	{
		case OPENING:
			break;// follow build order

		case ATTACK:
			break;// do attack

		case DEFEND:
			break;// do defend

		case EXPAND:
			break;// do expand

		default:
			break;
	}
 }

 void StrategyManager::updateState()
 {
	double enemyUncertaintyFactor = 1.5;
	double myEconomy = getEconomyPotential(BWAPI::Broodwar->self());
	double myArmy = getArmyPotential(BWAPI::Broodwar->self(), myEconomy);
	double myDefense = getDefensePotential(BWAPI::Broodwar->self());
	double opEconomy = getEconomyPotential(BWAPI::Broodwar->enemy())*enemyUncertaintyFactor;
	double opArmy = getArmyPotential(BWAPI::Broodwar->enemy(), opEconomy)*enemyUncertaintyFactor;
	double opDefense = getDefensePotential(BWAPI::Broodwar->enemy())*enemyUncertaintyFactor;
	
	//BWAPI::Broodwar->printf("myArmy: %f", myArmy);
	//BWAPI::Broodwar->printf("opArmy: %f", opArmy);
	//BWAPI::Broodwar->printf("opDefense: %f", opDefense);

	if (myArmy < opArmy)
	{
		state = DEFEND;
	}
	else if (myArmy < opDefense)
	{
		// state = EXPAND; // should expand here, but expand is not implemented
		state = ATTACK;
	}
	else
	{
		state = ATTACK;
	}

	std::string stateName = "";
	switch (state)
	{
		case OPENING:
			stateName = "OPENING";
			break;

		case ATTACK:
			stateName = "ATTACK";
			break;

		case DEFEND:
			stateName = "DEFEND";
			break;

		case EXPAND:
			stateName = "EXPAND";
			break;

		default:
			break;
	}
 }
 
const double StrategyManager::getArmyPotential(BWAPI::Player *player, double economy) const
{
	//upgrades (procentuellt)
	double nrKnownUpgrades = 0;
	double totalUpgrades = 0;
	BOOST_FOREACH (BWAPI::UpgradeType upgrade, BWAPI::UpgradeTypes::allUpgradeTypes())
	{
		if (upgrade.getRace() == player->getRace())
		{
			nrKnownUpgrades += InformationManager::Instance().getUpgradeLevel(player, upgrade);
			totalUpgrades += upgrade.maxRepeats();
		}
	}
	double nrKnownTechs = 0;
	double totalTechs = 0;
	BOOST_FOREACH (BWAPI::TechType tech, BWAPI::TechTypes::allTechTypes())
	{
		if (tech.getRace() == player->getRace())
		{
			nrKnownTechs += InformationManager::Instance().hasResearched(player, tech);
			totalTechs++;
		}
	}
	double techAndUpgradePercent = (nrKnownUpgrades+nrKnownTechs)/(totalUpgrades+totalTechs);

	// army size and production facilities
	double nrArmyUnits = 0;
	double nrProductionFacilities = 0;
	BOOST_FOREACH (BWAPI::UnitType t, BWAPI::UnitTypes::allUnitTypes()) 
	{
		int numUnits = InformationManager::Instance().getNumUnits(t, player);
		if (numUnits > 0)
		{
			if (t.canAttack() && !t.isWorker() && !t.isBuilding())
			{
				nrArmyUnits += numUnits;
			}
			if (t == BWAPI::UnitTypes::Terran_Bunker)
			{
				nrArmyUnits += 4; // potentially can hold 4 units
			}

			if (t.isBuilding() && t.canProduce())
			{
				nrProductionFacilities += numUnits;
			}
			if (t == BWAPI::UnitTypes::Zerg_Hatchery)
			{
				nrProductionFacilities += 2;
			}
			else if (t == BWAPI::UnitTypes::Zerg_Lair)
			{
				nrProductionFacilities +=4;
			}
			else if (t == BWAPI::UnitTypes::Zerg_Hive)
			{
				nrProductionFacilities += 6;
			}
		}
	}

	double potential = 2*techAndUpgradePercent + 1.2*nrArmyUnits + 0.3*(nrProductionFacilities)*(economy+2);
	return potential;
}

const double StrategyManager::getEconomyPotential(BWAPI::Player *player) const 
{
	// this should work as a guesstimate
	double nrKnownWorkers = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Drone, player);
	nrKnownWorkers += InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_SCV, player);
	nrKnownWorkers += InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Probe, player);

	double nrRegions = InformationManager::Instance().getOccupiedRegions(player).size();

	double nrKnownBases = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hatchery, player);
	nrKnownBases += InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lair, player);
	nrKnownBases += InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hive, player);
	nrKnownBases += InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Command_Center, player);
	nrKnownBases += InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Nexus, player);
	
	double potential = nrKnownWorkers*0.09 + nrRegions*0.8 + nrKnownBases*1.1;
	return potential;
}

const double StrategyManager::getDefensePotential(BWAPI::Player *player) const
{
	double defenseStructures = 2*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Bunker, player);
	defenseStructures += 0.9*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Missile_Turret, player);
	defenseStructures += 0.8*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spore_Colony, player);
	defenseStructures += 1.1*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Sunken_Colony, player);
	defenseStructures += 1*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Photon_Cannon, player);

	double defenseUnits = 1*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, player);
	defenseUnits += 1.6*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, player);
	defenseUnits += 1.2*InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lurker, player);

	// Can (should) also take number of units inside bases into account. Not necessary right now though.

	double potential = 1*defenseStructures + 0.8*defenseUnits;
	return 5*potential;
}

 bool StrategyManager::doStateUpdate()
 {
	 int updateInterval = 500; //frames

	 if (state == OPENING)
	 {
		 return false;
	 }

	 timeSinceLastStateUpdate++; //assumes this method is called every frame
	 if (timeSinceLastStateUpdate > updateInterval)
	 {
		 doStateUpdateNow = true;
	 }
	 return doStateUpdateNow;
 }

void StrategyManager::onUnitShow(BWAPI::Unit * unit)
{
	if (enemyRace == BWAPI::Races::Unknown || enemyRace == BWAPI::Races::Random) //Don't really know which one it is set as, but it doesn't matter
	{
		enemyRace = BWAPI::Broodwar->enemy()->getRace();
		loadBayesianNetwork();
	}
}

void StrategyManager::loadBayesianNetwork()
{
	if (!bayesianNet)
	{
		if (enemyRace ==  BWAPI::Races::Protoss)
		{
			BNetParser parser;
			dlib::parse_xml(BAYESNET_FOLDER + "protoss.xdsl", parser);
			bayesianNet = parser.getBayesianNet();
			bayesianNet->UpdateBeliefs();
		}
		else if (enemyRace ==  BWAPI::Races::Terran)
		{
			BNetParser parser;
			dlib::parse_xml(BAYESNET_FOLDER + "terran.xdsl", parser);
			bayesianNet = parser.getBayesianNet();
			bayesianNet->UpdateBeliefs();
		}
			else if (enemyRace == BWAPI::Races::Zerg)
		{
			BNetParser parser;
			dlib::parse_xml(BAYESNET_FOLDER + "zerg.xdsl", parser);
			bayesianNet = parser.getBayesianNet();
			bayesianNet->UpdateBeliefs();
		}
	}
}

void StrategyManager::addStrategies() 
{
	protossOpeningBook = std::vector<std::string>(NumProtossStrategies);

	// read in the name of the read and write directories from settings file
	struct stat buf;

	// if the file doesn't exist something is wrong so just set them to default settings
	if (stat(Options::FileIO::FILE_SETTINGS, &buf) == -1)
	{
		readDir = OPENINGS_FOLDER;
		writeDir = OPENINGS_FOLDER;
	}
	else
	{
		std::ifstream f_in(Options::FileIO::FILE_SETTINGS);
		getline(f_in, readDir);
		getline(f_in, writeDir);
		f_in.close();
	}
	enemyIsRandom = false;
	if (enemyRace == BWAPI::Races::Protoss)
	{	
		loadStrategiesFromFile("protoss_strats.txt");
	}
	else if (enemyRace == BWAPI::Races::Terran)
	{
		loadStrategiesFromFile("terran_strats.txt");
	}
	else if (enemyRace == BWAPI::Races::Zerg)
	{
		loadStrategiesFromFile("zerg_strats.txt");
	}
	else
	{
		enemyIsRandom = true;
		BWAPI::Broodwar->printf("Enemy Race Unknown");
		loadStrategiesFromFile("random_strats.txt");
	}


	if (Options::Modules::USING_STRATEGY_IO)
	{
		readResults();
	}
}

void StrategyManager::loadStrategiesFromFile(std::string filename)
{
	std::ifstream myfile ((readDir + filename).c_str());
	std::string line;
	if (myfile.is_open())
	{
		int i = 0;
		while (getline(myfile,line)) 
		{	
			protossOpeningBook[i]=line;
			usableStrategies.push_back(i);
			i++;
		}
		myfile.close();
		results = std::vector<IntPair>(i);
	}else
	{
		BWAPI::Broodwar->printf(
			"Unable to open file, some things may not be working or the entire program may crash glhf :).");
	}
}

void StrategyManager::readResults()
{

	struct stat buf;

	// the file corresponding to the enemy's previous results	
	if(enemyIsRandom)
	{
		readFile = readDir +  "random.txt";
	}else
	{
		readFile= readDir + BWAPI::Broodwar->enemy()->getRace().c_str() + ".txt";	
	}

	std::ifstream myfile ((readFile).c_str());
	std::string line;
	int i = 0;
	int j = 0;
	if (myfile.is_open())
	{
		while (getline(myfile,line)) 
		{	
			if(i%2==0)
			{
				results[i/2].first = atoi(line.c_str());
			}else
			{
				results[i/2].second = atoi(line.c_str());			
			}
			i++;
		}
		myfile.close();
	}else
	{
		std::fill(results.begin(), results.end(), IntPair(0,0));
		BWAPI::Broodwar->printf(
			"Unable to open file for recorded data, starting from scratch");
	}
	//BWAPI::Broodwar->printf("Results (%s): (%d %d) (%d %d) (%d %d)", BWAPI::Broodwar->enemy()->getName().c_str(), 
	//	results[0].first, results[0].second, results[1].first, results[1].second, results[2].first, results[2].second);
	
}

void StrategyManager::writeResults()
{
	std::string writeFile = writeDir  + BWAPI::Broodwar->enemy()->getRace().c_str() + ".txt";

	if(enemyIsRandom)
	{
		writeFile = writeDir +  "random.txt";
	}

	std::ofstream f_out(writeFile.c_str());
	for(int i = 0; i < results.size(); i++)
	{
		f_out << results[i].first   << "\n";
		f_out << results[i].second  << "\n";	
	}
	f_out.close();
}

void StrategyManager::setStrategy()
{
	// if we are using file io to determine strategy, do so
	if (Options::Modules::USING_STRATEGY_IO)
	{
		double bestUCB = -1;
		int bestStrategyIndex = 0;

		// UCB requires us to try everything once before using the formula
		for (size_t strategyIndex(0); strategyIndex<usableStrategies.size(); ++strategyIndex)
		{
			int sum = results[usableStrategies[strategyIndex]].first + results[usableStrategies[strategyIndex]].second;

			if (sum == 0)
			{
				currentStrategy = usableStrategies[strategyIndex];
				openingStrategy = currentStrategy; // since currentStrategy can change while openingStrategy should remain the same
				return;
			}
		}

		// if we have tried everything once, set the maximizing ucb value
		for (size_t strategyIndex(0); strategyIndex<usableStrategies.size(); ++strategyIndex)
		{
			double ucb = getUCBValue(usableStrategies[strategyIndex]);

			if (ucb > bestUCB)
			{
				bestUCB = ucb;
				bestStrategyIndex = strategyIndex;
			}
		}
		
		currentStrategy = usableStrategies[bestStrategyIndex];
		openingStrategy = currentStrategy;
	}
	else
	{
		//Will crasch if no strategies.. and lower #s more likely.
		int randomIndex = std::rand() % usableStrategies.size();
		currentStrategy = usableStrategies[randomIndex];
	}

}

void StrategyManager::onEnd(const bool isWinner)
{
	// write the win/loss data to file if we're using IO
	if (Options::Modules::USING_STRATEGY_IO)
	{
		// if the game ended before the tournament time limit
		if (BWAPI::Broodwar->getFrameCount() < Options::Tournament::GAME_END_FRAME)
		{
			if (isWinner)
			{
				results[openingStrategy].first = results[openingStrategy].first + 1;
			}
			else
			{
				results[openingStrategy].second = results[openingStrategy].second + 1;
			}
		}
		// otherwise game timed out so use in-game score
		else
		{
			if (getScore(BWAPI::Broodwar->self()) > getScore(BWAPI::Broodwar->enemy()))
			{
				results[openingStrategy].first = results[openingStrategy].first + 1;
			}
			else
			{
				results[openingStrategy].second = results[openingStrategy].second + 1;
			}
		}
		
		writeResults();
	}
}

const double StrategyManager::getUCBValue(const size_t & strategy) const
{
	double totalTrials(0);
	for (size_t s(0); s<usableStrategies.size(); ++s)
	{
		totalTrials += results[usableStrategies[s]].first + results[usableStrategies[s]].second;
	}

	double C		= 0.7;
	double wins		= results[strategy].first;
	double trials	= results[strategy].first + results[strategy].second;

	double ucb = (wins / trials) + C * sqrt(std::log(totalTrials) / trials);

	return ucb;
}

const int StrategyManager::getScore(BWAPI::Player * player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const std::string StrategyManager::getOpeningBook() const
{
	return protossOpeningBook[openingStrategy];
}

// when do we want to defend with our workers?
// this function can only be called if we have no fighters to defend with
const int StrategyManager::defendWithWorkers()
{
	if (!Options::Micro::WORKER_DEFENSE)
	{
		return false;
	}

	// our home nexus position
	BWAPI::Position homePosition = BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition();;

	// enemy units near our workers
	int enemyUnitsNearWorkers = 0;

	// defense radius of nexus
	int defenseRadius = 300;

	// fill the set with the types of units we're concerned about
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// if it's a zergling or a worker we want to defend
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			if (unit->getDistance(homePosition) < defenseRadius)
			{
				enemyUnitsNearWorkers++;
			}
		}
	}

	// if there are enemy units near our workers, we want to defend
	return enemyUnitsNearWorkers;
}

// called by combat commander to determine whether or not to send an attack force
// freeUnits are the units available to do this attack
const bool StrategyManager::doAttack(const std::set<BWAPI::Unit *> & freeUnits)
{
	int ourForceSize = (int)freeUnits.size();

	int numUnitsNeededForAttack = 2;

	bool doAttack  = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) >= 1
					|| ourForceSize >= numUnitsNeededForAttack;

	if (doAttack)
	{
		firstAttackSent = true;
	}
	switch (state)
	{
		case OPENING:
			// atm the same as in ATTACK
			return doAttack || firstAttackSent;
			break;

		case ATTACK:
			// do attack
			return doAttack || firstAttackSent;
			break;

		case DEFEND:
			return false;// do defend
			break;

		case EXPAND:
			return firstAttackSent;// do expand
			break;

		default:
			return false; // do not attack
			break;
	}
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
	MetaPairVector returnGoal; //These are used in the defend state, but could not have them there
	MetaPairVector cannonGoal; // because it gave some strange error
	MetaPairVector armyGoal;

	if (state == OPENING) // opening has just finished
	{
		updateState();
	}
	switch (state)
	{
		case OPENING:
			;// should never happen, since getBuildOrderGoal is only called when the opening has ended
			break;

		case ATTACK:
			// do attack
			updateBNandArmyComp();
			if (bayesianNet == NULL) break;
			if (armyCounters.size() > 0)
			{
				MetaPairVector goal = getProtossCounterBuildOrderGoal();
				if (goal.size() > 1)
				{
					return goal;
				} 
				else 
				{ 
					BWAPI::Broodwar->printf("ACG NOT EMPTY - NO GOAL!");
				}
			} else { BWAPI::Broodwar->printf("ACG EMPTY!"); }

			break;

		case DEFEND:
			updateBNandArmyComp();
			if (bayesianNet == NULL || armyCounters.size() == 0)
			{
				armyGoal = getDefaultBuildOrderGoal();
			} else
			{
				armyGoal = getProtossCounterBuildOrderGoal();
				if (armyGoal.size() < 2) armyGoal = getDefaultBuildOrderGoal(); //If counters is empty, then just go with default
			}

			cannonGoal = getStaticDefenceGoal();
				
			returnGoal.reserve( cannonGoal.size() + armyGoal.size() ); // preallocate memory
			returnGoal.insert( returnGoal.end(), armyGoal.begin(), armyGoal.end() );
			returnGoal.insert( returnGoal.end(), cannonGoal.begin(), cannonGoal.end() );
			return returnGoal;

			break;// do defend

		case EXPAND:
			break;// do expand

		default:
			break;
	}

	// if something goes wrong, use the default goal
	return getDefaultBuildOrderGoal();
}

bool comPair( const std::pair<MetaType, UnitCountType>& i, const std::pair<MetaType, UnitCountType>& j ) 
{
	int iNow = BWAPI::Broodwar->self()->allUnitCount(i.first.unitType);
	int jNow = BWAPI::Broodwar->self()->allUnitCount(j.first.unitType);
	return (i.second - iNow) < (j.second - jNow);
}

const MetaPairVector StrategyManager::getProtossCounterBuildOrderGoal()
{
	MetaPairVector goal;
	bool shouldMakeStorm = false;
	bool techedThisGoal = false;
	std::map<std::vector<BWAPI::UnitType>*, double>::iterator it;
	for (it = armyCounters.begin(); it != armyCounters.end(); it++)
	{
		int nrExtraUnits = it->second * MAX_UNITS_PER_GOAL;
		if (nrExtraUnits >= 1)
		{
			//Find out what counter we want to build (cheap or expensive)
			BWAPI::UnitType wantedType;
			if (it->first->size() > 1) //Have an expensive counter
			{
				//Check if we have teched for it yet
				bool haveTeched = true;
				std::map<BWAPI::UnitType, int> m = it->first->at(1).requiredUnits();
				std::map<BWAPI::UnitType, int>::iterator mit;
				for (mit = m.begin(); mit != m.end(); mit++)
				{
					if (BWAPI::Broodwar->self()->allUnitCount(mit->first) < mit->second)
					{
						haveTeched = false;
						break;
					}
				}

				if (haveTeched) //have Teched for expensive
				{
					if (nrExtraUnits > 1) //If we want more than 1 unit, then let half the units be cheap units
					{
						nrExtraUnits /= 2;
						int unitsNow = BWAPI::Broodwar->self()->allUnitCount(it->first->at(0));
						//If we already have included this unit, then just add the nr, don't add a new line
						boolean isIncluded = false;
						MetaPairVector::iterator gIt;
						for (gIt = goal.begin(); gIt != goal.end(); gIt++)
						{
							if (gIt->first.unitType == it->first->at(0))
							{
								gIt->second += nrExtraUnits;
								isIncluded = true;
								break;
							}
						}

						if (!isIncluded)
						{
							goal.push_back(MetaPair(it->first->at(0), unitsNow+nrExtraUnits));
						}

					}
					wantedType = it->first->at(1);
				} else
				{
					wantedType = it->first->at(0);

					//Tech to expensive if can afford
					if (!techedThisGoal && BWAPI::Broodwar->self()->minerals() > MINERALS_NEEDED_TO_TECH_EXPENSIVE_COUNTER && 
						BWAPI::Broodwar->self()->gas() > GAS_NEEDED_TO_TECH_EXPENSIVE_COUNTER)
					{
						techedThisGoal = true;
						BWAPI::Broodwar->printf("Teching for %s", it->first->at(1).getName().c_str());
						for(mit = m.begin(); mit != m.end(); mit++)
						{
							if (BWAPI::Broodwar->self()->allUnitCount(mit->first) < mit->second)
							{
								// change 1 to "mit->second - allUnitCount" if there are units that require
								// more than 1 of a specified unit (eg Dark Archon needs 2 Dark Templars) 
								goal.push_back(MetaPair(mit->first, 1));
							}
						}
					}
				}
			} else
			{
				wantedType = it->first->at(0);
			}

			//Add the number of units we want
			int nrUnitsNow = BWAPI::Broodwar->self()->allUnitCount(wantedType);
			int nrUnitsWanted = nrExtraUnits + nrUnitsNow;

			if (wantedType == BWAPI::UnitTypes::Protoss_Photon_Cannon
				|| wantedType == BWAPI::UnitTypes::Protoss_High_Templar
				|| wantedType == BWAPI::UnitTypes::Protoss_Reaver
				|| wantedType == BWAPI::UnitTypes::Protoss_Observer
				|| wantedType == BWAPI::UnitTypes::Protoss_Dark_Templar)
			{
				int maxUnits = DataModule::getSomeFuzzy(wantedType.getName());
				nrUnitsWanted = std::min(maxUnits, nrUnitsWanted); //Don't mass out tons of units not meant to
				if (nrUnitsWanted <= nrUnitsNow) continue; //Don't even add this line if we have enough cannons
				nrExtraUnits = 0; //So we don't add any extra if photon cannon is added already
			}

			if(wantedType == BWAPI::UnitTypes::Protoss_High_Templar)
			{
				shouldMakeStorm = true;
			}

			//If we already have included this unit, then just add the nr, don't add a new line
			boolean isIncluded = false;
			MetaPairVector::iterator gIt;
			for (gIt = goal.begin(); gIt != goal.end(); gIt++)
			{
				if (gIt->first.unitType == wantedType)
				{
					gIt->second += nrExtraUnits;
					isIncluded = true;
					break;
				}
			}

			if (!isIncluded)
			{
				goal.push_back(MetaPair(wantedType, nrUnitsWanted));
			}
		}
	}
	//Fix so the goals will not be too big. Take top 3 units but beware of Observers
	std::sort(goal.begin(), goal.end(), comPair);
	MetaPairVector::iterator start = goal.begin();
	while( goal.end() - start > 3)
	{
		BWAPI::UnitType ut = start->first.unitType;
		//dont remove buildings or observers form the queue
		if ( ut.isBuilding() || ut == BWAPI::UnitTypes::Protoss_Observer)
		{
			start += 1;
		}
		else
		{
			goal.erase(start);
		}
	}
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);

	//add psi-storm if we use high templars
	
	if(shouldMakeStorm && !BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Psionic_Storm) 
		&& !BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Psionic_Storm))
	{
		goal.push_back(MetaPair(BWAPI::TechTypes::Psionic_Storm,1));
	}

	if(shouldExpand())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}
	if (numNexusAll > 1)
	{
		int probesWanted = numProbes + PROBES_PER_CYCLE;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	}
	
	return goal;
}

const MetaPairVector StrategyManager::getDefaultBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int dragoonsWanted = numDragoons > 0 ? numDragoons + 6 : 2;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}
	else
	{
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}

		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || numDragoons > 6 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 2));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + PROBES_PER_CYCLE;
	}

	if(shouldExpand())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getStaticDefenceGoal() const
{

	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	int wantedTotalCannons = numNexusAll*3;
	int cannonsWanted = wantedTotalCannons - numCannon;
	// the goal to return
	MetaPairVector goal;
	
	if(cannonsWanted > 0)
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Photon_Cannon, numCannon + 1));
	}

	return goal;

}

const bool StrategyManager::shouldExpand() const
{

	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numNexus =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int frame =					BWAPI::Broodwar->getFrameCount();
	int minerals =				BWAPI::Broodwar->self()->minerals();

	//If we dont have any nexuses it is a good time to exp. 
	if( numNexus == 0 )
	{
		return true;
	}

	double enemyUncertaintyFactor = 1.5;
	double myEconomy = getEconomyPotential(BWAPI::Broodwar->self());
	double myArmy = getArmyPotential(BWAPI::Broodwar->self(), myEconomy);
	double myDefense = getDefensePotential(BWAPI::Broodwar->self());
	double opEconomy = getEconomyPotential(BWAPI::Broodwar->enemy())*enemyUncertaintyFactor;
	double opArmy = getArmyPotential(BWAPI::Broodwar->enemy(), opEconomy)*enemyUncertaintyFactor;
	double opDefense = getDefensePotential(BWAPI::Broodwar->enemy())*enemyUncertaintyFactor;

	//If we are already building a nexus
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus) < 
			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus))
	{
		return false;
	}

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > IDLE_WORKERS_THRESHOLD_TO_EXPAND) 
	{
		BWAPI::Broodwar->printf("Exping - too many idle workers");
		return true;
	}

	if(WorkerManager::Instance().getNumMineralWorkers() / numNexus < 11)
	{
		return false;
	}

	if (minerals > 600 && myArmy>opArmy*enemyUncertaintyFactor)
	{
		BWAPI::Broodwar->printf("Exping - good army");
		return true;
	}

	if(myEconomy<opEconomy*enemyUncertaintyFactor)
	{
		BWAPI::Broodwar->printf("Exping - good economy");
		return true;
	}

	if(minerals > MINERAL_THRESHOLD_TO_EXPAND)
	{
		BWAPI::Broodwar->printf("Exping - much minerals");
		return true;
	}

	return false;
}

 const int StrategyManager::getCurrentStrategy()
 {
	 return currentStrategy;
 }

 void StrategyManager::updateBNandArmyComp()
 {
	if(!bayesianNet) return;
	int now = BWAPI::Broodwar->getFrameCount();
	if (now - lastBnUpdate > 1000)
	{
		//Update network
		bayesianNet->SetEvidence("TimePeriod",odin_utils::getTimePeriod());
		bayesianNet->UpdateBeliefs();
		lastBnUpdate = now;
	}
	updateArmyComposition();
 }

 void StrategyManager::updateArmyComposition()
{
	//Read army comp
	std::map<BWAPI::UnitType, double>	armyComposition;
	std::set<BWAPI::UnitType> allUnits = BWAPI::UnitTypes::allUnitTypes();
	std::set<BWAPI::UnitType>::iterator it;
	for (it = allUnits.begin(); it != allUnits.end(); it++)
	{
		std::string name = it->c_str();
		odin_utils::shortenUnitName(name);

		if (it->canMove() && bayesianNet->exists(name) && bayesianNet->ReadProbability(name, 1) > ARMY_COMP_THRESHOLD
					&& DataModule::getCounter(it->c_str()) != NULL)
		{
			//Set initial value
			armyComposition[*it] = ARMY_COMP_START_VAL;
		}
	}


	//Add value depending on the percentage of the seen units
	int totalNrUnits = 0;
	std::map<BWAPI::UnitType, double>::iterator compIt;
	for (compIt = armyComposition.begin(); compIt != armyComposition.end(); compIt++)
	{
		totalNrUnits += BWAPI::Broodwar->enemy()->completedUnitCount(compIt->first);
	}
	
	double totalSum = 0;
	for (compIt = armyComposition.begin(); compIt != armyComposition.end(); compIt++)
	{
		int nrUnits = BWAPI::Broodwar->enemy()->completedUnitCount(compIt->first);
		if (nrUnits) //Don't add any value if none has been seen
		{
			double ratio = nrUnits;
			ratio /= totalNrUnits;
			compIt->second += ratio;
		}

		totalSum += compIt->second;
	}

	//Save normalised counters
	
	armyCounters.clear();
	for (compIt = armyComposition.begin(); compIt != armyComposition.end(); compIt++)
	{
		//compIt->second = compIt->second/totalSum;
		armyCounters[DataModule::getCounter(compIt->first.c_str())] = compIt->second/totalSum;
	}
 }


