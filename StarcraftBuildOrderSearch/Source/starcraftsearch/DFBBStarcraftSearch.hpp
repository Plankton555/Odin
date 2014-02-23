#pragma once

#include "BWAPI.h"
#include <boost/foreach.hpp>
#include "assert.h"
#include <stdio.h>

#include "StarcraftSearch.hpp"
#include "StarcraftSearchConstraint.hpp"
#include "SearchParameters.hpp"
#include "TranspositionTable.hpp"

namespace BuildOrderSearch
{
class DFBBStarcraftSearch : public StarcraftSearch
{
	SearchParameters params;		// parameters that will be used in this search
	
	int 	upperBound,									// the current upper bound for search
			winnerWorkerCount;							// how many workers are in best solution
			
	StarcraftState winner;							// the winning state so far
	
	SearchResults results;								// the results of the search so far
	
	bool 	winnerFound;								// whether a best solution has been found
	
	unsigned long long 	nodesExpanded;					// how many nodes have been expanded
	unsigned long long	numChildren;					// number of children generated
	unsigned long long	numGenerations;					// number of parents generated
	
	bool printNewBest;									// when set to true
	
	bool finishedLoadingSaveState;
	
	//TranspositionTable<unsigned int, int>   TT;
	int ttcuts;
	
public:
	
	DFBBStarcraftSearch(SearchParameters p) 
		: params(p)
		, winnerWorkerCount(0)
		, winner(true)
		, nodesExpanded(0)
		, numChildren(0)
		, numGenerations(0)
		, finishedLoadingSaveState(false)
		//, TT(1000000)
	{
		if (params.initialUpperBound == 0)
		{
			upperBound = params.initialState.calculateUpperBoundHeuristic(params.goal);
			//printf("Frame: %d, Upper: %d\n", params.initialState.getCurrentFrame(), upperBound);
		}
		else
		{
			upperBound = params.initialUpperBound;
		}
		
		if (params.useSaveState)
		{
			upperBound = params.saveState.getUpperBound();
		}
		
		results.upperBound = upperBound;
		
		// calculate the relevant actions to our goal
		relevantActions = calculateRelevantActions();
	}
	
	// function which is called to do the actual search
	virtual SearchResults search()
	{
		searchTimer.start();

		// try to search since we throw exception on timeout
		try {
		
			// search on the initial state
			DFBB(params.initialState, 0);

			results.timedOut = false;
		
		// if we catch a timeout exception
		} catch (int e) {
		
			//printf("Search timed out at %dms\n", params.searchTimeLimit);
		
			// store that we timed out in the results
			results.timedOut = true;

			// for some reason MSVC++ complains about e being unused, so use it
			e = e + 1;
		}
		
		// set the results
		results.nodesExpanded = nodesExpanded;
		results.timeElapsed   = searchTimer.getElapsedTimeInMilliSec();
		
		return results;
	}
	
