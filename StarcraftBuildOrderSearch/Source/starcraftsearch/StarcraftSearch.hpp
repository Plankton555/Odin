#ifndef STARCRAFT_SEARCH_H
#define STARCRAFT_SEARCH_H

#include "BWAPI.h"
#include <boost/foreach.hpp>
#include "assert.h"
#include <stdio.h>

#include "StarcraftSearchGoal.hpp"
#include "SearchResults.hpp"
#include "SearchParameters.hpp"
#include "Timer.hpp"

namespace BuildOrderSearch
{
class StarcraftSearch
{

protected:

	bool					goalSet;				// bool flag to let us know if we have set a goal
	Timer 					searchTimer;			// a timer we can use to time the search
	ActionSet 				relevantActions;		// the relevant actions to this particular search	
	ActionSet 				maxOneActions;			// the actions we only need one of
	
	std::set<std::string> 	nodes;
	std::set< std::pair<std::string,std::string> > nodePairs;
	
	StarcraftSearch() {}

	~StarcraftSearch() {}
	
	// stores all recursive prerequisites for an action in the input ActionSet
	void calculateRecursivePrerequisites(Action action, ActionSet & all) {

		// get the set of prerequisites for this action
		ActionSet pre = DATA[action].getPrerequisites();
	
		// remove things we already have in all
		pre.subtract(all);
	
		// if it's empty, stop the recursion
		if (pre.isEmpty()) 
		{
			return;
		}
	
		// add prerequisites to all
		all.add(pre);

		// while prerequisites exist
		while (!pre.isEmpty())
		{		
			// add p's prerequisites
			calculateRecursivePrerequisites(pre.popAction(), all);
		}
	}
	
	std::vector<Action> getBuildOrder(StarcraftState & state)
	{
		std::vector<Action> buildOrder;
		
		StarcraftState * s = &state;
		while (s->getParent() != NULL)
		{
			for (int i=0; i<s->getActionPerformedK(); ++i)
			{
				buildOrder.push_back(s->getActionPerformed());
			}
			s = s->getParent();
		}
		
		return buildOrder;
	}
	
public:
	
	void printNodeString(StarcraftState & state)
	{
		std::cout << getNodeString(state) << getNodeLabel(state) << ";" << std::endl;
		
		if (state.getParent())
		{
			std::cout << getNodeString(*state.getParent()) << " -> " << getNodeString(state) << ";" << std::endl;
		}
	}
	
	void graphVizOutput(StarcraftState & state, bool printFinishTime)
	{
		std::string nodeString(getNodeString(state));
		
		if (printFinishTime)
		{
			int frame = state.getLastFinishTime();
			std::cout << "\"" << frame << "\"" << "[shape=box style=filled fillcolor=\"red\"]";
			std::cout << nodeString << " -> \"" << frame << "\"" << std::endl;
		}
		
		printNodeStringRecursive(state);
	}
	
	void printNodeStringRecursive(StarcraftState & state)
	{
		std::string nodeString(getNodeString(state));
		
		if (nodes.find(nodeString) == nodes.end())
		{
			std::cout << nodeString << getNodeLabel(state) << ";" << std::endl;
			nodes.insert(nodeString);
		}
		
		if (state.getParent())
		{
			std::pair<std::string,std::string> pair(getNodeString(*state.getParent()), nodeString);
			if (nodePairs.find(pair) == nodePairs.end())
			{
				std::cout << getNodeString(*state.getParent()) << " -> " << nodeString << " [arrowhead=none color=\"#ffffff\"];" << std::endl;
				nodePairs.insert(pair);
			}
			
			// recurse if the parent hasn't been done yet
			if (nodes.find(getNodeString(*state.getParent())) == nodes.end())
			{
				printNodeStringRecursive(*state.getParent());
			}
			
		}
	}
	
	std::string getNodeString(StarcraftState & state)
	{
		std::ostringstream node("n");
		
		node << 'n';
	
		std::vector<Action> buildOrder = getBuildOrder(state);
		
		for (size_t i(0); i<buildOrder.size(); ++i)
		{
			node << (int)buildOrder[buildOrder.size() - 1 - i];
		}
		
		return node.str();
	}
	
	std::string getNodeLabel(StarcraftState & state)
	{
		std::ostringstream label("");
		
		if (state.getActionPerformed() == 255)
		{
			return " [label=\"Start\" color=\"#ffffff\" fontcolor=\"#ffffff\" style=\"filled,bold\" fillcolor=\"#00b0f0\"]";
		}
		
		label << " [label=\"";
		label << DATA[state.getActionPerformed()].getName().substr(8) << ", " << state.getCurrentFrame();
		label << "\" color=\"#ffffff\" fontcolor=\"#ffffff\" style=\"filled,bold\" fillcolor=\"#00b0f0\" shape=box]";
		
		return label.str();
	}
	
};
}

#endif
