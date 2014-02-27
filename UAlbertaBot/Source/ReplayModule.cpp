#include "ReplayModule.h"

using namespace BWAPI;

ReplayModule::ReplayModule()  {}
ReplayModule::~ReplayModule() {}

void ReplayModule::onStart()
{
	Broodwar->printf("Hello, my name is Odin! This is a replay.");
	Broodwar->printf("Map: %s \n", BWAPI::Broodwar->mapFileName().c_str());
	Broodwar->printf("Map: %s \n", BWAPI::Broodwar->mapPathName().c_str());
}

void ReplayModule::onFrame()
{
}