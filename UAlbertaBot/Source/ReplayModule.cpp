#include "ReplayModule.h"
#include "FuzzyModule.h"
#include <iostream>
#include <fstream>
#include <boost\filesystem.hpp>

using namespace BWAPI;
using namespace std;

Player* ReplayModule::player = NULL;
Player* ReplayModule::enemy = NULL;

ReplayModule::ReplayModule()  { ReplayModule::analyzePlayers(); }
ReplayModule::~ReplayModule()
{
	//Delete the lists in the maps
	std::map<const char*,list<int>*>::iterator it;
	for(it=zergUnits.begin(); it!=zergUnits.end(); it++)
	{
		delete it->second;
	}
	for(it=terranUnits.begin(); it!=terranUnits.end(); it++)
	{
		delete it->second;
	}
	for(it=protossUnitsp1.begin(); it!=protossUnitsp1.end(); it++)
	{
		delete it->second;
	}
	for(it=protossUnitsp2.begin(); it!=protossUnitsp2.end(); it++)
	{
		delete it->second;
	}
}


void ReplayModule::createMaps()
{	
	//Buildings 
	protossUnitsAll["Pylon"] = 1;
	protossUnitsAll["Gateway"] = 2;
	protossUnitsAll["Assimilator"] = 3;
	protossUnitsAll["Cybernetics Core"] = 4;
	protossUnitsAll["Citadel of Adun"] = 5;
	protossUnitsAll["Forge"] = 6;
	protossUnitsAll["Photon Cannon"] = 7;
	protossUnitsAll["Stargate"] = 8;
	protossUnitsAll["Robotics Facility"] = 9;
	protossUnitsAll["Robotics Support Bay"] = 10;
	protossUnitsAll["Observatory"] = 11;
	protossUnitsAll["Arbiter Tribunal"] = 12;
	protossUnitsAll["Templar Archives"] = 13;
	protossUnitsAll["Fleet Beacon"] = 14;
	protossUnitsAll["Shield Battery"] = 15;

	protossUnitsAll["Zealot"] = 16;
	protossUnitsAll["Dragoon"] = 17;
	protossUnitsAll["Dark Templar"] = 18;
	protossUnitsAll["High Templar"] = 19;
	protossUnitsAll["Archon"] = 20;
	protossUnitsAll["Dark Archon"] = 21;
	protossUnitsAll["Reaver"] = 22;
	protossUnitsAll["Scout"] = 23;
	protossUnitsAll["Corsair"] = 24;
	protossUnitsAll["Arbiter"] = 25;
	protossUnitsAll["Carrier"] = 26;
	protossUnitsAll["Observer"] = 27;
	protossUnitsAll["Shuttle"] = 28;


	terranUnitsAll["Supply Depot"] = 1;
	terranUnitsAll["Barracks"] = 2;
	terranUnitsAll["Refinery"] = 3;
	terranUnitsAll["Engineering Bay"] = 4;
	terranUnitsAll["Bunker"] = 5;
	terranUnitsAll["Academy"] = 6;
	terranUnitsAll["Missile Turret"] = 7;
	terranUnitsAll["Factory"] = 8;
	terranUnitsAll["Starport"] = 9;
	terranUnitsAll["Armory"] = 10;
	terranUnitsAll["Science Facility"] = 11;
	terranUnitsAll["Comsat Station"] = 12;
	terranUnitsAll["Nuclear Silo"] = 13;
	terranUnitsAll["Machine Shop"] = 14;
	terranUnitsAll["Control Tower"] = 15;
	terranUnitsAll["Physics Lab"] = 16;
	terranUnitsAll["Covert Ops"] = 17;

	terranUnitsAll["Marine"] = 18;
	terranUnitsAll["Firebat"] = 19;
	terranUnitsAll["Medic"] = 20;
	terranUnitsAll["Ghost"] = 21;
	terranUnitsAll["Vulture"] = 22;
	terranUnitsAll["Spider Mine"] = 23;
	terranUnitsAll["Siege Tank"] = 24;
	terranUnitsAll["Goliath"] = 25;
	terranUnitsAll["Wraith"] = 26;
	terranUnitsAll["Dropship"] = 27;
	terranUnitsAll["Science Vessel"] = 28;
	terranUnitsAll["Battlecruiser"] = 29;
	terranUnitsAll["Valkyrie"] = 30;


	zergUnitsAll["Creep Colony"] = 1;
	zergUnitsAll["Sunken Colony"] = 2;
	zergUnitsAll["Spore Colony"] = 3;
	zergUnitsAll["Extractor"] = 4;
	zergUnitsAll["Spawning Pool"] = 5;
	zergUnitsAll["Evolution Chamber"] = 6;
	zergUnitsAll["Hydralisk Den"] = 7;
	zergUnitsAll["Lair"] = 8;
	zergUnitsAll["Spire"] = 9;
	zergUnitsAll["Queen's Nest"] = 10;
	zergUnitsAll["Hive"] = 11;
	zergUnitsAll["Greater Spire"] = 12;
	zergUnitsAll["Nydus Canal"] = 13;
	zergUnitsAll["Ultralisk Cavern"] = 14;
	zergUnitsAll["Defiler Mound"] = 15;

	zergUnitsAll["Zergling"] = 16;
	zergUnitsAll["Hydralisk"] = 17;
	zergUnitsAll["Lurker"] = 18;
	zergUnitsAll["Ultralisk"] = 19;
	zergUnitsAll["Defiler"] = 20;
	zergUnitsAll["Overlord"] = 21;
	zergUnitsAll["Mutalisk"] = 22;
	zergUnitsAll["Scourge"] = 23;
	zergUnitsAll["Queen"] = 24;
	zergUnitsAll["Guardian"] = 25;
	zergUnitsAll["Devourer"] = 26;
	

	//Player
	std::map<const char*,int> *unitsAll;
	std::map<const char*,std::list<int>*>* currentUnits;
	if (getPlayer()->getRace() == Races::Protoss)
	{
		unitsAll = &protossUnitsAll;
		currentUnits = &protossUnitsp1;
	} else if (getPlayer()->getRace() == Races::Terran)
	{
		unitsAll = &terranUnitsAll;
		currentUnits = &terranUnits;
	}else if (getPlayer()->getRace() == Races::Zerg)
	{
		unitsAll = &zergUnitsAll;
		currentUnits = &zergUnits;
	}

	std::map<const char*,int>::iterator it;
	for(it=unitsAll->begin(); it!=unitsAll->end(); it++)
	{
		(*currentUnits)[it->first] = new std::list<int>;
	}

	//Enemy
	if (getEnemy()->getRace() == Races::Protoss)
	{
		unitsAll = &protossUnitsAll;
		currentUnits = &protossUnitsp2;
	} else if (getEnemy()->getRace() == Races::Terran)
	{
		unitsAll = &terranUnitsAll;
		currentUnits = &terranUnits;
	}else if (getEnemy()->getRace() == Races::Zerg)
	{
		unitsAll = &zergUnitsAll;
		currentUnits = &zergUnits;
	}

	for(it=unitsAll->begin(); it!=unitsAll->end(); it++)
	{
		(*currentUnits)[it->first] = new std::list<int>;
	}

}

