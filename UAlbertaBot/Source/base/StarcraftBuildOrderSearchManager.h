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

class StarcraftBuildOrderSearchManager
{
	// starcraftSearchData is hard coded to be protoss state for now
	StarcraftSearchData			starcraftSearchData;

	int lastSearchFinishTime;

	BuildOrderSearch::StarcraftState				getCurrentState();

	// gets a sample starting state for a race
	BuildOrderSearch::StarcraftState				getStartState();
	
	// starts a search
	BuildOrderSearch::SearchResults					search(const std::vector< std::pair<MetaType, UnitCountType> > & goalUnits);

	BuildOrderSearch::SearchResults					previousResults;

	std::vector<MetaType>							getMetaVector(const BuildOrderSearch::SearchResults & results);

	BuildOrderSearch::Action						getAction(MetaType t);

	BuildOrderSearch::StarcraftSearchGoal			getGoal(const std::vector< std::pair<MetaType, UnitCountType> > & goalUnits);

	void											setRepetitions(BuildOrderSearch::StarcraftSearchGoal goal);

	void						loadOpeningBook();
	std::vector<std::vector<MetaType>> openingBook;
	std::vector<MetaType>		getMetaVector(std::string buildString);
	MetaType					getMetaType(BuildOrderSearch::Action a);
	
	StarcraftBuildOrderSearchManager();

public:

	static StarcraftBuildOrderSearchManager &	Instance();

	void						update(double timeLimit);

	void						reset();

	void						setActionGoal(BuildOrderSearch::Action a, int count);
	void						setActionK(BuildOrderSearch::Action a, int k);
	void						drawSearchInformation(int x, int y);

	std::vector<MetaType>		getOpeningBuildOrder();
	
	std::vector<MetaType>		findBuildOrder(const std::vector< std::pair<MetaType, UnitCountType> > & goalUnits);
};