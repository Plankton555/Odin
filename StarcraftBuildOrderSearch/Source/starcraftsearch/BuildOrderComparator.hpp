#pragma once

#include <sstream>

#include "StarcraftData.hpp"
#include "TerranState.hpp"
#include "DFBBStarcraftSearch.hpp"
#include "StarcraftSearchConstraint.hpp"
#include "SmartStarcraftSearch.hpp"

class BuildOrderComparator
{

	int buildOrderNum;

public:

	BuildOrderComparator() : buildOrderNum(0) {}

	void printBuildOrder(const std::vector<Action> & buildOrder) const
	{
		fprintf(stdout, "%d : ", buildOrderNum);
		for (size_t a(0); a<buildOrder.size(); ++a)
		{
			fprintf(stdout, "%d ", buildOrder[a]);
		}	fprintf(stdout, "\n");
		
		fprintf(stderr, "%d : ", buildOrderNum);
		for (size_t a(0); a<buildOrder.size(); ++a)
		{
			fprintf(stderr, "%d ", buildOrder[a]);
		}	fprintf(stderr, "\n");
		
		//printf("\nPro Build takes %d frames\n", getCompletionTime(buildOrder));
	}
	
	int getCompletionTime(const std::vector<Action> & buildOrder) const
	{
		ProtossState s(true);
	
		for (size_t a(0); a<buildOrder.size(); ++a)
		{
			s.doAction(buildOrder[a], s.resourcesReady(buildOrder[a]));
		}
		
		return s.getLastFinishTime();
	}
	
	SearchResults doOptimalSearch(const ProtossState initialState, const std::vector<Action> & buildOrder, const int timeLimit, int upperBound = 0) const
	{
		SearchParameters<ProtossState> params(getGoalFromBuildOrder(initialState, buildOrder), initialState, StarcraftSearchConstraints());
		params.searchTimeLimit = timeLimit;
		params.initialUpperBound = upperBound;
		
		DFBBStarcraftSearch<ProtossState> SCSearch(params);
		
		return SCSearch.search();;
	}
	
	SearchResults doOptimalSearch(ProtossState initialState, StarcraftSearchGoal goal, int timeLimit, int upperBound = 0)
	{
		SearchParameters<ProtossState> params(goal, initialState, StarcraftSearchConstraints());
		params.searchTimeLimit = timeLimit;
		params.initialUpperBound = upperBound;

		DFBBStarcraftSearch<ProtossState> SCSearch(params);

		return SCSearch.search();;
	}
	
	SearchResults doSubOptimalSearch(ProtossState initialState, StarcraftSearchGoal goal, int timeLimit)
	{
		SearchParameters<ProtossState> params(goal, initialState, StarcraftSearchConstraints());
		params.searchTimeLimit = timeLimit;
		
		params.useRepetitions 				= true;
		params.useIncreasingRepetitions 	= true;
		
		params.setRepetitions(DATA.getSupplyProvider(), 2);
		params.setRepetitionThreshold(DATA.getSupplyProvider(), 1);
		params.setRepetitions(DATA.getWorker(), 2);
		params.setRepetitions(DATA.getAction(BWAPI::UnitTypes::Protoss_Gateway), 2);
		params.setRepetitions(DATA.getAction(BWAPI::UnitTypes::Protoss_Zealot), 4);
		params.setRepetitions(DATA.getAction(BWAPI::UnitTypes::Protoss_Dragoon), 4);
		
		DFBBStarcraftSearch<ProtossState> SCSearch(params);
		
		return SCSearch.search();
	}
		