void ReplayModule::onStart()
{
	ReplayModule::createMaps();

	//Print Hello
	string filename = Broodwar->mapFileName();
	string pathname = Broodwar->mapPathName();
	Broodwar->printf("Hello, my name is Odin! This is a replay.");
	Broodwar->printf("Replay: %s \n", filename.c_str());
	Broodwar->printf("Location: %s \n", pathname.c_str());

	gameSeen = false;
	replayLength = Broodwar->getReplayFrameCount();

	//Check if this replay was checked already
	string folder;
	folder = pathname.substr(0, pathname.size()-filename.size());
	string line;
	ifstream myfile ((folder + "seen.txt").c_str());
	if (myfile.is_open()) {
		while (getline(myfile,line)) {
			if (line.compare(Broodwar->mapFileName()) == 0) {
				Broodwar->printf("This replay has already been seen.");
				gameSeen = true;
				Broodwar->leaveGame();
				return;
			}
		}
		myfile.close();
	} else {
		Broodwar->printf("Unable to open file.");
	}

	//Set this replay as checked if it wasn't already
	ofstream myfile2 ((folder + "seen.txt").c_str(), ios::app);
	if (myfile2.is_open())
	{
		myfile2 << Broodwar->mapFileName().c_str() << endl;
		myfile2.close();
	} else {
		Broodwar->printf("Unable to save file.");
	}
}

