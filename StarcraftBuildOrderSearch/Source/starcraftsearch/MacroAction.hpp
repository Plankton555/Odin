#pragma once
#include "ActionSet.hpp"
#include <sstream>

namespace BuildOrderSearch
{
class MacroAction
{
	std::vector<Action> 	actions_;
	ActionSet				actionSet_;

public:
	
	MacroAction()  {}
	~MacroAction() {}

	const bool operator == (const MacroAction & m) const
	{
		return compareTo(m) == 0;
	}

	const bool operator < (const MacroAction & m) const
	{
		return compareTo(m) == -1;
	}

	const int compareTo(const MacroAction & m) const
	{
		if (actions_.size() != m.actions_.size())
		{
			return actions_.size() < m.actions_.size() ? -1 : 1;
		}

		for (size_t i(0); i<actions_.size(); ++i)
		{
			if (actions_[i] != m.actions_[i])
			{
				return actions_[i] < m.actions_[i] ? -1 : 1;
			}
		}

		return 0;
	}

	MacroAction(const Action action)
	{
		addAction(action);
	}
	
	void addAction(const Action action)
	{
		actions_.push_back(action);
		actionSet_.add(action);
	}
	
	const bool contains(const Action action) const
	{
		return actionSet_.contains(action);
	}
	
	const bool containsAny(const ActionSet actionSet) const
	{
		return actionSet_.containsAny(actionSet);
	}
	
	const std::vector<Action> & getActions() const
	{
		return actions_;
	}
	
	const size_t size() const
	{
		return actions_.size();
	}

	void print() const 
	{
		BOOST_FOREACH (const Action a, actions_)
		{
			printf("%d ", a);
		}
	}
};
}