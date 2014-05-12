#include "ReplayModule.h"
#include "FuzzyModule.h"
#include <iostream>
#include <fstream>
#include <boost\filesystem.hpp>
#include "Common.h"

using namespace BWAPI;
using namespace std;

const std::string REPLAY_DATA_PATH = ODIN_DATA_FILEPATH + "replays/";
const std::string SEEN_REPLAYS_PATH = REPLAY_DATA_PATH + "seen_replays.txt";

const char* RACE_RESULT_FILE_BASE = "bwapi-data/Odin/odin_data/BNlog/";

Player* ReplayModule::player = NULL;
Player* ReplayModule::enemy = NULL;

ReplayModule::ReplayModule()  { ReplayModule::analyzePlayers(); }
ReplayModule::~ReplayModule() { }


void ReplayModule::createMaps()
{	
	//Buildings 
	protossUnitsAll["Assimilator"] = 1;
	protossUnitsAll["Pylon"] = 2;
	protossUnitsAll["Gateway"] = 3;
	protossUnitsAll["Zealot"] = 4;
	protossUnitsAll["Cybernetics Core"] = 5;
	protossUnitsAll["Citadel of Adun"] = 6;
	protossUnitsAll["Templar Archives"] = 7;
	protossUnitsAll["Dark Templar"] = 8;
	protossUnitsAll["Robotics Facility"] = 9;
	protossUnitsAll["Robotics Support Bay"] = 10;
	protossUnitsAll["Forge"] = 11;
	protossUnitsAll["Photon Cannon"] = 12;
	protossUnitsAll["Stargate"] = 13;
	protossUnitsAll["Observatory"] = 14;
	protossUnitsAll["Arbiter Tribunal"] = 15;
	protossUnitsAll["Fleet Beacon"] = 16;
	protossUnitsAll["Shield Battery"] = 17;
	protossUnitsAll["Dragoon"] = 18;
	protossUnitsAll["High Templar"] = 19;
	protossUnitsAll["Shuttle"] = 20;
	protossUnitsAll["Reaver"] = 21;
	protossUnitsAll["Observer"] = 22;
	protossUnitsAll["Carrier"] = 23;
	protossUnitsAll["Scout"] = 24;
	protossUnitsAll["Corsair"] = 25;
	protossUnitsAll["Arbiter"] = 26;
	protossUnitsAll["Dark Archon"] = 27;
	protossUnitsAll["Archon"] = 28;

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
	

	terranUnitsAll["Barracks"] = 1;
	terranUnitsAll["Bunker"] = 2;
	terranUnitsAll["Supply Depot"] = 3;
	terranUnitsAll["Engineering Bay"] = 4;
	terranUnitsAll["Missile Turret"] = 5;
	terranUnitsAll["Refinery"] = 6;	
	terranUnitsAll["Academy"] = 7;
	terranUnitsAll["Factory"] = 8;
	terranUnitsAll["Comsat Station"] = 9;
	terranUnitsAll["Machine Shop"] = 10;
	terranUnitsAll["Armory"] = 11;
	terranUnitsAll["Starport"] = 12;
	terranUnitsAll["Control Tower"] = 13;
	terranUnitsAll["Science Facility"] = 14;
	terranUnitsAll["Physics Lab"] = 15;
	terranUnitsAll["Covert Ops"] = 16;
	terranUnitsAll["Nuclear Silo"] = 17;
	
	terranUnitsAll["Marine"] = 18;
	terranUnitsAll["Firebat"] = 19;
	terranUnitsAll["Medic"] = 20;
	terranUnitsAll["Vulture"] = 21;
	terranUnitsAll["Vulture Spider Mine"] = 22;
	terranUnitsAll["Siege Tank"] = 23;
	terranUnitsAll["Goliath"] = 24;
	terranUnitsAll["Valkyrie"] = 25;
	terranUnitsAll["Dropship"] = 26;
	terranUnitsAll["Wraith"] = 27;
	terranUnitsAll["Science Vessel"] = 28;
	terranUnitsAll["Battlecruiser"] = 29;
	terranUnitsAll["Ghost"] = 30;
	terranUnitsAll["Nuclear Missile"] = 31;

	terranTechAll["Tank_Siege_Mode"] = 32;
	terranTechAll["Cloaking_Field"] = 33;
	terranTechAll["Yamato_Gun"] = 34;
	terranTechAll["EMP_Shockwave"] = 35;
	terranTechAll["Irradiate"] = 36;
	terranTechAll["Personnel_Cloaking"] = 37;
	terranTechAll["Lockdown"] = 38;
	terranTechAll["Stim_Packs"] = 39;
	terranTechAll["Restoration"] = 40;
	terranTechAll["Optical_Flare"] = 41;

	terranUpgradesAll["Terran_Infantry_Armor"] = 42;
	terranUpgradesAll["Terran_Infantry_Weapons"] = 43;
	terranUpgradesAll["Terran_Ship_Plating"] = 44;
	terranUpgradesAll["Terran_Ship_Weapons"] = 45;
	terranUpgradesAll["Terran_Vehicle_Plating"] = 46;
	terranUpgradesAll["Terran_Vehicle_Weapons"] = 47;
	terranUpgradesAll["Ion_Thrusters"] = 48;
	terranUpgradesAll["Charon_Boosters"] = 49;
	terranUpgradesAll["Apollo_Reactor"] = 50;
	terranUpgradesAll["Colossus_Reactor"] = 51;
	terranUpgradesAll["Titan_Reactor"] = 52;
	terranUpgradesAll["Moebius_Reactor"] = 53;
	terranUpgradesAll["Ocular_Implants"] = 54;
	terranUpgradesAll["U_238_Shells"] = 55;
	terranUpgradesAll["Caduceus_Reactor"] = 56;


	zergUnitsAll["Spawning Pool"] = 1;
	zergUnitsAll["Zergling"] = 2;
	zergUnitsAll["Evolution Chamber"] = 3;
	zergUnitsAll["Creep Colony"] = 4;
	zergUnitsAll["Spore Colony"] = 5;
	zergUnitsAll["Sunken Colony"] = 6;
	zergUnitsAll["Extractor"] = 7;
	zergUnitsAll["Hydralisk Den"] = 8;
	zergUnitsAll["Lair"] = 9;
	zergUnitsAll["Queens Nest"] = 10;
	zergUnitsAll["Hive"] = 11;
	zergUnitsAll["Defiler Mound"] = 12;
	zergUnitsAll["Nydus Canal"] = 13;
	zergUnitsAll["Ultralisk Cavern"] = 14;
	zergUnitsAll["Spire"] = 15;
	zergUnitsAll["Greater Spire"] = 16;

	zergUnitsAll["Hydralisk"] = 17;
	zergUnitsAll["Lurker"] = 18;
	zergUnitsAll["Queen"] = 19;
	zergUnitsAll["Defiler"] = 20;
	zergUnitsAll["Mutalisk"] = 21;
	zergUnitsAll["Scourge"] = 22;
	zergUnitsAll["Guardian"] = 23;
	zergUnitsAll["Devourer"] = 24;
	zergUnitsAll["Ultralisk"] = 25;

	zergUnitsAll["Overlord"] = 26;

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
	ifstream repfile (SEEN_REPLAYS_PATH.c_str());
	if (repfile.is_open()) {
		while (getline(repfile,line)) {
			if (line.compare(Broodwar->mapFileName()) == 0) {
				Broodwar->printf("This replay has already been seen.");
				gameSeen = true;
				Broodwar->leaveGame();
				return;
			}
		}
		repfile.close();
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

void ReplayModule::analyseResults(int timePeriodAhead, BWAPI::Race race)
{
	std::string gameFile = odin_utils::getOutputFile(OdinUtils::Instance().gameID, timePeriodAhead);
	std::string replayFile = odin_utils::getBNOutputFile(OdinUtils::Instance().gameID);

	// analyse this games/replays result
	ifstream gameBN (gameFile.c_str());
	ifstream replayBN (replayFile.c_str());
	if (gameBN.is_open() && replayBN.is_open()) {
		std::vector<IntPair> result;
		std::string gameLine, replayLine;
		bool gameLineOK = getline(gameBN, gameLine);
		while (gameLineOK && getline(replayBN, replayLine)) {
			std::vector<std::string> gameValues, replayValues;
			boost::split(gameValues, gameLine, boost::is_any_of(",d"));
			boost::split(replayValues, replayLine, boost::is_any_of(",d"));

			int gamePeriod = atoi(gameValues[1].c_str());
			int replayPeriod = atoi(replayValues[1].c_str());
			if (gamePeriod > replayPeriod) 
			{
				result.push_back(std::make_pair(0, 0));
				continue;
			}

			int gameSize = gameValues.size();
			int replaySize = replayValues.size(); 
			int correctPredictions = 0;
			for (int i = 2; i < gameSize && i < replaySize; i++)
			{
				double diff = abs(atof(gameValues[i].c_str()) - atof(replayValues[i].c_str()));
				if (diff < MARGIN_OF_ERROR)
				{
					correctPredictions++;
				}
				else
				{
					//print to file what was wrong!!
					BWAPI::Race race = getEnemy()->getRace();//std::map<const char*,int>
					std::map<const char*,int> units;
					if (race == BWAPI::Races::Protoss) { units = protossUnitsAll; }
					if (race == BWAPI::Races::Terran) { units = terranUnitsAll; }
					if (race == BWAPI::Races::Zerg) { units = zergUnitsAll; }
					std::map<const char*, int>::iterator it;
					int index = 0;
					for(it = units.begin(); index < i-1 && it != units.end(); index++, it++) { }
					if (it != units.end())
					{
						std::ostringstream out;
						out << it->first << ", ";
						out << atof(gameValues[i].c_str()) << " vs " << atof(replayValues[i].c_str());
						odin_utils::debug(out.str());
					}


				}
			}

			int totalPredictions = std::min(gameSize, replaySize) - 2;
			result.push_back(std::make_pair(correctPredictions, totalPredictions));
			gameLineOK = getline(gameBN, gameLine);
		}

		//write to file
		std::ostringstream resultFilename;
		resultFilename << "bwapi-data/Odin/odin_data/BNlog/" << timePeriodAhead << "/" << OdinUtils::Instance().gameID << ".txt";
		storeResult(resultFilename.str(), result);
		
		//sum up all results EVER into results-vector
		std::string raceResultFile = getRaceResultFile(race, timePeriodAhead);
		std::ifstream allResults (raceResultFile.c_str());
		if (allResults.is_open())
		{
			std::string line;
			int i = 0;
			while (getline(allResults, line))
			{
				std::vector<std::string> buff;
				boost::split(buff, line, boost::is_any_of(","));
				if (i < result.size())
				{
					result[i].first += atoi(buff[0].c_str());
					result[i].second += atoi(buff[1].c_str());
				} 
				else
				{
					int first = atoi(buff[0].c_str());
					int second = atoi(buff[1].c_str());
					result.push_back(std::make_pair(first, second));
				}
				i++;
			}

			allResults.close();
		}

		//write summed up results to file
		storeResult(raceResultFile, result);

		gameBN.close();
		replayBN.close();
	}

}

void ReplayModule::storeResult(std::string filename, std::vector<IntPair> result)
{
	std::ofstream resultFile (filename.c_str());
	if (resultFile.is_open())
	{
		for (int i = 0; i < result.size(); i++)
		{
			std::ostringstream resultStream;
			resultStream << result[i].first << "," << result[i].second << "\n";
			resultFile << resultStream.str();
		}
		resultFile.close();
	}
}

std::string ReplayModule::getRaceResultFile(BWAPI::Race race, int timePeriodAhead)
{
	std::ostringstream resultStream;
	resultStream << RACE_RESULT_FILE_BASE << race.getName() << timePeriodAhead << ".txt";
	return resultStream.str();
}

void ReplayModule::onEnd(std::string BNfilename, bool isWinner)
{

	//Replay has ended. Save data to database here
	if(!gameSeen)
	{
		std::string replayBN = odin_utils::getBNOutputFile(OdinUtils::Instance().gameID);

		if(!zergUnits.empty())
		{
			writeToFile(getReplayFileSpecificForInstance(BWAPI::Races::Zerg), zergUnits, zergUnitsAll);
			writeToFile(replayBN.c_str(), zergUnits, zergUnitsAll);
		}
	
		if(!protossUnitsp1.empty())
		{
			// If PvP, protossUnitsp1 stores the units for player 1
			writeToFile(getReplayFileSpecificForInstance(BWAPI::Races::Protoss), protossUnitsp1, protossUnitsAll);
			//writeToFile(BNfilename.c_str(), protossUnitsp1, protossUnitsAll); //not for our bot, only for his opponent
		}

		if(!protossUnitsp2.empty())
		{
			// If PvP, protossUnitsp2 stores the units for player 2
			writeToFile(getReplayFileSpecificForInstance(BWAPI::Races::Protoss), protossUnitsp2, protossUnitsAll);
			writeToFile(replayBN.c_str(), protossUnitsp2, protossUnitsAll);
		}
	
		if(!terranUnits.empty())
		{
			writeToFile(getReplayFileSpecificForInstance(BWAPI::Races::Terran), terranUnits, terranUnitsAll);
			writeToFile(replayBN.c_str(), terranUnits, terranUnitsAll);
		}

		// analyse results for all predictions
		for (int timePeriodsAhead = 0; timePeriodsAhead <= OdinUtils::Instance().predictTimePeriodsAhead; timePeriodsAhead++)
		{
			analyseResults(timePeriodsAhead, getEnemy()->getRace());
		}
		
	}

	//Count seen replays
	string line;
	int nrFiles = 0;
	ifstream seenrepfile (SEEN_REPLAYS_PATH.c_str());
	if (seenrepfile.is_open()) {
		while (getline(seenrepfile,line)) {
			nrFiles++;
		}
		seenrepfile.close();
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

std::string ReplayModule::getReplayFileSpecificForInstance(BWAPI::Race race)
{
	std::ostringstream filename;
	filename << REPLAY_DATA_PATH << race.getName().c_str()[0] << BWAPI::Broodwar->getInstanceNumber() <<".txt";
	return filename.str();
}

void ReplayModule::writeToFile(std::string file, std::map<const char*,int> stuffToWrite, std::map<const char*,int> unitList)
{
	std::vector<int> temp(unitList.size()+1, 0);
	std::map<const char*,int>::iterator it;
	for(it=stuffToWrite.begin(); it!=stuffToWrite.end();)
	{	
		std::string stuffName = it->first;
		odin_utils::replaceAllString(stuffName, " Siege Mode", ""); //Special case for siege tanks
		odin_utils::replaceAllString(stuffName, " Tank Mode", "");

		std::map<const char*,int>::iterator tempIt;
		for(tempIt=unitList.begin(); tempIt!=unitList.end();)
		{
			if(strcmp(tempIt->first, stuffName.c_str())==0)
			{
				if (temp.at(tempIt->second) == 0)	//Just set the value if it hasn't been set already
				{									//(Special case with siege tanks)
					temp.at(tempIt->second) = it->second;
				}
				
				break;
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

	//std::ofstream myfile (file, ios::app);
	//if (!myfile.is_open()) { odin_utils::debug("öhh, filen är ju inte ens öppen!"); odin_utils::debug(file); }

	std::string filename = file; //"bwapi-data/Odin/odin_data/replays/P/test.txt";
	for(int timePeriod = 1; timePeriod <= nrOfPeriods; timePeriod++)
	{
		odin_utils::logBN(filename, "period");
		odin_utils::logBN(filename, timePeriod, true);
		odin_utils::logBN(filename, ",");


		//myfile << "period";
		//if (timePeriod <= 9) myfile << "0";
		//myfile <<timePeriod << ",";

		for(int i = 1; i < temp.size(); i++)
		{	
			bool present = temp.at(i)>0&&(temp.at(i)/1000<timePeriod||timePeriod==nrOfPeriods);
			odin_utils::logBN(filename, present ? 1 : 0);
			if (i < temp.size()-1)
			{
				odin_utils::logBN(filename, ",");
			}

			//if(temp.at(i)>0&&(temp.at(i)/1000<timePeriod||timePeriod==nrOfPeriods))
			//{
			//	myfile << 1;
			//}else
			//{
			//	myfile << 0;
			//}
			//if(i<temp.size()-1)
			//{
			//	myfile << ",";
			//}

		}
		odin_utils::logBN(filename, "\n");
		//myfile << "\n";
	}
	//myfile.close();
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