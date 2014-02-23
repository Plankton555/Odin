#pragma once

#include "StarcraftSearchConstraint.hpp"
#include "StarcraftSearchGoal.hpp"
#include "StarcraftState.hpp"
#include "SearchSaveState.hpp"

namespace BuildOrderSearch
{

class SearchParameters
{

	void init()
	{
		for (int i=0; i<MAX_ACTIONS; ++i)
		{
			repetitionValues[i] = 1;
			repetitionThresholds[i] = 0;
		}
	}

public:

	//      Flag which determines whether or not doubling macro actions will be used in search.
	//      Macro actions (see paper) trade suboptimality for decreasing search depth. For large
	//          plans repetitions are necessary. For example, to make probes only build in twos
	//          set useRepetitions = true and repetitionValues[probeAction] = 2
	//
	//      true:  macro actions are used, stored in repetitionValues array
    //      false: macro actions not used, all actions will be carried out once
	bool 	useRepetitions;
	int		repetitionValues[MAX_ACTIONS];
	
	//      Flag which determines whether increasing repetitions will be used in search
	//      Increasing repetitions means the reptition value will be 1 until we have at least
    //      repetitionThresholds[a] count of action a. For example, setting:
	//          repetitionThresholds[pylonAction] = 1, repetitionValues[pylonAction] = 2
	//          means that the first pylon will build on its own but all further pylons will
	//          be built 2 at a time.
	//
	//      true:  increasing repetitions are used
	//      false: increasing repetitions not used
	bool	useIncreasingRepetitions;
	int		repetitionThresholds[MAX_ACTIONS];

	//		Flag which determines whether or not we use worker cutoff pruning in search.
	//		Worker cutoff pruning stops workers from being constructed after a certain number
	//			of frames have passed in the search. Intuitively we build the majority of workers
	//			at the beginning of a build, so this can enforce it to make search faster. If
	//          true, workers are no longer legal if currentFrame > workerCutoff * uperBound
	//          in our search algorithm. If workerCutoff is 1, workers will not be pruned.
	//
	//      true:  worker cutoff is used
	//      false: worker cutoff not used
	bool	useWorkerCutoff;					
	double 	workerCutoff;
	
	//      Flag which determines whether or not we use constraints in our search
	//      Constraints are StarcraftSearchConstraints objects which can take any form, and are just
	//          used as a boolean evaluator about our current StarcraftState. Examples of this can be
	//          pruning the search if we do not have a certain unit by frame F, allowing for searches
	//          to incorporate early defenses into a longer build, etc. The constraints are stored
	//          in the StarcraftSearchConstraints object names ssc
	//
	//      true:  constraints are used
	//      false: constraints not used
	bool	useConstraints;						
	StarcraftSearchConstraints 	ssc;
	
	//      Flag which determines whether or not we always make workers during search
	//      This abstraction changes the search so that it always makes a worker if it is able to. It
	//          accomplished this by modifying the current legal actions to exclude anything that
	//          can't be started before the next worker. This ends up producing longer makespans but
	//          the economy it produces is much better. Optimal makespan plans often produce very
	//          few workers and the long term economy suffers.
	//
	//      true:  always make workers is used
	//      false: always make workers is not used
	bool	useAlwaysMakeWorkers;
	
	//      Flag which determines whether or not we use supply bounding in our search
	//      Supply bounding makes supply producing buildings illegal if we are currently ahead
	//          on supply by a certain amount. If we currently have more than
	//          supplyBoundingThreshold extra supply buildings worth of supply, we no longer
	//          build them. This is an abstraction used to make search faster which may
	//          produce suboptimal plans.
	//
	//      true:  supply bounding is used
	//      false: supply bounding is not used
	bool	useSupplyBounding;
	int		supplyBoundingThreshold;
	
	
	//      Flag which determines whether or not we use various heuristics in our search.
	//
	//      true:  the heuristic is used
	//      false: the heuristic is not used
	bool	useLandmarkLowerBoundHeuristic;
	bool	useResourceLowerBoundHeuristic;
	
