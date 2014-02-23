#pragma once
#include "ActionSet.hpp"
#include <iostream>

namespace BuildOrderSearch
{
class MacroActionBuilder
{
	std::vector< std::vector<Action> > 		buildOrders_;
	std::map< MacroAction, int >			macroActionCount_;

	std::vector<Action> getBuildOrder(const std::string & buildOrderString) const
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

	void readBuildOrders(const std::string & filename)
	{
		std::string line;
		std::ifstream input(filename.c_str());

		while (input.good())
		{
			getline(input, line);
			buildOrders_.push_back(getBuildOrder(line));
		}
	}

	void countMacroActions(const size_t length)
	{
		// for each build order
		BOOST_FOREACH (const std::vector<Action> & buildOrder, buildOrders_)
		{
			// count each macro action of given length
			for (size_t i(0); i + length < buildOrder.size(); ++i)
			{
				MacroAction m;

				for (size_t l(0); l < length; ++l)
				{
					m.addAction(buildOrder[i + l]);
				}

				int count = (macroActionCount_.find(m) == macroActionCount_.end()) ? 0 : macroActionCount_[m];
				macroActionCount_[m] = count + 1;
			}
		}
	}
	
public:
	
	MacroActionBuilder() 
	{
		readBuildOrders("C:\\Users\\Dave\\Dropbox\\Documents\\Starcraft\\Research\\buildorders\\all_uniq.txt");
		countMacroActions(2);
		countMacroActions(3);
		countMacroActions(4);
		countMacroActions(5);
		countMacroActions(6);
		//print();
	}

	~MacroActionBuilder() {}

    const int getCount(const MacroAction & m) 
    {
        return macroActionCount_.find(m) == macroActionCount_.end() ? 0 : macroActionCount_[m];
    }

	void print() const
	{
		std::map<int, MacroAction> reverseMap;

		typedef std::map<MacroAction, int> map_t;
		BOOST_FOREACH (const map_t::value_type & p, macroActionCount_)
		{
			reverseMap[p.second] = p.first;
		}

		typedef std::map<int, MacroAction> map_t2;
		BOOST_FOREACH (const map_t2::value_type & p, reverseMap)
		{
			printf("%6d  ", p.first);
			p.second.print();
			printf("\n");
		}
	}
};
}