#pragma once

#include "Common.h"

using namespace std;

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

	inline void shortenUnitName(std::string &str)
	{
		replaceAllString(str, "Protoss", "");
		replaceAllString(str, "Terran", "");
		replaceAllString(str, "Zerg", "");
		replaceAllString(str," ",""); //Remove all spaces
		replaceAllString(str,"-",""); //remove '-' for u-238 shells
	}

	inline int getTimePeriod()
	{
		int timePeriod = BWAPI::Broodwar->getFrameCount()/1000;
		return std::min(25, timePeriod);
	}

	inline void debug(std::string str)
	{
		std::ofstream file ("bwapi-data/Odin/odin_data/debug.txt", ios::app);
		if (file.is_open())
		{
			file << str.c_str() << endl;
			file.close();
		}
	}
	
	inline void debug(std::string str, int i)
	{
		std::ostringstream stringStream;
		stringStream << str;
		stringStream << ": ";
		stringStream << i;
		std::string newStr = stringStream.str();
		debug(newStr);
	}

	inline void debug(std::string str, double d)
	{
		std::ostringstream stringStream;
		stringStream << str;
		stringStream << ": ";
		stringStream << d;
		std::string newStr = stringStream.str();
		debug(newStr);
	}
};