void ReplayModule::onFrame()
{
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) 
	{
		/* Doesn't seem to be possible to get size from the Game itself. */
		int width = 640;
		int height = 480;
		int x = width - 200;
		int y = 25;

		ReplayModule::drawUnitInformation(x, y);
	}


	//Update nr of units
	if (Broodwar->getFrameCount()%1000 == 999) //Only update every 1000 frames
	{
		//Update player
		std::string race;
		std::map<const char*,std::list<int>*>* currentUnits;
		if (getPlayer()->getRace() == Races::Protoss)
		{
			currentUnits = &protossUnitsp1;
			race = "Protoss ";
		} else if (getPlayer()->getRace() == Races::Terran)
		{
			currentUnits = &terranUnits;
			race = "Terran ";
		}else if (getPlayer()->getRace() == Races::Zerg)
		{
			currentUnits = &zergUnits;
			race = "Zerg ";
		}
	
		std::map<const char*, std::list<int>*>::iterator it;
		for (it=currentUnits->begin(); it!=currentUnits->end(); it++)
		{
			std::string unitName = race;
			unitName.append(it->first);
			int nrUnits = player->completedUnitCount(BWAPI::UnitTypes::getUnitType(unitName));
			it->second->push_back(nrUnits);
		}

		//Update enemy
		if (getEnemy()->getRace() == Races::Protoss)
		{
			currentUnits = &protossUnitsp2;
			race = "Protoss ";
		} else if (getEnemy()->getRace() == Races::Terran)
		{
			currentUnits = &terranUnits;
			race = "Terran ";
		}else if (getEnemy()->getRace() == Races::Zerg)
		{
			currentUnits = &zergUnits;
			race = "Zerg ";
		}
	
		for (it=currentUnits->begin(); it!=currentUnits->end(); it++)
		{
			std::string unitName = race;
			unitName.append(it->first);
			int nrUnits = enemy->completedUnitCount(BWAPI::UnitTypes::getUnitType(unitName));
			it->second->push_back(nrUnits);
		}
	}
}

void ReplayModule::onEnd(bool isWinner)
{
	std::string filename = Broodwar->mapFileName();
	std::string pathname = Broodwar->mapPathName();
	std::string folder = pathname.substr(0, pathname.size()-filename.size());

	//Replay has ended. Save data to database here
	if(!gameSeen)
	{
		if(!zergUnits.empty())
		{
			writeToFile((folder+"zerg.txt").c_str(), zergUnits, zergUnitsAll);
		}
	
		if(!protossUnitsp1.empty())
		{
			writeToFile((folder+"protoss.txt").c_str(), protossUnitsp1, protossUnitsAll);
		}

		if(!protossUnitsp2.empty())
		{
			writeToFile((folder+"protoss.txt").c_str(), protossUnitsp2, protossUnitsAll);
		}
	
		if(!terranUnits.empty())
		{
			writeToFile((folder+"terran.txt").c_str(), terranUnits, terranUnitsAll);
		}
	}

	//Count seen replays
	string line;
	int nrFiles = 0;
	ifstream myfile ((folder + "seen.txt").c_str());
	if (myfile.is_open()) {
		while (getline(myfile,line)) {
			nrFiles++;
		}
		myfile.close();
	} else {
		Broodwar->printf("Unable to open file.");
	}

	int nrFilesInFolder = 0;
	//Get how many replays there are in total
	for(boost::filesystem::directory_iterator it(folder); it != boost::filesystem::directory_iterator(); ++it)
	{
		nrFilesInFolder++;
	}

	//If seen all, then exit
	if (nrFiles == nrFilesInFolder-1)
	{
		exit(0);
	}

}