	void compareBuildOrderIncremental(const std::vector<Action> & buildOrder, int replanInterval, int timeLimit, int searchType)
	{
		// print the build order we are comparing against
		if (searchType == 0)
		{
			printBuildOrder(buildOrder);
		}
		
		int proPlanLength = getCompletionTime(buildOrder);
		
		std::pair<int, int> proResources = getTotalResources(buildOrder, false);
		
		// get the 2D vector of build orders
		std::vector< std::vector< Action > > incrementalBuildOrder = getIncrementalBuildOrder(buildOrder, replanInterval);
		
		// the current state, starts out as start of game
		ProtossState currentState(true);
		std::vector<Action> finalBuildOrder;
		StarcraftSearchGoal currentGoal;
		int totalSearchTime(0);
		
		bool solved = true;
		
		// attempt to solve the entire build order first
		if (searchType == 0)
		{
			StarcraftSearchGoal goal = getGoalFromBuildOrder(currentState, buildOrder);
			
			SearchResults initialResults = doOptimalSearch(currentState, goal, timeLimit, proPlanLength);
			
			double moreMinerals = (double)getTotalResources(initialResults.buildOrder, true).first / (double)proResources.first;
			double moreGas      = (proResources.second == 0) ? 1.0 : (double)getTotalResources(initialResults.buildOrder, true).second / (double)proResources.second;
			
			totalSearchTime += (int)initialResults.timeElapsed;
			
			if (initialResults.timeElapsed < timeLimit && initialResults.solved)
			{
				printf("%5d %10d %10d %10.4lf %10.4lf %10d %10d ", (int)buildOrder.size(), getCompletionTime(buildOrder), initialResults.solutionLength, moreMinerals, moreGas, totalSearchTime, 1);
				
				//printf("\n");
				//printBuildOrder(initialResults.buildOrder);
				
				return;
			}
		}
		else
		{
			StarcraftSearchGoal goal = getGoalFromBuildOrder(currentState, buildOrder);
			
			SearchResults initialResults = doSubOptimalSearch(currentState, goal, timeLimit);
			
			totalSearchTime += (int)initialResults.timeElapsed;
			
			if (initialResults.timeElapsed < timeLimit && initialResults.solved)
			{
			
				double moreMinerals = (double)getTotalResources(initialResults.buildOrder, true).first / (double)proResources.first;
				double moreGas      = (proResources.second == 0) ? 1.0 : (double)getTotalResources(initialResults.buildOrder, true).second / (double)proResources.second;
				printf("%10d %10.4lf %10.4lf %10d %10d\n", initialResults.solutionLength, moreMinerals, moreGas, totalSearchTime, 1);
				return;
			}
		}
				
		// for each of the re-planning phases
		for (size_t i(0); i<incrementalBuildOrder.size() && incrementalBuildOrder[i].size() > 0; ++i)
		{
			// get the goal from the current build order
			currentGoal = getGoalFromBuildOrder(currentState, incrementalBuildOrder[i]);
		
			SearchResults result;
			Timer t;
			
			//printf("Starting a search\n");
							
			if (searchType == 0)
			{			
				result = doOptimalSearch(currentState, currentGoal, timeLimit);
			}	
			else
			{
				result = doSubOptimalSearch(currentState, currentGoal, timeLimit);
			}
			
			//printf("Search took %d\n", (int)t.elapsed());
			totalSearchTime += (int)t.getElapsedTimeInMilliSec();		
			solved = result.solved;
						
			// for each action in that build order
			if (solved)
			{
				for (size_t a(0); a<result.buildOrder.size(); ++a)
				{
					// grab the next action
					Action nextAction = result.buildOrder[result.buildOrder.size() - 1 - a];
				
					// do the action in the current state
					currentState.doAction(nextAction, currentState.resourcesReady(nextAction));
					
					// push it into the final build order
					finalBuildOrder.push_back(nextAction);
				}
			}
			else
			{
				break;
			}
		}
		
		if (searchType == 0)
		{
			if (solved)
			{
				double moreMinerals = (double)getTotalResources(finalBuildOrder, false).first / (double)proResources.first;
				double moreGas      = (proResources.second == 0) ? 1.0 : (double)getTotalResources(finalBuildOrder, false).second / (double)proResources.second;
			
				printf("%5d %10d %10d %10.4lf %10.4lf %10d %10d ", (int)buildOrder.size(), getCompletionTime(buildOrder), currentState.getLastFinishTime(), moreMinerals, moreGas, totalSearchTime, 0);
			}
			else
			{
				printf("%5d %10d %10d %10.4lf %10.4lf %10d %10d ", (int)buildOrder.size(), getCompletionTime(buildOrder), -1, -1.0, -1.0, totalSearchTime, 0);
			}
		}
		else
		{
			if (solved)
			{
				double moreMinerals = (double)getTotalResources(finalBuildOrder, false).first / (double)proResources.first;
				double moreGas      = (proResources.second == 0) ? 1.0 : (double)getTotalResources(finalBuildOrder, false).second / (double)proResources.second;
			
				printf("%10d %10.4lf %10.4lf %10d %10d\n", currentState.getLastFinishTime(), moreMinerals, moreGas, totalSearchTime, 0);
			}
			else
			{
				printf("%10d %10.4lf %10.4lf %10d %10d\n", -1, -1.0, -1.0, totalSearchTime, 0);
			}
		}
	}
	
