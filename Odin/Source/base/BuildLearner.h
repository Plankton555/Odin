#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"
#include "../StrategyManager.h"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\ActionSet.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\DFBBStarcraftSearch.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftState.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftSearchGoal.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\SmartStarcraftSearch.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\SearchSaveState.hpp"

#include "StarcraftSearchData.h"

class BuildLearner
{
	std::vector<MetaType>				actionsTaken;

	BuildOrderSearch::StarcraftState	getCurrentState();
	std::vector<MetaType>				getMetaVector(const BuildOrderSearch::SearchResults & results);
	BuildOrderSearch::Action			getAction(MetaType t);
	MetaType							getMetaType(BuildOrderSearch::Action a);
	bool								isLegalAction(BWAPI::UnitType t);
	int									getUnitTypeCount(BWAPI::UnitType type);

public:

	BuildLearner();

	void					addAction(const MetaType & type);
	void					update();
	void					onGameEnd();
	MetaType				getRandomLegalAction();
	
};