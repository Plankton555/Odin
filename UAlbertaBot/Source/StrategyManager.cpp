#include "Common.h"
#include "StrategyManager.h"

#define ARMY_COMP_START_VAL (0.1)
#define ARMY_COMP_THRESHOLD (0.5)

const std::string BAYESNET_FOLDER = ODIN_DATA_FILEPATH + "bayesian_networks/";
const std::string OPENINGS_FOLDER = ODIN_DATA_FILEPATH + "openings/";

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
	double enemyUncertaintyFactor = 1.6;
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
	BWAPI::Broodwar->printf(("Strategy state updated to " + stateName).c_str());
 }
 
double StrategyManager::getArmyPotential(BWAPI::Player *player, double economy)
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

double StrategyManager::getEconomyPotential(BWAPI::Player *player)
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

double StrategyManager::getDefensePotential(BWAPI::Player *player)
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
			BWAPI::Broodwar->printf("Enemy race identified as Protoss. Bayesian Network loaded.");
		}
		else if (enemyRace ==  BWAPI::Races::Terran)
		{
			BNetParser parser;
			dlib::parse_xml(BAYESNET_FOLDER + "terran.xdsl", parser);
			bayesianNet = parser.getBayesianNet();
			bayesianNet->UpdateBeliefs();
			BWAPI::Broodwar->printf("Enemy race identified as Terran. Bayesian Network loaded.");
		}
			else if (enemyRace == BWAPI::Races::Zerg)
		{
			BNetParser parser;
			dlib::parse_xml(BAYESNET_FOLDER + "zerg.xdsl", parser);
			bayesianNet = parser.getBayesianNet();
			bayesianNet->UpdateBeliefs();
			BWAPI::Broodwar->printf("Enemy race identified as Zerg. Bayesian Network loaded.");
		}
	}
}