void ReplayModule::writeToFile(const char* file, std::map<const char*,std::list<int>*> stuffToWrite, std::map<const char*,int> unitList)
{
	myfile.open (file, std::ios::app);
	
	int timePeriod = 1;
	std::vector<std::list<int>::iterator> temp(unitList.size());
	std::vector<const char*> tempNames(unitList.size());
	std::map<const char*,std::list<int>*>::iterator it;
	std::list<int>::iterator lastElem;

	//Put the units in the correct order
	for(it=stuffToWrite.begin(); it!=stuffToWrite.end(); it++) //Every unit
	{
		//Find unitID and store an iterator to the list<int> in the vector
		std::map<const char*,int>::iterator tempIt;
		for(tempIt=unitList.begin(); tempIt!=unitList.end();)
		{
			if(strcmp(tempIt->first ,it->first)==0)
			{
				tempNames.at(tempIt->second-1) = it->first;
				temp.at(tempIt->second-1) = it->second->begin();
				if (tempIt->second == 1)
				{
					lastElem = it->second->end();
				}
				break;
			}
			tempIt++;
		}
	}
	
	//Actually print units
	for (std::list<int>::iterator itLoop = temp.at(0); itLoop != lastElem;) //For every timeperiod
	{
		myfile << "period" <<timePeriod << ",";

		for (int i = 0; i < unitList.size(); i++) //For every unit
		{
			int nrEnemies = *(temp.at(i));
			temp.at(i)++; //Increase iterator

			myfile << FuzzyModule::getFuzzyNr(nrEnemies,tempNames.at(i));
			if(i<temp.size()-1)
			{
				myfile << ",";
			}
		}

		myfile << "\n";
		itLoop++;
		timePeriod++;
	}

	myfile.close();
}


void ReplayModule::onUnitDestroy(BWAPI::Unit * unit)
{
	//Not needed yet
}

void ReplayModule::onUnitMorph(BWAPI::Unit * unit)
{	
	//Not needed yet	
}

void ReplayModule::onUnitRenegade(BWAPI::Unit * unit)
{
	//Not needed yet
}
void ReplayModule::onUnitCreate(BWAPI::Unit * unit)
{
	//Not needed yet
}

void ReplayModule::onUnitComplete(BWAPI::Unit * unit)
{	
	//Not needed yet
}

/* Displays unit count and predictions during replays. */
void ReplayModule::drawUnitInformation(int x, int y) {
	if (!Options::Debug::DRAW_UALBERTABOT_DEBUG) return;

	BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04UNIT NAME");
	BWAPI::Broodwar->drawTextScreen(x+140, y+20, "\x04#");
	BWAPI::Broodwar->drawTextScreen(x+160, y+20, "\x04X");
	BWAPI::Broodwar->drawTextScreen(x+180, y+20, "\x04->");

	int yspace = 0;
	std::string prefix = "\x04";

	Player* enemy = getEnemy();
	if (enemy == NULL)	return;

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Enemy Unit Information: %s", enemy->getRace().getName().c_str());
	BWAPI::Broodwar->setTextSize(0);

	std::set<Unit*> enemyUnitData = enemy->getUnits();
	BOOST_FOREACH (BWAPI::UnitType t, BWAPI::UnitTypes::allUnitTypes()) 
	{
		int numUnits = enemy->completedUnitCount(t); 
		int numDeadUnits = enemy->deadUnitCount(t);
		int numPredictedUnits = 0;
		
		// if there exist units in the vector
		if (t.getRace() == enemy->getRace() && !t.isHero()) 
		{
			if (t.isDetector())			{ prefix = "\x10"; }		
			else if (t.canAttack())		{ prefix = "\x08"; }		
			else if (t.isBuilding())	{ prefix = "\x03"; }
			else						{ prefix = "\x04"; }
			
			BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "%s%s", prefix.c_str(), t.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x+140, y+40+((yspace)*10), "%s%d", prefix.c_str(), numUnits);
			BWAPI::Broodwar->drawTextScreen(x+160, y+40+((yspace)*10), "%s%d", prefix.c_str(), numDeadUnits);
			BWAPI::Broodwar->drawTextScreen(x+180, y+40+((yspace++)*10), "%s%d", prefix.c_str(), numPredictedUnits);
		}
	}
}

void ReplayModule::analyzePlayers(void) {
	/* Loop over all players. */
	std::set<Player*> allPlayers = Broodwar->getPlayers();
	std::set<Player*>::iterator it;
	bool foundProtoss = false;
	for (it = allPlayers.begin(); it != allPlayers.end(); ++it)
	{
		Player* p = *it; 
		if (p->isObserver() || p->isNeutral())	continue;
		if (p->getRace() == BWAPI::Races::Protoss && !foundProtoss ) {
			player = p;
			foundProtoss = true; 
			continue; 
		}

		enemy = p;
	}

}


Player* ReplayModule::getEnemy() {
	if (!ReplayModule::enemy)	ReplayModule::analyzePlayers();
	return ReplayModule::enemy;
}

Player* ReplayModule::getPlayer() {
	if (!ReplayModule::player)	ReplayModule::analyzePlayers();
	return ReplayModule::player;
}