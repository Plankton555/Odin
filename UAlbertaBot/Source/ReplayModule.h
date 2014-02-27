#pragma once

#include "Common.h"
#include <iostream>
#include <fstream>

class ReplayModule
{
public:
	ReplayModule();
	~ReplayModule();
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit * unit);
	void onUnitMorph(BWAPI::Unit * unit);
	void onUnitRenegade(BWAPI::Unit * unit);
	void onUnitCreate(BWAPI::Unit * unit);
	void onUnitComplete(BWAPI::Unit * unit);

private:
	std::list<BWAPI::Unit*> morphingBuildings;  
	std::map<const char*,int> zergUnits;
	std::map<const char*,int> terranUnits;
	std::map<const char*,int> protossUnits;
	std::ofstream myfile;

	void writeToFile(char* file, std::map<const char*,int> stuffToWrite);

};