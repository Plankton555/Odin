#include "ReplayModule.h"

using namespace BWAPI;

ReplayModule::ReplayModule()  {}
ReplayModule::~ReplayModule() {}

void ReplayModule::onStart()
{
	Broodwar->printf("Hello, my name is Odin! And this is a replay");
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
	std::map<const char*,int>::iterator it;
	for(it=stuffToWrite.begin(); it!=stuffToWrite.end();)
	{				 
		int timePeriod = it->second+1000;
		myfile << it->first <<" " << it->second << "\n";
		it++;
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
			protossUnits.insert(std::map<const char*,int>::value_type (unit->getType().c_str(),Broodwar->getFrameCount()));
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
	//Broodwar->printf("%s was created at time (%d)", unit->getType().c_str(), Broodwar->getFrameCount()); 
	if(unit->getType().getRace()==Races::Protoss)
	{
		protossUnits.insert(std::map<const char*,int>::value_type (unit->getType().c_str(),Broodwar->getFrameCount()));
	}else if (unit->getType().getRace()==Races::Terran)
	{
		terranUnits.insert(std::map<const char*,int>::value_type (unit->getType().c_str(),Broodwar->getFrameCount()));
	}
}

