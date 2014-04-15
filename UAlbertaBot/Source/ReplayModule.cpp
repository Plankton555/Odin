#include "ReplayModule.h"
#include "FuzzyModule.h"
#include <iostream>
#include <fstream>
#include <boost\filesystem.hpp>
#include "Common.h"

using namespace BWAPI;
using namespace std;

const std::string REPLAY_DATA_PATH = ODIN_DATA_FILEPATH + "replaydatastuff/";
const std::string SEEN_REPLAYS_PATH = REPLAY_DATA_PATH + "seen_replays.txt";

Player* ReplayModule::player = NULL;
Player* ReplayModule::enemy = NULL;

ReplayModule::ReplayModule()  { ReplayModule::analyzePlayers(); }
ReplayModule::~ReplayModule() { }


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

	protossTechAll["Hallucination"] = 29;
	protossTechAll["Maelstrom"] = 30;
	protossTechAll["Mind_Control"] = 31;
	protossTechAll["Recall"] = 32;
	protossTechAll["Psionic_Storm"] = 33;
	protossTechAll["Stasis_Field"] = 34;
	protossTechAll["Disruption_Web"] = 35;

	protossUpgradesAll["Protoss_Air_Weapons"] = 36;
	protossUpgradesAll["Protoss_Ground_Armor"] = 37;
	protossUpgradesAll["Protoss_Ground_Weapons"] = 38;
	protossUpgradesAll["Protoss_Plasma_Shields"] = 39;
	protossUpgradesAll["Protoss_Air_Armor"] = 40;
	protossUpgradesAll["Apial_Sensors"] = 41;
	protossUpgradesAll["Argus_Jewel"] = 42;
	protossUpgradesAll["Argus_Talisman"] = 43;
	protossUpgradesAll["Carrier_Capacity"] = 44;
	protossUpgradesAll["Gravitic_Boosters"] = 45;
	protossUpgradesAll["Gravitic_Drive"] = 46;
	protossUpgradesAll["Gravitic_Thrusters"] = 47;
	protossUpgradesAll["Khaydarin_Amulet"] = 48;
	protossUpgradesAll["Khaydarin_Core"] = 49;
	protossUpgradesAll["Leg_Enhancements"] = 50;
	protossUpgradesAll["Reaver_Capacity"] = 51;
	protossUpgradesAll["Scarab_Damage"] = 52;
	protossUpgradesAll["Sensor_Array"] = 53;
	protossUpgradesAll["Singularity_Charge"] = 54;
	

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

	terranTechAll["Tank_Siege_Mode"] = 31;
	terranTechAll["Cloaking_Field"] = 32;
	terranTechAll["Yamato_Gun"] = 33;
	terranTechAll["EMP_Shockwave"] = 34;
	terranTechAll["Irradiate"] = 35;
	terranTechAll["Personnel_Cloaking"] = 36;
	terranTechAll["Lockdown"] = 37;
	terranTechAll["Stim_Packs"] = 38;
	terranTechAll["Restoration"] = 39;
	terranTechAll["Optical_Flare"] = 40;

	terranUpgradesAll["Terran_Infantry_Armor"] = 41;
	terranUpgradesAll["Terran_Infantry_Weapons"] = 42;
	terranUpgradesAll["Terran_Ship_Plating"] = 43;
	terranUpgradesAll["Terran_Ship_Weapons"] = 44;
	terranUpgradesAll["Terran_Vehicle_Plating"] = 45;
	terranUpgradesAll["Terran_Vehicle_Weapons"] = 46;
	terranUpgradesAll["Ion_Thrusters"] = 47;
	terranUpgradesAll["Charon_Boosters"] = 48;
	terranUpgradesAll["Apollo_Reactor"] = 49;
	terranUpgradesAll["Colossus_Reactor"] = 50;
	terranUpgradesAll["Titan_Reactor"] = 51;
	terranUpgradesAll["Moebius_Reactor"] = 52;
	terranUpgradesAll["Ocular_Implants"] = 53;
	terranUpgradesAll["U_238_Shells"] = 54;
	terranUpgradesAll["Caduceus_Reactor"] = 55;

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

	zergTechAll["Spawn_Broodlings"] = 27;
	zergTechAll["Ensnare"] = 28;
	zergTechAll["Plague"] = 29;
	zergTechAll["Consume"] = 30;
	zergTechAll["Lurker_Aspect"] = 31;
	zergTechAll["Burrowing"] = 32;

	zergUpgradesAll["Zerg_Carapace"] = 33;
	zergUpgradesAll["Zerg_Flyer_Attacks"] = 34;
	zergUpgradesAll["Zerg_Flyer_Carapace"] = 35;
	zergUpgradesAll["Zerg_Melee_Attacks"] = 36;
	zergUpgradesAll["Zerg_Missile_Attacks"] = 37;
	zergUpgradesAll["Gamete_Meiosis"] = 38;
	zergUpgradesAll["Metasynaptic_Node"] = 39;
	zergUpgradesAll["Chitinous_Plating"] = 40;
	zergUpgradesAll["Anabolic_Synthesis"] = 41;
	zergUpgradesAll["Muscular_Augments"] = 42;
	zergUpgradesAll["Grooved_Spines"] = 43;
	zergUpgradesAll["Adrenal_Glands"] = 44;
	zergUpgradesAll["Metabolic_Boost"] = 45;
	zergUpgradesAll["Antennae"] = 46;
	zergUpgradesAll["Pneumatized_Carapace"] = 47;
	zergUpgradesAll["Ventral_Sacs"] = 48;

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
	ifstream myfile (SEEN_REPLAYS_PATH.c_str());
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
	ofstream myfile2 (SEEN_REPLAYS_PATH.c_str(), ios::app);
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

	//Check if any morphing buildigns is completed
	std::list<Unit*>::iterator it;
	for(it=morphingBuildings.begin(); it!=morphingBuildings.end();)
	{	if((*it)->isCompleted())
		{
			//Broodwar->printf("%s was morphed at time (%d)", (*it)->getType().c_str(), Broodwar->getFrameCount()); //Used for debugging

			if(zergUnits.count((*it)->getType().c_str())==0)
			{
				if((*it)->getType().getRace()==Races::Zerg)
				{	
					const char* temp = (*it)->getType().c_str() + 5;
					zergUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
				}else if((*it)->getType().getRace()==Races::Protoss&&getEnemy()->getRace() == BWAPI::Races::Protoss)
				{	
					const char* temp = (*it)->getType().c_str() + 8;
					if((*it)->getPlayer() == getPlayer())
					{
						protossUnitsp1.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
					}else
					{
						protossUnitsp2.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
					}
				}else if((*it)->getType().getRace()==Races::Terran)
				{	
					const char* temp = (*it)->getType().c_str() + 7;
					terranUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
				}	
			}
			it = morphingBuildings.erase(it);
		}else
		{
			it++;
		}
	}
}