void StrategyManager::addStrategies() 
{
	protossOpeningBook = std::vector<std::string>(NumProtossStrategies);
	terranOpeningBook  = std::vector<std::string>(NumTerranStrategies);
	zergOpeningBook    = std::vector<std::string>(NumZergStrategies);

	
	terranOpeningBook[TerranMarineRush]		= "0 0 0 0 0 1 0 0 3 0 0 3 0 1 0 4 0 0 0 6";
	zergOpeningBook[ZergZerglingRush]		= "0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 1 6";

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

	if (selfRace == BWAPI::Races::Protoss)
	{
		
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
	}
	else if (selfRace == BWAPI::Races::Terran)
	{
		results = std::vector<IntPair>(NumTerranStrategies);
		usableStrategies.push_back(TerranMarineRush);
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		results = std::vector<IntPair>(NumZergStrategies);
		usableStrategies.push_back(ZergZerglingRush);
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
	int i = 0;
	if (myfile.is_open())
	{
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
	BWAPI::Broodwar->printf("Results (%s): (%d %d) (%d %d) (%d %d)", BWAPI::Broodwar->enemy()->getName().c_str(), 
		results[0].first, results[0].second, results[1].first, results[1].second, results[2].first, results[2].second);
	
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
		// Is this ever called / Plankton
		// otherwise return a "random" strategy

        std::string enemyName(BWAPI::Broodwar->enemy()->getName());
        
        if (enemyName.compare("Skynet") == 0)
        {
            currentStrategy = ProtossDarkTemplar;
        }
        else
        {
            currentStrategy = ProtossObserver;
        }
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
	if (selfRace == BWAPI::Races::Protoss)
	{
		return protossOpeningBook[openingStrategy];
	}
	else if (selfRace == BWAPI::Races::Terran)
	{
		return terranOpeningBook[openingStrategy];
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		return zergOpeningBook[openingStrategy];
	} 

	// something wrong, return the protoss one
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

const bool StrategyManager::expandProtossZealotRush() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numNexus =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numZealots =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int frame =					BWAPI::Broodwar->getFrameCount();

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}

	// 2nd Nexus Conditions:
	//		We have 12 or more zealots
	//		It is past frame 7000
	if ((numNexus < 2) && (numZealots > 12 || frame > 9000))
	{
		return true;
	}

	// 3nd Nexus Conditions:
	//		We have 24 or more zealots
	//		It is past frame 12000
	if ((numNexus < 3) && (numZealots > 24 || frame > 15000))
	{
		return true;
	}

	if ((numNexus < 4) && (numZealots > 24 || frame > 21000))
	{
		return true;
	}

	if ((numNexus < 5) && (numZealots > 24 || frame > 26000))
	{
		return true;
	}

	if ((numNexus < 6) && (numZealots > 24 || frame > 30000))
	{
		return true;
	}

	return false;
}

const bool StrategyManager::expandProtossObserver() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}
	int numNexus =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	
	//we need 3 nexus and only 3 nexus. 
	if(numNexus < 3)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
	int now;
	MetaPairVector returnGoal; //These are used in the defend state, but could not have them there
	MetaPairVector cannonGoal; // because it gave some strange error
	MetaPairVector armyGoal;

	if (state == OPENING) // opening has just finished
	{
		updateState();
	}
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		switch (state)
		{
			case OPENING:
				;// should never happen, since getBuildOrderGoal is only called when the opening has ended
				break;

			case ATTACK:
				// do attack
				/*
				if (getCurrentStrategy() == ProtossZealotRush)
				{
					return getProtossZealotRushBuildOrderGoal();
				}
				else if (getCurrentStrategy() == ProtossDarkTemplar)
				{
					return getProtossDarkTemplarBuildOrderGoal();
				}
				else if (getCurrentStrategy() == ProtossDragoons)
				{
					return getProtossDragoonsBuildOrderGoal();
				}
				else if (getCurrentStrategy() == ProtossObserver)
				{
					return getProtossObserverBuildOrderGoal();
				}
				*/
				if(!bayesianNet) break;
				now = BWAPI::Broodwar->getFrameCount();
				if (now - lastBnUpdate > 1000)
				{
					//Update network
					bayesianNet->SetEvidence("TimePeriod",odin_utils::getTimePeriod());
					bayesianNet->UpdateBeliefs();
					lastBnUpdate = now;
				}
				updateArmyComposition();
				if (armyCounters.size() == 0)
				{
					odin_utils::debug("NO COUNTERS NOW!");
				} else {
					MetaPairVector goal = getProtossCounterBuildOrderGoal();
					if (goal.size() > 0)
					{
						return goal;
					} else
					{
						odin_utils::debug("ARMY EXIST; BUT BAD NR COUNTERS!");
					}
				}

				break;

			case DEFEND:
				
				cannonGoal = getStaticDefenceGoal();
				armyGoal = getProtossDragoonsBuildOrderGoal();
				returnGoal.reserve( cannonGoal.size() + armyGoal.size() ); // preallocate memory
				returnGoal.insert( returnGoal.end(), cannonGoal.begin(), cannonGoal.end() );
				returnGoal.insert( returnGoal.end(), armyGoal.begin(), armyGoal.end() );

				BWAPI::Broodwar->printf("Goal set with length: (%d) ", returnGoal.size());

				return returnGoal;

				break;// do defend

			case EXPAND:
				break;// do expand

			default:
				break;
		}

		// if something goes wrong, use zealot goal
		return getProtossZealotRushBuildOrderGoal();
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
	{
		return getTerranBuildOrderGoal();
	}
	else
	{
		return getZergBuildOrderGoal();
	}
}

