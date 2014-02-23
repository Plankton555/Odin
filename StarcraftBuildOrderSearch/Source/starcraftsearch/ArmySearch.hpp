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
class ArmySearch : public StarcraftSearch
{
	SearchParameters					 	params;				// parameters that will be used in this search
	
	FrameCountType							upperBound; 		// the current upper bound for search
			
	StarcraftState	 						winner;				// the winning state so far
	
	SearchResults 							results;			// the results of the search so far
	
	unsigned long long 						nodesExpanded;		// how many nodes have been expanded
	unsigned long long 						numChildren;		// number of children generated
	unsigned long long 						numGenerations;		// number of parents generated
	
	bool 									printNewBest;		// when set to true
	bool 									finishedLoadingSaveState;
	
	int 									buckets;
	std::vector<int>    					evalValues;
	std::vector< std::vector<Action> > 		buildOrders;
	std::vector<StarcraftState>				finalStates;
	
	
	int 									ttcuts;
	TranspositionTable<unsigned int, int>   TT;
	
public:
	
	ArmySearch() {}
	
	ArmySearch(SearchParameters p) 
		: params(p)
		, nodesExpanded(0)
		, numChildren(0)
		, numGenerations(0)
		, buckets(100)
		, evalValues(buckets, 0)
		, buildOrders(buckets, std::vector<Action>())
		, finalStates(buckets)
		, ttcuts(0)
		, TT(1000000)
	{
		if (params.initialUpperBound == 0)
		{
			upperBound = 0;
		}
		else
		{
			upperBound = params.initialUpperBound;
		}
		
		results.upperBound = upperBound;
	}
	
	// function which is called to do the actual search
	virtual SearchResults search()
	{	
		searchTimer.start();
		
		// try to search since we throw exception on timeout
		try 
		{
			DFBB(params.initialState, 0);
			
			results.timedOut = false;
		
		// if we catch a timeout exception
		} 
		catch (int e) 
		{
			// store that we timed out in the results
			results.timedOut = true;

			// for some reason MSVC++ complains 1about e being unused, so use it
			e = e + 1;
		}
		
		// set the results
		results.nodesExpanded = nodesExpanded;
		results.timeElapsed   = searchTimer.getElapsedTimeInMilliSec();
		
	    int skip = upperBound / buckets;
	    int max = 0;
		
		for (int i=0; i<buckets; i++)
		{
		    if (evalValues[i] > 0)
		    {
		        int low = i * skip;
		
		        printf("%6d  %6d  %10d   ", low, low + skip, evalValues[i]);
		        //for (size_t a=0; a<buildOrders[i].size(); ++a)
		        //{
		        //    printf("%d ", buildOrders[i][buildOrders[i].size()-1-a]);
		        //}
		        printf("\n");
		        
		        if (evalValues[i] > max)
		        {
		        	max = evalValues[i];
		        	results.buildOrder = buildOrders[i];
		        }	
		    }
		}
		
		printf("Transposition (Cuts = %d) (Collision = %d) (Found = %d) (NotFound = %d)\n", ttcuts, TT.numCollisions(), TT.numFound(), TT.numNotFound());
		
		
		std::vector< std::pair< FrameCountType, ResourceCountType > > armyIntegral = finalStates[buckets-1].getArmyIntegral();
		for (size_t i(0); i<armyIntegral.size(); ++i)
		{
			
			printf("%d %d\n", armyIntegral[i].first, armyIntegral[i].second);
		}
		printf("%d %d\n", finalStates[buckets-1].getLastFinishTime(), finalStates[buckets-1].getArmyValue());
		
		return results;
	}
	
	const int getBucket(int finishTime) const
	{
	    return (int)  (((float)finishTime / upperBound) * buckets);
	}
	
	const int eval(StarcraftState & s) const
	{
		return s.getFinishArmyValue();
	}

	void updateBuckets(int val, int bucket, StarcraftState & s)
	{
		for (int i(bucket); i<buckets; ++i)
		{
			if (val > evalValues[i])
			{
				evalValues[i] = val;
				buildOrders[i] = getBuildOrder(s);
				finalStates[i] = s;
			}
			else
			{
				break;
			}
		}
	}
	
	// recursive function which does all search logic
	void DFBB(StarcraftState & s, int depth)
	{		
		// increase the node expansion count
		nodesExpanded++;

		if (nodesExpanded % 1000000 == 0)
		{
			std::vector<Action> buildOrder(getBuildOrder(s));
			
			fprintf(stderr, "%15d ", nodesExpanded);
			for (size_t a=0; a<buildOrder.size(); ++a)
			{
				fprintf(stderr, "%d ", buildOrder[buildOrder.size()-1-a]);
			}

			fprintf(stderr, "\n");
		}
		
		// the time at which the last thing in the queue will finish
		int finishTime = s.getLastFinishTime();
		if (finishTime >= upperBound)
		{
		    return;
		}
		
		// transposition table lookup
		int lookupVal = TT.lookup(s.hashAllUnits(1), s.hashAllUnits(2));
		
		// if we have a stored value and it is earlier
		if (lookupVal != -1 && lookupVal < finishTime)
		{
		    ttcuts++;
		    
		    // make a cut here and stop the search
		   // return;
		}

		// transposition table save
		TT.save(s.hashAllUnits(1), s.hashAllUnits(2), finishTime);
		
		// evaluate the current state's army somehow
		int bucket = getBucket(s.getCurrentFrame());
		int evalValue = eval(s);		
		
		// if it is a new maximum for this time, store it
		if (evalValue > evalValues[bucket])
		{
		    updateBuckets(evalValue, bucket, s);
		}
		
		// if we are using search timeout and we are over the limit
		if (params.searchTimeLimit && (nodesExpanded % 1000 == 0) && (searchTimer.getElapsedTimeInMilliSec() > params.searchTimeLimit))
		{
			// throw an exception to unroll the recursion
			throw 1;
		}
		
		// get the legal action set
		ActionSet legalActions = s.getLegalActionsMonteCarlo(params.goal); 

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

		// if we have children, update the counter
		if (!legalActions.isEmpty())
		{
			numGenerations += 1;
			numChildren += legalActions.numActions();
		}
	
		// while there are still legal actions to perform
		while (!legalActions.isEmpty()) 
		{				
			// get the next action
			Action nextAction = legalActions.popAction();

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

			// limit repetitions to how many we can make based on current used supply
			if (DATA[nextAction].supplyRequired() > 0)
			{
				int haveSupplyFor = (s.getMaxSupply() + s.getSupplyInProgress() - s.getCurrentSupply()) / DATA[nextAction].supplyRequired();

				repeat = std::min(repeat, haveSupplyFor);
			}

			// for each repetition of this action
			for (int r = 0; r < repeat; ++r)
			{
				// if the action is still legal
				if (child.isLegalMonteCarlo(nextAction, params.goal))
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
			
			//if (stillLegal)
			//{
			    child.setParent(&s);
			    child.setActionPerformedK((UnitCountType)repeat);
			    DFBB(child, depth+1);
			//}
		}	
	}
};

}