void ReplayModule::onEnd(bool isWinner)
{

	//Replay has ended. Save data to database here
	if(!gameSeen)
	{
		if(!zergUnits.empty())
		{
			writeToFile((REPLAY_DATA_PATH+"zerg.txt").c_str(), zergUnits, zergUnitsAll);
		}
	
		if(!protossUnitsp1.empty())
		{
			// If PvP, protossUnitsp1 stores the units for player 1
			writeToFile((REPLAY_DATA_PATH+"protoss.txt").c_str(), protossUnitsp1, protossUnitsAll);
		}

		if(!protossUnitsp2.empty())
		{
			// If PvP, protossUnitsp2 stores the units for player 2
			writeToFile((REPLAY_DATA_PATH+"protoss.txt").c_str(), protossUnitsp2, protossUnitsAll);
		}
	
		if(!terranUnits.empty())
		{
			writeToFile((REPLAY_DATA_PATH+"terran.txt").c_str(), terranUnits, terranUnitsAll);
		}
	}

	//Count seen replays
	string line;
	int nrFiles = 0;
	ifstream myfile (SEEN_REPLAYS_PATH.c_str());
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
	std::string filename = Broodwar->mapFileName();
	std::string pathname = Broodwar->mapPathName();
	std::string replayFolder = pathname.substr(0, pathname.size()-filename.size()); // directory containing replays
	for(boost::filesystem::directory_iterator it(replayFolder); it != boost::filesystem::directory_iterator(); ++it)
	{
		nrFilesInFolder++;
	}

	//If seen all, then exit
	if (nrFiles == nrFilesInFolder)
	{
		exit(0);
	}

}

