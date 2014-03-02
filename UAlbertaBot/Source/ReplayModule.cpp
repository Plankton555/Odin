#include "ReplayModule.h"

using namespace BWAPI;

ReplayModule::ReplayModule()  {}
ReplayModule::~ReplayModule() {}



void ReplayModule::createMaps()
{	
	//Buildings 
	protossUnitsAll["Pylon"] = 1;
	protossUnitsAll["Gateway"] = 2;
	protossUnitsAll["Assimilatior"] = 3;
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

}

void ReplayModule::onStart()
{
	Broodwar->printf("Hello, my name is Odin! And this is a replay");
	ReplayModule::createMaps();
}

void ReplayModule::onFrame()
{
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
					zergUnits.insert(std::map<const char*,int>::value_type ((*it)->getType().c_str(),Broodwar->getFrameCount()));
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
		writeToFile("replaydatastuff/zerg.txt", zergUnits);
	}
	
	if(!protossUnits.empty())
	{
		writeToFile("replaydatastuff/protoss.txt", protossUnits);
	}
	
	if(!terranUnits.empty())
	{
		writeToFile("replaydatastuff/terran.txt", terranUnits);
	}
	

}

void ReplayModule::writeToFile(char* file, std::map<const char*,int> stuffToWrite)
{
	myfile.open (file, std::ios::app);
	
	
	std::vector<int> temp(protossUnitsAll.size()+1);
	std::map<const char*,int>::iterator it;
	for(it=stuffToWrite.begin(); it!=stuffToWrite.end();)
	{				 
		
		std::map<const char*,int>::iterator tempIt;
		for(tempIt=protossUnitsAll.begin(); tempIt!=protossUnitsAll.end();)
		{
			if(strcmp(tempIt->first ,it->first)==0)
			{
				temp.at(tempIt->second) = it->second;
				//myfile << " in if "<< tempIt->second << " " << it->second << " ";
			}
			tempIt++;
		}
		
		
		myfile << it->first <<" " << it->second << "\n";
		it++;
	}

	for(it=protossUnitsAll.begin(); it!=protossUnitsAll.end();)
	{
		myfile << it->first <<" ";
		it++;
	}
	myfile << "\n";
	for(int timePeriod = 1; timePeriod <= 15; timePeriod++)
	{
		myfile << timePeriod << "  ";
		
		for(int i = 1; i < temp.size(); i++)
		{		
			if(temp.at(i)>0&&(temp.at(i)/1000<timePeriod||timePeriod==15))
			{
				myfile << 1 << " ";
			}else
			{
				myfile << 0 << " ";
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
				zergUnits.insert(std::map<const char*,int>::value_type (unit->getType().c_str(),Broodwar->getFrameCount()));
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
	//Not needed yet
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
		terranUnits.insert(std::map<const char*,int>::value_type (unit->getType().c_str(),Broodwar->getFrameCount()));
	}
}