	// recursive function which does all search logic
	void DFBB(StarcraftState & s, int depth)
	{		
		// increase the node expansion count
		nodesExpanded++;
		
		// if we have constraints and they are not met
		if (params.useConstraints && !s.meetsConstraints(params.ssc))
		{
			// this state is not legal
			return;
		}
	
		// the time at which the last thing in the queue will finish
		int finishTime = s.getLastFinishTime();
		
		/*int lookupVal = TT.lookup(s.hashAllUnits(1), s.hashAllUnits(2));
		if (lookupVal != -1 && lookupVal < finishTime)
		{
		    ttcuts++;
		    //return;
		}

		TT.save(s.hashAllUnits(1), s.hashAllUnits(2), finishTime);*/

		// if we already have completed the units for the goal, don't worry about last finish time
		if (s.meetsGoalCompleted(params.goal))
		{
			finishTime = s.getCurrentFrame();
		}

		// if we have met the goal, we're good
		if (s.meetsGoal(params.goal)) 
		{
			// if it's better than the current best solution, set the new best
			if (finishTime < upperBound)//  || ((finishTime == upperBound) && (s.getWorkerCount() > winnerWorkerCount))) 
			{
				// set the winning info
				upperBound = finishTime;
				winner = s;
				winnerFound = true;
				winnerWorkerCount = s.getWorkerCount();

				results = SearchResults(true, upperBound, nodesExpanded, searchTimer.getElapsedTimeInMilliSec(), s.getBuildOrder());
				results.upperBound = s.calculateUpperBoundHeuristic(params.goal);
				results.lowerBound = s.eval(params.goal);
				results.avgBranch = numChildren / (double)numGenerations;
				results.minerals = s.getFinishTimeMinerals();
				results.gas = s.getFinishTimeGas();
				
				results.saveState = SearchSaveState(getBuildOrder(s), upperBound);
				
				//graphVizOutput(s, true);
				results.printResults(true);
				
				s.printData();

				return;
			}
		}
		
		// if we are using search timeout and we are over the limit
		// (nodesExpanded % 1000 == 0) only checks the time every 1000 expansions, since it is slow
		if (params.searchTimeLimit && (nodesExpanded % 200 == 0) && (searchTimer.getElapsedTimeInMilliSec() > params.searchTimeLimit))
		{
			results.saveState = SearchSaveState(getBuildOrder(s), upperBound);
			//results.saveState.print();
		
			// throw an exception to unroll the recursion
			throw 1;
		}
		
		// get the legal action set
		ActionSet legalActions = s.getLegalActions(params.goal); 

		// only use relevant actions
		legalActions = legalActions & relevantActions;
		
		// if we enabled the supply bounding flag
		if (params.useSupplyBounding)
		{
			// if we are more than 2 supply providers in the lead 
			if ((s.getMaxSupply() - s.getCurrentSupply()) >= params.supplyBoundingThreshold*DATA[DATA.getSupplyProvider()].supplyProvided())
			{
				// make supply providers illegal
				legalActions.subtract(DATA.getSupplyProvider());
			}
		}
		
		// if we enabled the always make workers flag, and workers are legal
		if (params.useAlwaysMakeWorkers && !params.goal[DATA.getWorker()] && legalActions[DATA.getWorker()])
		{
			ActionSet tempLegal(legalActions);
			ActionSet legalBeforeWorker;
			
			// compute when the next worker will be trainable
			int workerReady = s.resourcesReady(DATA.getWorker());
			
			// for each other legal action
			while (!tempLegal.isEmpty())
			{
				Action nextAction = tempLegal.popAction();
				
				// if the action will be ready before the next worker
				if (s.resourcesReady(nextAction) <= workerReady)
				{
					// it's legal
					legalBeforeWorker.add(nextAction);
				}
			}
			
			// update the legal actions
			legalActions = legalBeforeWorker;
		}
		
		// if we enabled the use worker cutoff flag and we're above the cutoff
		if (params.useWorkerCutoff && s.getCurrentFrame() > (params.workerCutoff * upperBound))
		{
			// workers are no longer legal
			legalActions.subtract(DATA.getWorker());

			// if we have enough supply for the remaining goal
			if (s.hasEnoughSupplyForGoal(params.goal))
			{
				// make supply providers illegal
				legalActions.subtract(DATA.getSupplyProvider());
			}
		}	

		// if we have children, update the counter
		if (!legalActions.isEmpty())
		{
			numGenerations += 1;
			numChildren += legalActions.numActions();
		}
		
		// load the save state if we are using it
		if (params.useSaveState && !finishedLoadingSaveState)
		{
			// if we are under the saved depth, load accordingly
			if (depth < params.saveState.getDepth())
			{
				// pop actions until the NEXT action is the one we want to start on
				while (!legalActions.isEmpty() && legalActions.nextAction() != params.saveState[depth])
				{
					legalActions.popAction();
				}
			}
			// if we are over the depth, we are finished loading
			else
			{
				finishedLoadingSaveState = true;
			}
		}
		
		// children of this state in the search
		std::vector<StarcraftState> childStates;

		// while there are still legal actions to perform
		while (!legalActions.isEmpty()) 
		{				
			// get the next action
			Action nextAction = legalActions.popAction();
			
			// when this action would finish
			int actionFinishTime = s.resourcesReady(nextAction) + DATA[nextAction].buildTime();
			
			// heuristic value of the goal state
			int heuristicTime = s.getCurrentFrame() + s.eval(params.goal, params.useLandmarkLowerBoundHeuristic);
			
			// the h value for this node
			int h = (actionFinishTime > heuristicTime) ? actionFinishTime : heuristicTime;
			
			// primary cut-off, very quick heuristic
			if (h <= upperBound)
			{
				bool stillLegal = true;
				StarcraftState child(s);
				
				// set the repetitions if we are using repetitions, otherwise set to 1
				int repeat = params.useRepetitions ? params.getRepetitions(nextAction) : 1;
				
				// if we are using increasing repetitions
				if (params.useIncreasingRepetitions)
				{
					// if we don't have the threshold amount of units, use a repetition value of 1
					repeat = child.getNumUnits(nextAction) >= params.getRepetitionThreshold(nextAction) ? repeat : 1;
				}
				
				// make sure we don't repeat to more than we need for this unit type
				if (params.goal.get(nextAction))
				{
					repeat = std::min(repeat, params.goal.get(nextAction) - child.getNumUnits(nextAction));
				}
				else if (params.goal.getMax(nextAction))
				{
					repeat = std::min(repeat, params.goal.getMax(nextAction) - child.getNumUnits(nextAction));
				}

				// limit repetitions to how many we can make based on current used supply
				if (DATA[nextAction].supplyRequired() > 0)
				{
					int haveSupplyFor = (s.getMaxSupply() + s.getSupplyInProgress() - s.getCurrentSupply()) / DATA[nextAction].supplyRequired();

					repeat = std::min(repeat, haveSupplyFor);
				}
				
				// if we're not finished loading the state, repeat value is 1
				if (params.useSaveState && !finishedLoadingSaveState)
				{
					repeat = 1;
				}

				// for each repetition of this action
				for (int r = 0; r < repeat; ++r)
				{
					// if the action is still legal
					if (child.isLegal(nextAction, params.goal))
					{						
						int readyTime = child.resourcesReady(nextAction); 
						child.doAction(nextAction, readyTime);
					}
					// if it's not legal, break the chain
					else
					{
						stillLegal = false;
						break;
					}
				}
				
				// if all actions in a row are legal, recurse on the child
				if (stillLegal)
				{
					child.setParent(&s);
					child.setActionPerformedK((UnitCountType)repeat);
					//DFBB(child, depth+1);
					childStates.push_back(child);
				}
			}
		}	
		
		//std::sort(childStates.begin(), childStates.end(), StarcraftStateCompare<StarcraftStateType>(params));
		//std::random_shuffle(childStates.begin(), childStates.end());
		for (size_t i(0); i<childStates.size(); ++i)
		{
		    DFBB(childStates[i], depth+1);
		}
	}
	

// builds the relevant action set for this search
	ActionSet calculateRelevantActions() 
	{
		// the initial mask is blank
		ActionSet all;

		// let's say that we will always allow workers and supply producers
		all.add(DATA.getWorker());
		all.add(DATA.getSupplyProvider());

		// loop through each nonzero element of the goal
		for (Action a(0); a<DATA.size(); ++a)
		{
			// if we want some of this action
			if (params.goal.get(a) > 0)
			{
				//printf("GOAL HAS: %s\n", DATA[a].getName().c_str());
				
				// add itself to the mask
				all.add(a);

				// if this action costs gas
				if (DATA[a].gasPrice() > 0) 
				{
					// extractors are now relevant
					all.add(DATA.getRefinery());
				}

				// also add all recursive prerequisites of this action
				calculateRecursivePrerequisites(a, all);
			}
		}

		return all;
	}
};
}