void ReplayModule::writeToFile(const char* file, std::map<const char*,int> stuffToWrite, std::map<const char*,int> unitList)
{
	myfile.open (file, std::ios::app);


	std::vector<int> temp(unitList.size()+1);
	std::map<const char*,int>::iterator it;
	for(it=stuffToWrite.begin(); it!=stuffToWrite.end();)
	{	

		std::map<const char*,int>::iterator tempIt;
		for(tempIt=unitList.begin(); tempIt!=unitList.end();)
		{
			if(strcmp(tempIt->first ,it->first)==0)
			{
				temp.at(tempIt->second) = it->second;
				//myfile << " in if "<< tempIt->second << " " << it->second << " ";
			}
			tempIt++;
		}


		//myfile << it->first <<" " << it->second << "\n";
		it++;
	}

	int nrOfPeriods = replayLength/1000;
	if(nrOfPeriods>25)
	{
		nrOfPeriods = 25;
	}
	for(int timePeriod = 1; timePeriod <= nrOfPeriods; timePeriod++)
	{
		myfile << "period";
		if (timePeriod <= 9) myfile << "0";
		myfile <<timePeriod << ",";

		for(int i = 1; i < temp.size(); i++)
		{	
			if(temp.at(i)>0&&(temp.at(i)/1000<timePeriod||timePeriod==nrOfPeriods))
			{
				myfile << 1;
			}else
			{
				myfile << 0;
			}
			if(i<temp.size()-1)
			{
				myfile << ",";
			}

		}
		myfile << "\n";
	}
	myfile.close();
}


void ReplayModule::onUnitDestroy(BWAPI::Unit * unit)
{
	//Not needed yet
}

void ReplayModule::onUnitMorph(BWAPI::Unit * unit)
{	
	
	if(unit->getType().isBuilding())
	{
		morphingBuildings.push_front(unit);

	}else
	{
		//Broodwar->printf("%s was morphed at time (%d)", unit->getType().c_str(), Broodwar->getFrameCount()); //Used for debugging
		if(unit->getType().getRace()==Races::Zerg)
		{
			if(zergUnits.count(unit->getType().c_str())==0)
			{	
				const char* temp = unit->getType().c_str() + 5;
				zergUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
			}
		}else if(unit->getType().getRace()==Races::Protoss&&getEnemy()->getRace() == BWAPI::Races::Protoss)
		{	
			const char* temp = unit->getType().c_str() + 8;
			if(unit->getPlayer() == getPlayer())
			{
				protossUnitsp1.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
			}else
			{
				protossUnitsp2.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
			}
		}	
	}
}

void ReplayModule::onUnitRenegade(BWAPI::Unit * unit)
{
	morphingBuildings.push_front(unit);
}

void ReplayModule::onUnitCreate(BWAPI::Unit * unit)
{
	//Not needed yet
}

void ReplayModule::onUnitComplete(BWAPI::Unit * unit)
{	

	Player* enemy = getEnemy();
	//Broodwar->printf("%s was created at time (%d)", unit->getType().c_str(), unit->getType().getID()); 
	if(unit->getType().getRace()==Races::Protoss&&enemy->getRace() == BWAPI::Races::Protoss)
	{	
		const char* temp = unit->getType().c_str() + 8;
		if(unit->getPlayer() == getPlayer())
		{
			protossUnitsp1.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
		}else
		{
			protossUnitsp2.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
		}
	}else if (unit->getType().getRace()==Races::Terran)
	{	
		const char* temp = unit->getType().c_str() + 7;

		terranUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
	}
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