	void compareBuildOrderIncremental(const std::string & buildOrderString, int replanInterval, int timeLimit)
	{
		compareBuildOrderIncremental(getBuildOrder(buildOrderString), replanInterval, timeLimit, 0);
		compareBuildOrderIncremental(getBuildOrder(buildOrderString), replanInterval, timeLimit, 1);
	}
	
	void compareAllBuildOrders(const std::string & buildOrderFile)
	{
		std::string line;
		std::ifstream bofile(buildOrderFile.c_str());
		
		while (!bofile.eof()) 
		{		
			getline(bofile, line);
			
			if (line.compare(""))
			{
				std::vector<Action> buildOrder = getBuildOrder(line);
				compareBuildOrderIncremental(buildOrder, 2880, 30000, 0);
				compareBuildOrderIncremental(buildOrder, 2880, 30000, 1);
				buildOrderNum++;
			}
			
			
		}
	}

	std::vector<Action> getBuildOrder(std::string buildOrderString) const
	{
		std::vector<Action> buildOrder;
		std::stringstream ss(buildOrderString);	
		
		Action temp(0);
		
		while (!(ss >> temp).fail())
		{
			buildOrder.push_back(temp);
		}
		
		return buildOrder;
	}
	
	// gets a StarcraftSearchGoal from a build order
	StarcraftSearchGoal getGoalFromBuildOrder(const ProtossState initialState, const std::vector<Action> & buildOrder) const
	{
		StarcraftSearchGoal goal;
		
		// set the goal to include everything within the initial state first
		for (Action i=0; i<MAX_ACTIONS; ++i)
		{
			goal.setGoal(i, initialState.getNumUnits(i));
		}
		
		// then add each item in the build order
		for (size_t a(0); a<buildOrder.size(); ++a)
		{
			goal.setGoal(buildOrder[a], goal[buildOrder[a]] + 1);
		}
		
		return goal;
	}
	
	int getPlanLength(const std::vector< std::vector<Action> > & buildOrders, int buildOrderIndex) const
	{
		// initial state
		ProtossState s(true);
		
		// for each build order up until the index we're concerned with
		for (int i(0); i<=buildOrderIndex; ++i)
		{
			// do each action in this build order
			for (size_t a(0); a<buildOrders[i].size(); ++a)
			{			
				Action nextAction = buildOrders[i][a];
				s.doAction(nextAction, s.resourcesReady(nextAction));
			}
		}
		
		return s.getLastFinishTime();
	}
	
