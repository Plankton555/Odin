#ifndef ACTION_IN_PROGRESS_H
#define ACTION_IN_PROGRESS_H

#include <string.h>
#include <queue>
#include <algorithm>

#include "BWAPI.h"
#include "ActionSet.hpp"
#include "StarcraftAction.hpp"
#include "StarcraftData.hpp"
#include <math.h>

namespace BuildOrderSearch
{
class ActionInProgress
{
	friend class ActionsInProgress;

	Action 			action;
	FrameCountType 	time;
	
	ActionInProgress() : action(0), time(0) { memset(this, 0, sizeof(*this)); }
	ActionInProgress(unsigned char a, FrameCountType t) : action(a), time(t) {}
	
public:

	// we want to sort these in descending order in ActionsInProgress
	bool operator < (const ActionInProgress & rhs) const { return time > rhs.time; }
};

class ActionsInProgress
{

	ActionInProgress	inProgress[MAX_PROGRESS];
	unsigned char		inProgressSize;
	UnitCountType 		numProgress[MAX_ACTIONS];	// how many of each unit are in progress
	
public:

	ActionsInProgress() 
		: inProgressSize(0) 
	{
		// set the array to all zeros LOLOLOLOL
		for (int i(0); i<MAX_ACTIONS; ++i)
		{
			numProgress[i] = 0;
		}
	}
	
	UnitCountType operator [] (int a) const
	{
		return numProgress[a];
	}
	
	UnitCountType numInProgress(Action a) const
	{
		return numProgress[a];	
	}
	
	void addAction(Action a, int time)
	{
		assert(inProgressSize < (MAX_PROGRESS - 1));
		assert(numProgress[a] < 100);
	
		// add the action to the in progress array
		inProgress[inProgressSize] = ActionInProgress(a, (unsigned short)time);
		
		// increase the counter
		inProgressSize++;
		
		// increase the specific count of a
		numProgress[a]++;
		
		// if there is more than one element in the array and the new one has higher time, sort
		if ((inProgressSize > 1) && (time > inProgress[inProgressSize-2].time))
		{
			// sort the list in descending order
			std::sort(inProgress, inProgress + inProgressSize);
		}
	}
	
	void popNextAction()	
	{
		assert(inProgressSize > 0);
	
		// there is one less of the last unit in progress
		numProgress[inProgress[inProgressSize-1].action]--;
	
		// the number of things in progress goes down
		inProgressSize--;
	}
	
	bool isEmpty() const
	{
		return inProgressSize == 0;
	}
	
	FrameCountType nextActionFinishTime() const
	{
		assert(inProgressSize > 0);
		
		return inProgress[inProgressSize-1].time;
	}
	
	int size() const
	{
		return inProgressSize;
	}
	
	FrameCountType getLastFinishTime() const
	{
		if (inProgressSize == 0)
		{
			return 0;
		}
		
		return inProgress[0].time;
	}
	
	FrameCountType nextActionFinishTime(Action a) const
	{
		assert(numProgress[a] > 0);
		
		for (int i=inProgressSize-1; i >=0; --i)
		{
			if (inProgress[i].action == a)
			{
				return inProgress[i].time;
			}
		}
		
		return -1;
	}
	
	Action getAction(int i) const
	{
		return inProgress[i].action;
	}
	
	FrameCountType getTime(int i) const
	{
		return inProgress[i].time;
	}
	
	Action nextAction() const
	{
		assert(inProgressSize > 0);
	
		return inProgress[inProgressSize-1].action;
	}
	
	void printActionsInProgress()
	{
		for (int i(0); i<MAX_ACTIONS; ++i)
		{
			if (numProgress[i] > 0)
			{
				printf("Progress: %d %d %d\n", i, (int)numProgress[i], (int)inProgress[i].time);
			}
		}
	}
	
	FrameCountType whenActionsFinished(ActionSet actions) const
	{
		assert(!actions.isEmpty());
	
		// the maximum of the (minimums for each action)
		int totalMax = 0;
	
		// if there are actions still left
		while (!actions.isEmpty()) 
		{	
			// pop an action off the set
			Action a = actions.popAction();
			
			// define a new minimum
			int actionMin = INT_MAX;
		
			// for each unit in our progress vector
			for (int i(0); i<inProgressSize; ++i) 
			{
				// if the action matches
				if (inProgress[i].action == a) 
				{
					// check to see if we have a new minimum
					actionMin = (inProgress[i].time < actionMin) ? inProgress[i].time : actionMin;
				}
			}
			
			// if we found a new minimum
			if (actionMin < INT_MAX)
			{
				// check to see if we have a new maximum
				totalMax = (actionMin > totalMax) ? actionMin : totalMax;
			}
		}
		
		// return the new maximum
		return totalMax;
	}
	

};
}

#endif
