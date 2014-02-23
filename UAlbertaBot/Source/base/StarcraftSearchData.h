#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\ActionSet.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\DFBBStarcraftSearch.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftState.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftSearchGoal.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\SmartStarcraftSearch.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\SearchSaveState.hpp"

class StarcraftSearchData
{
	// parameters being used for this search
	BuildOrderSearch::SearchParameters			previousParameters;

	// whether a search is currently in progress
	bool searchInProgress;

	// the search results from the previous search attempt
	// if this is solved, we can return the build order
	BuildOrderSearch::SearchResults previousSearchResults;
	BuildOrderSearch::SearchResults lastSolvedResults;

	// data for statistics
	int lastSearchFrame;
	int searchFrames;
	int totalNodes;
	double branch;
	double totalSearchTime;

	std::vector<std::pair<double,bool>>		searchHistory;

public:

	StarcraftSearchData() 
		: searchInProgress(false)
		, lastSearchFrame(0)
		, searchFrames(0)
		, totalNodes(0)
		, branch(0)
		, totalSearchTime(0) {}

	~StarcraftSearchData() {}

	void update(double timeLimit)
	{
		// if a search is in progress 
		if (searchInProgress)
		{
			//BWAPI::Broodwar->printf("Searching with %lf ms left", timeLimit);
			lastSearchFrame = BWAPI::Broodwar->getFrameCount();

			// set the time limit based on how much time we have this frame
			previousParameters.searchTimeLimit = (int)(timeLimit > 3 ? timeLimit : 3);//(int)timeLimit;

			// construct the new search object
			BuildOrderSearch::DFBBStarcraftSearch SCSearch(previousParameters);

			// set the results based on the
			previousSearchResults = SCSearch.search();

			// if there is a solution in these results, store it
			if (previousSearchResults.solved)
			{
				lastSolvedResults = previousSearchResults;
			}

			// if we didn't time out, then we're finished the search
			if (!previousSearchResults.timedOut || searchFrames > 500)
			{
				searchInProgress = false;
			}
			// if we did time out, set the save state
			else
			{
				previousParameters.useSaveState = true;
				previousParameters.saveState = previousSearchResults.saveState;
			}

			// keep statistics
			lastSearchFrame = BWAPI::Broodwar->getFrameCount();
			searchFrames++;
			totalNodes += (int)previousSearchResults.nodesExpanded;
			branch += previousSearchResults.avgBranch;
			totalSearchTime += previousSearchResults.timeElapsed;

			//BWAPI::Broodwar->printf("Search took %lf ms, and %s", previousSearchResults.timeElapsed, previousSearchResults.solved ? "solved" : "not solved");
		}

	}

	void drawSearchResults(int x, int y)
	{
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
		{
			BWAPI::Broodwar->drawBoxScreen(x-5, y-15, x+125, y+55, BWAPI::Colors::Black, true);

			BWAPI::Broodwar->drawTextScreen(x, y-13,	"\x07Search Information");
				
			BWAPI::Broodwar->drawTextScreen(x, y,		"\x04 Total Time");
			BWAPI::Broodwar->drawTextScreen(x+75, y,	"%.3lf ms", totalSearchTime);

			BWAPI::Broodwar->drawTextScreen(x, y+10,	"\x04 Nodes");
			BWAPI::Broodwar->drawTextScreen(x+75, y+10,	"%d", totalNodes);

			BWAPI::Broodwar->drawTextScreen(x, y+20,	"\x04 Branch");
			BWAPI::Broodwar->drawTextScreen(x+75, y+20,	"%.3lf", searchFrames > 0 ? (branch/searchFrames) : 0);
			
			BWAPI::Broodwar->drawTextScreen(x, y+30,	"\x04 Frames");
			BWAPI::Broodwar->drawTextScreen(x+75, y+30,	"%d", searchFrames);

			BWAPI::Broodwar->drawTextScreen(x, y+40,	"\x04 End Frame");
			BWAPI::Broodwar->drawTextScreen(x+75, y+40,	"%d", lastSearchFrame);
		}
	}

	// get the build order which has been calculated
	// this build order is stored in previousSearchResults
	BuildOrderSearch::SearchResults getResults()
	{
		// if the search is still ongoing, return blank
		if (searchInProgress)
		{
			return BuildOrderSearch::SearchResults();
		}

		// when it's done return the last solution, which will be the best
		return lastSolvedResults;
	}

	void startNewSearch(BuildOrderSearch::SearchParameters params, bool forceNewSearch = false)
	{
		// if this is a new goal, start a new search
		if (forceNewSearch || !(previousParameters.goal == params.goal))
		{
			//BWAPI::Broodwar->printf("New goal! Searching...");

			searchFrames = 0;
			totalSearchTime = 0;
			totalNodes = 0;
			branch = 0;

			// set the class parameters
			previousParameters = params;

			// re-set the search results
			lastSolvedResults = BuildOrderSearch::SearchResults();
			previousSearchResults = BuildOrderSearch::SearchResults();
			previousSearchResults.timedOut = true;
			
			// start search on next frame
			searchInProgress = true;

			searchHistory.clear();
		}
		// otherwise don't do anything
		else
		{
			
		}
	}
};