	void setGoalResources(StarcraftSearchGoal & goal, const std::vector< std::vector<Action> > & buildOrders, int buildOrderIndex) const
	{
		assert(buildOrderIndex < (int)buildOrders.size());

	
		// initial state
		ProtossState s(true);
		
		// for each build order up until the index we're concerned with
		for (int i(0); i<=buildOrderIndex; ++i)
		{
			// do each action in this build order
			for (size_t a(0); a<buildOrders[i].size(); ++a)
			{			
				Action nextAction = buildOrders[i][a];
				s.doAction(nextAction, s.resourcesReady(nextAction));
			}
		}
		
		// set the goal accordingly
		goal.setMineralGoal(s.getFinishTimeMinerals());
		goal.setGasGoal(s.getFinishTimeGas());
	}
	
	void setGoalResources(StarcraftSearchGoal & goal, const std::vector< Action > & buildOrder) const
	{	
		// initial state
		ProtossState s(true);
		
		// do each action in this build order
		for (size_t a(0); a<buildOrder.size(); ++a)
		{			
			Action nextAction = buildOrder[a];
			s.doAction(nextAction, s.resourcesReady(nextAction));
		}
		
		// set the goal accordingly
		//goal.setMineralGoal(s.getFinishTimeMinerals());
		//goal.setGasGoal(s.getFinishTimeGas());
	}
	
	std::pair<int, int> getTotalResources(const std::vector<Action> & buildOrder, bool reverse) const
	{
		ProtossState s(true);
		int minerals(0), gas(0);
	
		for (size_t a(0); a<buildOrder.size(); ++a)
		{			
			Action nextAction = reverse ? buildOrder[buildOrder.size() - 1 - a] : buildOrder[a];
			s.doAction(nextAction, s.resourcesReady(nextAction));
			minerals += DATA[nextAction].mineralPrice();
			gas += DATA[nextAction].gasPrice();
		}
		
		minerals += s.getFinishTimeMinerals();
		gas += s.getFinishTimeGas();
		
		return std::pair<int, int>(minerals, gas);
	}
	
	void compareBuildOrderReplan(const std::vector<Action> & buildOrder, int replanInterval, int timeLimit)
	{	
		// print the build order we are comparing against
		printBuildOrder(buildOrder);
		
		// the current state, starts out as start of game
		ProtossState currentState(true);
		
		// the current build order, reversed for easy popping
		StarcraftSearchGoal currentGoal = getGoalFromBuildOrder(currentState, buildOrder);
		
		std::vector<Action> finalBuildOrder;
		
		int replans = 0;
		//currentGoal.printGoal();
		
		// while we still have units to look for
		while (currentGoal.hasGoal())
		{
			// do the search
			SearchResults currentResult = doOptimalSearch(currentState, currentGoal, timeLimit);
			
			// if we have finished the search in time
			if (currentResult.timeElapsed < timeLimit)
			{
				// copy the rest of the build order into the final build order
				for (size_t i(0); i<currentResult.buildOrder.size(); ++i)
				{
					// get the action (results build order is reversed)
					Action a = currentResult.buildOrder[currentResult.buildOrder.size() - 1 - i];
					
					// do the action in the current state
					currentState.doAction(a, currentState.resourcesReady(a));
					
					// push it into the final build order
					finalBuildOrder.push_back(a);
					
					// decrease the goal amount of this unit by 1
					//currentGoal.setGoal(a, currentGoal[a] - 1);
				}
				
				// break out since we will have no actions left
				break;
			}
			// otherwise we have timed out, so carry out more of the build order until we reach replanInterval
			else
			{
				
				int originalFrame = currentState.getCurrentFrame();
				replans++;
				
				// keep doing actions from the build order until:
				// 1) We pass an additional replanInterval in time
				// 2) We run out of actions in the build order
				for (size_t i(0); (currentState.getCurrentFrame() < (originalFrame + replanInterval)) && (i < currentResult.buildOrder.size()); ++i)
				{
					// get the action
					Action a = currentResult.buildOrder[currentResult.buildOrder.size() - 1 - i];
					
					// do the action in the current state
					currentState.doAction(a, currentState.resourcesReady(a));
					
					// push it into the final build order
					finalBuildOrder.push_back(a);
					
					// decrease the goal amount of this unit by 1
					//currentGoal.setGoal(a, currentGoal[a] - 1);
				}			
				
				//printBuildOrder(finalBuildOrder);
			}
		}
		
		printf("%5d %10d %10d ", (int)buildOrder.size(), getCompletionTime(buildOrder), currentState.getLastFinishTime());
		//printBuildOrder(finalBuildOrder);
	}
	