const MetaPairVector StrategyManager::getProtossCounterBuildOrderGoal()
{
	MetaPairVector goal;

	std::map<std::vector<BWAPI::UnitType>*, double>::iterator it;
	for (it = armyCounters.begin(); it != armyCounters.end(); it++)
	{
		int nrExtraUnits = it->second * 10; //TODO: Some threshold here? Depend on economy?
		if (nrExtraUnits >= 1)
		{
			BWAPI::UnitType wantedType = it->first->at(0);//TODO: Choose cheap or expensive counter (if expensive even exists!)
			int nrUnitsWanted = nrExtraUnits + BWAPI::Broodwar->self()->allUnitCount(wantedType);

			//If we already have included this unit, then just add the nr, don't add a new line
			boolean isIncluded = false;
			MetaPairVector::iterator gIt;
			for (gIt = goal.begin(); gIt != goal.end(); gIt++)
			{
				if (strcmp(gIt->first.getName().c_str(),wantedType.getName().c_str()) == 0)
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
	
	MetaPairVector::iterator gIt;
	odin_utils::debug("=== GOAL STARTING ===");
	for (gIt = goal.begin(); gIt != goal.end(); gIt++)
	{
		stringstream ss;
		ss << gIt->first.getName();
		ss << ": ";
		int nr = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::getUnitType(gIt->first.getName()));
		ss << nr;
		ss << " + ";
		ss << gIt->second - nr;
		odin_utils::debug(ss.str());
		//odin_utils::debug(gIt->first.getName(), gIt->second);
	}
	odin_utils::debug("=== GOAL ENDING ===");
	odin_utils::debug(" ");

	return goal;
}

const MetaPairVector StrategyManager::getProtossObserverBuildOrderGoal() const
{
	//When the first observer has been done transition to ? TODO: THIS IS WHERE WE USE OUR PREDICTION
	MetaPairVector goal;
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int dragoonsWanted = numDragoons > 0 ? numDragoons + 6 : 2;
	int gatewayWanted = numNexusCompleted * 3;
	int probesWanted = numProbes + 6;
	int observersWanted = 1;

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer,	observersWanted));

	return goal;
}

const MetaPairVector StrategyManager::getProtossDragoonsBuildOrderGoal() const
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
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getProtossDarkTemplarBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numDarkTeplar =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int darkTemplarWanted = 0;
	int dragoonsWanted = numDragoons + 6;
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

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numDragoons > 0)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, darkTemplarWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	
	return goal;
}

const MetaPairVector StrategyManager::getProtossZealotRushBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numZealots =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int zealotsWanted = numZealots + 8;
	int dragoonsWanted = numDragoons;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 4;

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

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}

	if (numCyber > 0)
	{
		dragoonsWanted = numDragoons + 2;
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot,	zealotsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getStaticDefenceGoal() const
{

	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	int wantedExtraCannons =	(BWAPI::Broodwar->self()->minerals()/BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice());
	if(wantedExtraCannons<2)
	{
		wantedExtraCannons = 2;
	}
	// the goal to return
	MetaPairVector goal;

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Photon_Cannon, numCannon + wantedExtraCannons));

	return goal;

}

const MetaPairVector StrategyManager::getTerranBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMedics =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numWraith =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Wraith);

	int marinesWanted = numMarines + 12;
	int medicsWanted = numMedics + 2;
	int wraithsWanted = numWraith + 4;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine,	marinesWanted));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;
	
	int numMutas  =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
	int numHydras  =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);

	int mutasWanted = numMutas + 6;
	int hydrasWanted = numHydras + 6;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, 4));
	//goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Stim_Packs,	1));

	//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Medic,		medicsWanted));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

 const int StrategyManager::getCurrentStrategy()
 {
	 return currentStrategy;
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

	/*
	odin_utils::debug("=====FINAL RESULT?=====");
	int size = armyCounters.size();
	odin_utils::debug("Counters Size", size);
	std::map<std::vector<BWAPI::UnitType>*, double>::iterator a;
	for (a = armyCounters.begin(); a != armyCounters.end(); a++)
	{
		odin_utils::debug("First Null?", a->first == NULL);
		stringstream ss;
		ss << a->first->at(0).c_str();
		ss << " and ";
		ss << a->first->at(1).c_str();
		ss << " have prob: ";
		ss << a->second;
		odin_utils::debug(ss.str());
	}
	odin_utils::debug("===== END FINAL RESULT?=====");
	*/
 }


