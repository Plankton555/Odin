#include "ReplayModule.h"

using namespace BWAPI;

Player* ReplayModule::player = NULL;
Player* ReplayModule::enemy = NULL;

ReplayModule::ReplayModule()  { ReplayModule::analyzePlayers(); }
ReplayModule::~ReplayModule() {}



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
	zergUnitsAll["Devour"] = 26;

}

void ReplayModule::onStart()
{
	Broodwar->printf("Hello, my name is Odin! And this is a replay");
	ReplayModule::createMaps();
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
				}else if((*it)->getType().getRace()==Races::Protoss)
				{	
					const char* temp = (*it)->getType().c_str() + 8;
					protossUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
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
	
	if(!zergUnits.empty())
	{
		writeToFile("replaydatastuff/zerg.txt", zergUnits, zergUnitsAll);
	}
	
	if(!protossUnits.empty())
	{
		writeToFile("replaydatastuff/protoss.txt", protossUnits, protossUnitsAll);
	}
	
	if(!terranUnits.empty())
	{
		writeToFile("replaydatastuff/terran.txt", terranUnits, terranUnitsAll);
	}
	

}

void ReplayModule::writeToFile(char* file, std::map<const char*,int> stuffToWrite, std::map<const char*,int> unitList)
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
	for(it=unitList.begin(); it!=unitList.end();)
	{
		myfile << it->first <<",";
		it++;
	}
	myfile << "\n";
	for(int timePeriod = 1; timePeriod <= 15; timePeriod++)
	{
		myfile << "period" <<timePeriod << ",";
		
		for(int i = 1; i < temp.size(); i++)
		{		
			if(temp.at(i)>0&&(temp.at(i)/1000<timePeriod||timePeriod==15))
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
		}else if(unit->getType().getRace()==Races::Protoss)
		{	
			const char* temp = unit->getType().c_str() + 8;
			protossUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
		}	
	}
}

void ReplayModule::onUnitRenegade(BWAPI::Unit * unit)
{
	morphingBuildings.push_front(unit);
}
void ReplayModule::onUnitCreate(BWAPI::Unit * unit)
{
	
}

void ReplayModule::onUnitComplete(BWAPI::Unit * unit)
{
	//Broodwar->printf("%s was created at time (%d)", unit->getType().c_str(), unit->getType().getID()); 
	if(unit->getType().getRace()==Races::Protoss)
	{	
		const char* temp = unit->getType().c_str() + 8;

		protossUnits.insert(std::map<const char*,int>::value_type (temp,Broodwar->getFrameCount()));
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
