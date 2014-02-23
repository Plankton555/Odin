#pragma once

#include "Common.h"
#include <BWTA.h>
#include "MetaType.h"

#define PRIORITY_TYPE int

template <class T>
class BuildOrderItem {

public:

	MetaType			metaType;		// the thing we want to 'build'
	T					priority;	// the priority at which to place it in the queue
	bool				blocking;	// whether or not we block further items

	BuildOrderItem(MetaType m, T p, bool b) : metaType(m), priority(p), blocking(b) {}

	bool operator<(const BuildOrderItem<T> &x) const
	{
		return priority < x.priority;
	}
};

class BuildOrderQueue {

	std::deque< BuildOrderItem<PRIORITY_TYPE> >			queue;

	PRIORITY_TYPE lowestPriority;		
	PRIORITY_TYPE highestPriority;
	PRIORITY_TYPE defaultPrioritySpacing;

	int numSkippedItems;

public:

	BuildOrderQueue();

	void clearAll();											// clears the entire build order queue
	void skipItem();											// increments skippedItems
	void queueAsHighestPriority(MetaType m, bool blocking);		// queues something at the highest priority
	void queueAsLowestPriority(MetaType m, bool blocking);		// queues something at the lowest priority
	void queueItem(BuildOrderItem<PRIORITY_TYPE> b);			// queues something with a given priority
	void removeHighestPriorityItem();								// removes the highest priority item
	void removeCurrentHighestPriorityItem();

	int getHighestPriorityValue();								// returns the highest priority value
	int	getLowestPriorityValue();								// returns the lowest priority value
	size_t size();													// returns the size of the queue

	bool isEmpty();

	void removeAll(MetaType m);									// removes all matching meta types from queue

	BuildOrderItem<PRIORITY_TYPE> & getHighestPriorityItem();	// returns the highest priority item
	BuildOrderItem<PRIORITY_TYPE> & getNextHighestPriorityItem();	// returns the highest priority item

	bool canSkipItem();
	bool hasNextHighestPriorityItem();								// returns the highest priority item

	void drawQueueInformation(int x, int y);

	// overload the bracket operator for ease of use
	BuildOrderItem<PRIORITY_TYPE> operator [] (int i); 
};