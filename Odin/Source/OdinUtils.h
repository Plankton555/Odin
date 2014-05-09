#pragma once

#include "Common.h"

#define ODIN_DEBUG true

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

	//In order to use the short name in the BN
	inline void shortenUnitName(std::string &str)
	{
		replaceString(str, "Protoss", "");
		replaceString(str, "Terran", "");
		replaceString(str, "Zerg", "");
		replaceAllString(str," ",""); //Remove all spaces
		replaceAllString(str,"-",""); //remove '-' for u-238 shells
		replaceAllString(str,"SiegeMode",""); //Siege tanks have two modes
		replaceAllString(str,"TankMode","");
	}

	inline int getTimePeriod()
	{
		int timePeriod = BWAPI::Broodwar->getFrameCount()/1000;
		return std::min(24, timePeriod);
	}

	inline void debug(std::string str)
	{
		if(!ODIN_DEBUG) return;

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

	inline std::vector<std::string> splitDelim(const std::string& str, const std::string& delim)
	{
		std::string s = str;
		std::vector<std::string> output;
		size_t pos = 0;
		std::string token;
		while ((pos = s.find(delim)) != std::string::npos) {
			token = s.substr(0, pos);
			output.push_back(token);
			s.erase(0, pos + delim.length());
		}
		output.push_back(s);
		return output;
	}

	inline std::vector<BWAPI::UnitType> getRequiredUnits(BWAPI::UnitType entity)
	{
		std::vector<BWAPI::UnitType> needTech;
		std::map<BWAPI::UnitType, int> m = entity.requiredUnits();
		std::map<BWAPI::UnitType, int>::iterator mit;
		for (mit = m.begin(); mit != m.end(); mit++)
		{
			if (BWAPI::Broodwar->self()->allUnitCount(mit->first) < mit->second)
			{
				needTech.push_back(mit->first);
			}
		}
		return needTech;
	}
};