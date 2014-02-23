#include "DependencyGraph.hpp"
#include "DFBBStarcraftSearch.hpp"
#include "StarcraftData.hpp"

namespace BuildOrderSearch
{
	StarcraftData & getStarcraftDataInstance()
	{
		static StarcraftData sd;
		return sd;
	}
}