	void compareSubBuildOrderReplan(const std::vector<Action> & buildOrder, int replanInterval, int timeLimit)
	{
		// print the build order we are comparing against
		//printBuildOrder(buildOrder);
		
		// the current state, starts out as start of game
		ProtossState currentState(true);
		
		// the current build order, reversed for easy popping
		StarcraftSearchGoal currentGoal = getGoalFromBuildOrder(currentState, buildOrder);
		
		std::vector<Action> finalBuildOrder;
		
		int replans = 0;
		
		//currentGoal.printGoal();
		
		
		// while we still have units to look for
		while (currentGoal.hasGoal())
		{
			// do the search
			SearchResults currentResult = doSubOptimalSearch(currentState, currentGoal, timeLimit);
			// if we have finished the search in time
			if (currentResult.timeElapsed < timeLimit)
			{
				// copy the rest of the build order into the final build order
				for (size_t i(0); i<currentResult.buildOrder.size(); ++i)
				{
					// get the action (results build order is reversed)
					Action a = currentResult.buildOrder[currentResult.buildOrder.size() - 1 - i];
					
					// do the action in the current state
					currentState.doAction(a, currentState.resourcesReady(a));
					
					// push it into the final build order
					finalBuildOrder.push_back(a);
					
					// decrease the goal amount of this unit by 1
					//currentGoal.setGoal(a, currentGoal[a] - 1);
				}
				
				// break out since we will have no actions left
				break;
			}
			// otherwise we have timed out, so carry out more of the build order until we reach replanInterval
			else
			{
				
				int originalFrame = currentState.getCurrentFrame();
				replans++;
				
				// keep doing actions from the build order until:
				// 1) We pass an additional replanInterval in time
				// 2) We run out of actions in the build order
				for (size_t i(0); (currentState.getCurrentFrame() < (originalFrame + replanInterval)) && (i < currentResult.buildOrder.size()); ++i)
				{
					// get the action
					Action a = currentResult.buildOrder[currentResult.buildOrder.size() - 1 - i];
					
					// do the action in the current state
					currentState.doAction(a, currentState.resourcesReady(a));
					
					// push it into the final build order
					finalBuildOrder.push_back(a);
					
					// decrease the goal amount of this unit by 1
					//currentGoal.setGoal(a, currentGoal[a] - 1);
				}			
				
				//printBuildOrder(finalBuildOrder);
			}
		}
		
		printf("%10d\n", currentState.getLastFinishTime());
	}
	
	std::vector< std::vector<Action> > getIncrementalBuildOrder(const std::vector<Action> & buildOrder, int replanInterval)
	{
		int length = getCompletionTime(buildOrder);
		int count = 0;
	
		std::vector< std::vector< Action > > incrementalBuildOrder((length / replanInterval) + 1, std::vector<Action>());
		
		ProtossState s(true);
		
		for (size_t i(0); i<buildOrder.size(); ++i)
		{
			s.doAction(buildOrder[i], s.resourcesReady(buildOrder[i]));
			incrementalBuildOrder[s.getCurrentFrame() / replanInterval].push_back(buildOrder[i]);
			count++;
		}
		
	//	for (size_t i(0); i<incrementalBuildOrder.size(); ++i)
	///	{
	//		for (size_t j(0); j<incrementalBuildOrder[i].size(); ++j)
	//		{
	//			printf("%d ", incrementalBuildOrder[i][j]);
	//		}
	//		printf("\n");
	//	}
		
		//printf("Parsed length %d into length %d\n", (int)buildOrder.size(), count);
		
		return incrementalBuildOrder;
	}

};
