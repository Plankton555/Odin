#pragma once

#include "Common.h"

namespace odin_utils
{
	inline bool replaceString(std::string &str, const std::string &from, const std::string &to)
	{
		int fromPos = str.find(from);
		if (fromPos == std::string::npos)
		{
			return false;
		} else
		{
			str.replace(fromPos, from.length(), to);
			return true;
		}
	}

	inline void replaceAllString(std::string &str, const std::string &from, const std::string &to)
	{
		while (replaceString(str, from, to)) ;
	}

	//In order to use the short name in the BN
	inline void shortenUnitName(std::string &str)
	{
		replaceAllString(str, "Protoss", "");
		replaceAllString(str, "Terran", "");
		replaceAllString(str, "Zerg", "");
		replaceAllString(str," ",""); //Remove all spaces
		replaceAllString(str,"-",""); //remove '-' for u-238 shells
		replaceAllString(str,"SiegeMode",""); //Siege tanks have two modes
		replaceAllString(str,"TankMode","");
	}

	inline int getTimePeriod()
	{
		int timePeriod = BWAPI::Broodwar->getFrameCount()/1000;
		return std::min(25, timePeriod);
	}
};