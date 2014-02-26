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
}