	//      Search time limit measured in milliseconds
	//      If searchTimeLimit is set to a value greater than zero, the search will effectively
	//          time out and the best solution so far will be used in the results. This is
	//          accomplished by throwing an exception if the time limit is hit. Time is checked
	//          once every 1000 nodes expanded, as checking the time is slow.
	int		searchTimeLimit;
	
	//      Initial upper bound for the DFBB search
	//      If this value is set to zero, DFBB search will automatically determine an
	//          appropriate upper bound using an upper bound heuristic. If it is non-zero,
	//          it will use the value as an initial bound.
	int		initialUpperBound;
			
	//      StarcraftSearchGoal used for the search. See StarcraftSearchGoal.hpp for details
	StarcraftSearchGoal			goal;	
	
	//      Initial StarcraftState used for the search. See StarcraftState.hpp for details
	StarcraftState				initialState;
	
	//		Save State
	bool 	useSaveState;
	SearchSaveState				saveState;
	
	// default constructor
	SearchParameters() 
		: useRepetitions(true)
		, useIncreasingRepetitions(false)
		, useWorkerCutoff(false)
		, workerCutoff(1)
		, useConstraints(false)
		, useAlwaysMakeWorkers(false)
		, useSupplyBounding(false)
		, supplyBoundingThreshold(1)
		, useLandmarkLowerBoundHeuristic(true)
		, useResourceLowerBoundHeuristic(true)
		, searchTimeLimit(0)
		, initialUpperBound(0)
		, useSaveState(false)	
	{
		init();
	}

	// alternate constructor
	SearchParameters(StarcraftSearchGoal g, StarcraftState i = StarcraftState(true), StarcraftSearchConstraints s = StarcraftSearchConstraints()) : 	
	
		useRepetitions(true), 
		useIncreasingRepetitions(false), 
		useWorkerCutoff(false), 
		workerCutoff(1), 
		useConstraints(false),
		ssc(s), 
		useAlwaysMakeWorkers(false),
		useSupplyBounding(false), 
		supplyBoundingThreshold(1),
		useLandmarkLowerBoundHeuristic(true), 
		useResourceLowerBoundHeuristic(true), 
		searchTimeLimit(0), 
		initialUpperBound(0), 
		
		goal(g),initialState(i),
		
		useSaveState(false)
	{
		init();
	}
	
	void 	setRepetitions(Action a, int repetitions) 		{ repetitionValues[a] = repetitions; }
	void 	setRepetitionThreshold(Action a, int thresh)	{ repetitionThresholds[a] = thresh; }
	int 	getRepetitions(Action a)						{ return repetitionValues[a]; }
	int 	getRepetitionThreshold(Action a)				{ return repetitionThresholds[a]; }
	
	void print()
	{
		goal.printGoal();
	
		printf("\n\nSearch Parameter Information\n\n");
	
		printf("%s", useRepetitions ? 					"\tUSE       Repetitions\n" : "");
		printf("%s", useIncreasingRepetitions ? 		"\tUSE       Increasing Repetitions\n" : "");
		printf("%s", useWorkerCutoff ? 					"\tUSE       Worker Cutoff\n" : "");
		printf("%s", useConstraints ? 					"\tUSE       Search Constraints\n" : "");
		printf("%s", useLandmarkLowerBoundHeuristic ? 	"\tUSE       Landmark Lower Bound\n" : "");
		printf("%s", useResourceLowerBoundHeuristic ? 	"\tUSE       Resource Lower Bound\n" : "");
		printf("%s", useAlwaysMakeWorkers ? 			"\tUSE       Always Make Workers\n" : "");
		printf("%s", useSupplyBounding ? 				"\tUSE       Supply Bounding\n" : "");
		printf("\n");
	
		for (int a = 0; a < DATA.size(); ++a)
		{
			if (repetitionValues[a] != 1)
			{
				printf("\tREP %7d %s\n", repetitionValues[a], DATA[a].getName().c_str());
			}
		}
		
		for (int a = 0; a < DATA.size(); ++a)
		{
			if (repetitionThresholds[a] != 0)
			{
				printf("\tTHR %7d %s\n", repetitionThresholds[a], DATA[a].getName().c_str());
			}
		}
		
		printf("\n\n");
	}
};

}