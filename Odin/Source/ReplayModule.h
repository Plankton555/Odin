#pragma once

#include "Common.h"
#include <iostream>
#include <fstream>
#include <OdinUtils.h>
#include <boost/algorithm/string.hpp>

#define MARGIN_OF_ERROR (0.25) // if diff between prediction and truth is lower than this -> good prediction
typedef std::pair<int, int> IntPair;

class ReplayModule
{
public:
	ReplayModule();
	~ReplayModule();
	void onStart();
	void onFrame();
	void onEnd(std::string filename, bool isWinner);
	void onUnitDestroy(BWAPI::Unit * unit);
	void onUnitMorph(BWAPI::Unit * unit);
	void onUnitRenegade(BWAPI::Unit * unit);
	void onUnitCreate(BWAPI::Unit * unit);
	void onUnitComplete(BWAPI::Unit * unit);

	static BWAPI::Player* getEnemy(void);
	static BWAPI::Player* getPlayer(void);

	int gameID;

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

	void analyseResults(BWAPI::Race race, const char* gameFile, const char* replayFile);
	void storeResult(std::string filename, std::vector<IntPair> result);
	const char* getRaceResultFile(BWAPI::Race race);

};