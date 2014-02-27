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
	{	if((*it)->isCompleted()){
			Broodwar->printf("%s was morphed at time (%d)", (*it)->getType().c_str(), Broodwar->getFrameCount()); 
			it = morphingBuildings.erase(it);
		}else{
			 it++;
		}
	}
}

void ReplayModule::onEnd(bool isWinner)
{
	//Replay ahs ended. Save data to database here
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

	}else{
		Broodwar->printf("%s was morphed at time (%d)", unit->getType().c_str(), Broodwar->getFrameCount()); 
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
	Broodwar->printf("%s was created at time (%d)", unit->getType().c_str(), Broodwar->getFrameCount()); 
}

