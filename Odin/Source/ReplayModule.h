#pragma once

#include "Common.h"
#include <iostream>
#include <fstream>
#include <OdinUtils.h>

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

	static BWAPI::Player* getEnemy(void);
	static BWAPI::Player* getPlayer(void);

private:
	std::list<BWAPI::Unit*> morphingBuildings;
	std::map<const char*,int> zergUnits;
	std::map<const char*,int> terranUnits;
	std::map<const char*,int> protossUnitsp1;
	std::map<const char*,int> protossUnitsp2;
	std::map<const char*,int> zergUnitsAll;
	std::map<const char*,int> terranUnitsAll;
	std::map<const char*,int> protossUnitsAll;
	std::map<const char*,int> zergTechAll;
	std::map<const char*,int> zergUpgradesAll;
	std::map<const char*,int> terranTechAll;
	std::map<const char*,int> terranUpgradesAll;
	std::map<const char*,int> protossTechAll;
	std::map<const char*,int> protossUpgradesAll;
	std::ofstream myfile;
	bool gameSeen;
	int replayLength;

	void writeToFile(const char* file, std::map<const char*,int> stuffToWrite, std::map<const char*,int> unitList);
	void createMaps();

	static BWAPI::Player* ReplayModule::enemy;
	static BWAPI::Player* ReplayModule::player;

	static void analyzePlayers(void);
	void drawUnitInformation(int x, int y);

};